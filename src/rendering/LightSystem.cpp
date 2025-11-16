/*
------------------------------------------------------------------------------

Luma Engine - A Next-Generation Game Engine

Copyright (c) 2025 Nexel Games. All Rights Reserved.

This source code is part of the Luma Engine project developed by Nexel Games.

Use of this software is governed by the Luma Engine License Agreement.

Unauthorized copying of this file, via any medium, is strictly prohibited.

Distribution of source or binary forms, with or without modification, is
subject to the terms of the Luma Engine License.

For more information, visit: https://nexelgames.com/luma-engine

------------------------------------------------------------------------------

*/

#include "LGE/rendering/LightSystem.h"
#include "LGE/core/scene/World.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/core/scene/components/LightComponent.h"
#include "LGE/core/scene/components/Transform.h"
#include "LGE/core/scene/components/MeshRenderer.h"
#include "LGE/rendering/Camera.h"
#include "LGE/rendering/Shader.h"
#include "LGE/rendering/Mesh.h"
#include "LGE/rendering/VertexArray.h"
#include "LGE/rendering/IndexBuffer.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <glad/glad.h>
#include <algorithm>
#include <cmath>

namespace LGE {

LightSystem::LightSystem()
    : m_FrameUBOID(0)
    , m_LightBufferID(0)
{
    m_Lights.reserve(MAX_LIGHTS);
    m_DirectionalShadow = {};
    m_DirectionalShadow.IsValid = false;
    m_CascadedShadowMap.IsValid = false;
    m_ShadowAtlas.IsValid = false;
}

LightSystem::~LightSystem() {
    Shutdown();
}

bool LightSystem::Initialize() {
    try {
        // Create Frame UBO
        glGenBuffers(1, &m_FrameUBOID);
        if (glGetError() != GL_NO_ERROR) {
            Log::Error("Failed to create Frame UBO!");
            return false;
        }
        glBindBuffer(GL_UNIFORM_BUFFER, m_FrameUBOID);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(int) * 4, nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, FRAME_UBO_BINDING, m_FrameUBOID);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
        
        // Create Light SSBO
        glGenBuffers(1, &m_LightBufferID);
        if (glGetError() != GL_NO_ERROR) {
            Log::Error("Failed to create Light SSBO!");
            return false;
        }
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightBufferID);
        glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_LIGHTS * sizeof(LightDataGPU), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHT_SSBO_BINDING, m_LightBufferID);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
        
        // Load shadow caster shader (optional - shadows won't work if this fails)
        // Defer shadow map creation until first use to avoid issues if OpenGL isn't fully ready
        std::string shadowVertSource = FileSystem::ReadFile("assets/shaders/ShadowCaster.vert");
        std::string shadowFragSource = FileSystem::ReadFile("assets/shaders/ShadowCaster.frag");
        
        if (shadowVertSource.empty() || shadowFragSource.empty()) {
            Log::Warn("Failed to load shadow caster shader files! Shadow mapping will be disabled.");
            m_DirectionalShadow.IsValid = false;
        } else {
            m_ShadowCasterShader = std::make_shared<Shader>(shadowVertSource, shadowFragSource);
            if (m_ShadowCasterShader->GetRendererID() == 0) {
                Log::Warn("Failed to compile shadow caster shader! Shadow mapping will be disabled.");
                m_ShadowCasterShader.reset();
                m_DirectionalShadow.IsValid = false;
            }
            // Don't create shadow map here - defer to first RenderShadowMaps call
        }
        
        Log::Info("LightSystem initialized");
        return true;
    } catch (const std::exception& e) {
        Log::Error("LightSystem::Initialize exception: " + std::string(e.what()));
        return false;
    } catch (...) {
        Log::Error("LightSystem::Initialize unknown exception!");
        return false;
    }
}

void LightSystem::Shutdown() {
    // Clean up shadow map
    if (m_DirectionalShadow.ShadowMapTextureID != 0) {
        glDeleteTextures(1, &m_DirectionalShadow.ShadowMapTextureID);
        m_DirectionalShadow.ShadowMapTextureID = 0;
    }
    if (m_DirectionalShadow.ShadowMapFBO != 0) {
        glDeleteFramebuffers(1, &m_DirectionalShadow.ShadowMapFBO);
        m_DirectionalShadow.ShadowMapFBO = 0;
    }
    
    if (m_FrameUBOID != 0) {
        glDeleteBuffers(1, &m_FrameUBOID);
        m_FrameUBOID = 0;
    }
    
    if (m_LightBufferID != 0) {
        glDeleteBuffers(1, &m_LightBufferID);
        m_LightBufferID = 0;
    }
}

void LightSystem::BeginFrame(World& world) {
    // Clear lights for new frame - this ensures we always have fresh data
    m_Lights.clear();
    m_Lights.reserve(MAX_LIGHTS); // Reserve space to avoid reallocations
    
    // Collect all active lights from the world
    // This is called every frame, so any changes to light properties will be picked up
    CollectLights(world);
}

void LightSystem::CollectLights(World& world) {
    auto allObjects = world.GetAllGameObjects();
    
    for (const auto& obj : allObjects) {
        if (!obj || !obj->IsActive()) {
            continue;
        }
        
        auto lightComponent = obj->GetComponent<LightComponent>();
        if (!lightComponent) {
            continue;
        }
        // Note: LightComponent doesn't have IsEnabled() - all lights are active if component exists
        
        auto* transform = obj->GetTransform();
        if (!transform) {
            continue;
        }
        
        LightDataGPU lightData = {};
        
        // Get position and forward direction from transform
        Math::Vector3 position = transform->GetWorldPosition();
        Math::Vector3 forward = transform->Forward();
        
        // Normalize forward direction
        float forwardLen = std::sqrt(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
        if (forwardLen > 0.0001f) {
            forward.x /= forwardLen;
            forward.y /= forwardLen;
            forward.z /= forwardLen;
        } else {
            forward = Math::Vector3(0.0f, 0.0f, -1.0f); // Default forward
        }
        
        if (lightComponent->IsDirectional()) {
            // Directional light - use forward direction, position is ignored
            lightData.Position = Math::Vector4(0.0f, 0.0f, 0.0f, 0.0f); // w = 0 indicates directional
            lightData.Direction = Math::Vector4(forward.x, forward.y, forward.z, 0.0f);
            lightData.Range = 0.0f;
            lightData.InnerCone = 0.0f;
            lightData.OuterCone = 0.0f;
        }
        else {
            // Point or Spot light - use position
            lightData.Position = Math::Vector4(position.x, position.y, position.z, 1.0f); // w = 1 indicates point/spot
            lightData.Direction = Math::Vector4(forward.x, forward.y, forward.z, 0.0f);
            lightData.Range = lightComponent->Range;
            
            if (lightComponent->IsPoint()) {
                lightData.InnerCone = 0.0f;
                lightData.OuterCone = 0.0f;
            } else {
                // Spot light
                lightData.InnerCone = lightComponent->InnerAngle;
                lightData.OuterCone = lightComponent->OuterAngle;
            }
        }
        
        // Set color and intensity (multiply color by intensity in RGB, store intensity in alpha)
        // IMPORTANT: This ensures intensity changes are immediately reflected
        float intensity = lightComponent->Intensity;
        Math::Vector3 color = lightComponent->Color;
        lightData.ColorIntensity = Math::Vector4(
            color.x * intensity,
            color.y * intensity,
            color.z * intensity,
            intensity
        );
        
        // Set type and shadow flag
        lightData.Type = static_cast<int>(lightComponent->Type);
        lightData.CastShadows = lightComponent->CastShadows ? 1 : 0;
        
        if (m_Lights.size() < MAX_LIGHTS) {
            m_Lights.push_back(lightData);
        } else {
            Log::Warn("Maximum light count reached (" + std::to_string(MAX_LIGHTS) + ")");
            break;
        }
    }
}

void LightSystem::UploadToGPU() {
    // Update Light SSBO with latest light data
    // This is called every frame to ensure changes are reflected immediately
    
    if (m_LightBufferID == 0) {
        return; // Buffer not initialized
    }
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightBufferID);
    
    if (!m_Lights.empty()) {
        // Update the buffer with current light data
        size_t dataSize = m_Lights.size() * sizeof(LightDataGPU);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, dataSize, m_Lights.data());
        
        // Clear any remaining space in the buffer (if we have fewer lights than MAX_LIGHTS)
        if (m_Lights.size() < MAX_LIGHTS) {
            LightDataGPU emptyLight = {};
            std::vector<LightDataGPU> emptyLights(MAX_LIGHTS - m_Lights.size(), emptyLight);
            size_t remainingSize = emptyLights.size() * sizeof(LightDataGPU);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, dataSize, remainingSize, emptyLights.data());
        }
    } else {
        // Clear the buffer if no lights - zero out all entries
        LightDataGPU emptyLight = {};
        std::vector<LightDataGPU> emptyLights(MAX_LIGHTS, emptyLight);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, MAX_LIGHTS * sizeof(LightDataGPU), emptyLights.data());
    }
    
    // Unbind before setting binding point (some drivers require this)
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    // Ensure the buffer is bound to the correct binding point
    // This must be called every frame to ensure the binding is active
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHT_SSBO_BINDING, m_LightBufferID);
    
    // Memory barrier to ensure the SSBO update is visible to shaders
    // This ensures all writes to the SSBO are complete before shaders read from it
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void LightSystem::BindLightingBuffers() {
    // Bind UBO for frame lighting data
    glBindBufferBase(GL_UNIFORM_BUFFER, FRAME_UBO_BINDING, m_FrameUBOID);
    
    // Bind SSBO for light array - this must be called every frame
    // The binding point (3) must match the shader's layout(binding = 3)
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHT_SSBO_BINDING, m_LightBufferID);
}

void LightSystem::CreateShadowMap() {
    if (!m_ShadowCasterShader) {
        m_DirectionalShadow.IsValid = false;
        return;
    }
    
    // Check if already created
    if (m_DirectionalShadow.ShadowMapTextureID != 0) {
        return;
    }
    
    try {
        // Create shadow map texture
        glGenTextures(1, &m_DirectionalShadow.ShadowMapTextureID);
        if (glGetError() != GL_NO_ERROR || m_DirectionalShadow.ShadowMapTextureID == 0) {
            Log::Warn("Failed to create shadow map texture! Shadow mapping will be disabled.");
            m_DirectionalShadow.IsValid = false;
            return;
        }
        
        glBindTexture(GL_TEXTURE_2D, m_DirectionalShadow.ShadowMapTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glBindTexture(GL_TEXTURE_2D, 0);
        
        // Create framebuffer for shadow map
        glGenFramebuffers(1, &m_DirectionalShadow.ShadowMapFBO);
        if (glGetError() != GL_NO_ERROR || m_DirectionalShadow.ShadowMapFBO == 0) {
            Log::Warn("Failed to create shadow map framebuffer! Shadow mapping will be disabled.");
            if (m_DirectionalShadow.ShadowMapTextureID != 0) {
                glDeleteTextures(1, &m_DirectionalShadow.ShadowMapTextureID);
                m_DirectionalShadow.ShadowMapTextureID = 0;
            }
            m_DirectionalShadow.IsValid = false;
            return;
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, m_DirectionalShadow.ShadowMapFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_DirectionalShadow.ShadowMapTextureID, 0);
        glDrawBuffer(GL_NONE); // No color output
        glReadBuffer(GL_NONE); // No color output
        
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            Log::Warn("Shadow map framebuffer is not complete! Shadow mapping will be disabled.");
            m_DirectionalShadow.IsValid = false;
        } else {
            m_DirectionalShadow.IsValid = true;
            m_DirectionalShadow.ShadowMapSize = SHADOW_MAP_SIZE;
            Log::Info("Shadow map created successfully (" + std::to_string(SHADOW_MAP_SIZE) + "x" + std::to_string(SHADOW_MAP_SIZE) + ")");
        }
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } catch (const std::exception& e) {
        Log::Error("CreateShadowMap exception: " + std::string(e.what()));
        m_DirectionalShadow.IsValid = false;
    } catch (...) {
        Log::Error("CreateShadowMap unknown exception!");
        m_DirectionalShadow.IsValid = false;
    }
}

void LightSystem::CalculateLightViewProj(const Math::Vector3& lightDir, Camera* camera, Math::Matrix4& outViewProj) {
    if (!camera) {
        return;
    }
    
    // Calculate orthographic frustum that covers camera view
    // For now, use a fixed size around camera position
    float orthoSize = 20.0f; // Size of orthographic frustum
    float nearPlane = 0.1f;
    float farPlane = 50.0f;
    
    // Get camera position and calculate light position (behind camera in light direction)
    Math::Vector3 cameraPos = camera->GetPosition();
    Math::Vector3 lightPos = cameraPos - lightDir * (farPlane * 0.5f);
    
    // Calculate up vector (perpendicular to light direction)
    Math::Vector3 up = Math::Vector3(0.0f, 1.0f, 0.0f);
    if (std::abs(lightDir.y) > 0.9f) {
        up = Math::Vector3(1.0f, 0.0f, 0.0f);
    }
    
    // Create light view matrix (look at camera area from light)
    Math::Vector3 target = cameraPos;
    Math::Matrix4 lightView = Math::Matrix4::LookAt(lightPos, target, up);
    
    // Create orthographic projection
    Math::Matrix4 lightProj = Math::Matrix4::Orthographic(
        -orthoSize, orthoSize,
        -orthoSize, orthoSize,
        nearPlane, farPlane
    );
    
    outViewProj = lightProj * lightView;
}

void LightSystem::RenderSceneToShadowMap(World& world, const Math::Matrix4& lightViewProj) {
    if (!m_ShadowCasterShader) {
        return;
    }
    
    // Save current OpenGL state
    GLint currentFBO;
    GLint currentViewport[4];
    GLboolean depthTestEnabled;
    GLint currentCullFace;
    GLboolean colorMask[4];
    
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    glGetIntegerv(GL_VIEWPORT, currentViewport);
    depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    glGetIntegerv(GL_CULL_FACE_MODE, &currentCullFace);
    glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
    
    // Bind shadow map framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, m_DirectionalShadow.ShadowMapFBO);
    glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
    
    // Clear depth buffer
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    
    // Disable color writes (we only need depth)
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    
    // Cull front faces to reduce shadow acne
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    
    // Bind shadow caster shader
    m_ShadowCasterShader->Bind();
    m_ShadowCasterShader->SetUniformMat4("u_LightViewProj", lightViewProj.m);
    
    // Render all objects with MeshRenderer
    auto allObjects = world.GetAllGameObjects();
    for (const auto& obj : allObjects) {
        if (!obj || !obj->IsActive()) {
            continue;
        }
        
        auto meshRenderer = obj->GetComponent<MeshRenderer>();
        if (!meshRenderer) {
            continue;
        }
        
        auto mesh = meshRenderer->GetMesh();
        if (!mesh) {
            continue;
        }
        
        auto* transform = obj->GetTransform();
        if (!transform) {
            continue;
        }
        
        // Set model matrix
        Math::Matrix4 modelMatrix = transform->GetWorldMatrix();
        m_ShadowCasterShader->SetUniformMat4("u_Model", modelMatrix.m);
        
        // Draw mesh
        auto vertexArray = mesh->GetVertexArray();
        if (vertexArray) {
            vertexArray->Bind();
            
            auto indexBuffer = mesh->GetIndexBuffer();
            uint32_t indexCount = mesh->GetIndexCount();
            uint32_t vertexCount = mesh->GetVertexCount();
            
            if (indexBuffer && indexCount > 0) {
                indexBuffer->Bind();
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
                indexBuffer->Unbind();
            } else if (vertexCount > 0) {
                glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
            }
            
            vertexArray->Unbind();
        }
    }
    
    // Restore state
    m_ShadowCasterShader->Unbind();
    glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
    glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);
    glCullFace(currentCullFace);
    glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
    if (!depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }
}

void LightSystem::RenderShadowMaps(World& world, Camera* camera) {
    if (!camera || !m_ShadowCasterShader) {
        m_DirectionalShadow.IsValid = false;
        return;
    }
    
    // Create shadow map on first use (lazy initialization)
    if (m_DirectionalShadow.ShadowMapTextureID == 0) {
        CreateShadowMap();
        if (!m_DirectionalShadow.IsValid) {
            return; // Shadow map creation failed
        }
    }
    
    // Find first directional light that casts shadows
    Math::Vector3 lightDir;
    bool foundLight = false;
    
    for (const auto& lightData : m_Lights) {
        if (lightData.Type == static_cast<int>(LightType::Directional) && lightData.CastShadows != 0) {
            lightDir = Math::Vector3(lightData.Direction.x, lightData.Direction.y, lightData.Direction.z);
            // Normalize and negate (direction points toward light, we need away from light)
            float len = std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
            if (len > 0.0001f) {
                lightDir.x = -lightDir.x / len;
                lightDir.y = -lightDir.y / len;
                lightDir.z = -lightDir.z / len;
            }
            foundLight = true;
            break;
        }
    }
    
    if (!foundLight || !m_DirectionalShadow.IsValid) {
        m_DirectionalShadow.IsValid = false;
        return;
    }
    
    try {
        // Calculate light view-projection matrix
        CalculateLightViewProj(lightDir, camera, m_DirectionalShadow.LightViewProj);
        
        // Render scene to shadow map
        RenderSceneToShadowMap(world, m_DirectionalShadow.LightViewProj);
        
        m_DirectionalShadow.IsValid = true;
    } catch (const std::exception& e) {
        Log::Error("RenderShadowMaps exception: " + std::string(e.what()));
        m_DirectionalShadow.IsValid = false;
    } catch (...) {
        Log::Error("RenderShadowMaps unknown exception!");
        m_DirectionalShadow.IsValid = false;
    }
}

// Cascaded Shadow Maps (CSM) - future implementation
void LightSystem::RenderCascadedShadowMaps(World& world, Camera* camera) {
    // TODO: Implement CSM rendering
    // This will split the camera frustum into multiple cascades
    // and render a shadow map for each cascade
    if (!camera || !m_ShadowCasterShader) {
        m_CascadedShadowMap.IsValid = false;
        return;
    }
    
    // Placeholder: CSM not yet implemented
    m_CascadedShadowMap.IsValid = false;
}

// Shadow map atlas - future implementation
void LightSystem::InitializeShadowAtlas() {
    // TODO: Implement shadow map atlas
    // This will create a large texture (e.g., 4096x4096) divided into tiles
    // Each light that casts shadows gets a tile
    m_ShadowAtlas.IsValid = false;
}

} // namespace LGE
