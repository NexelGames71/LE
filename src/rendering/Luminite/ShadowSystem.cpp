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

#include "LGE/rendering/Luminite/ShadowSystem.h"
#include "LGE/rendering/Camera.h"
#include "LGE/rendering/Shader.h"
#include "LGE/rendering/Framebuffer.h"
#include "LGE/core/FileSystem.h"
#include "LGE/core/Log.h"
#include "LGE/math/Vector.h"
#include <glad/glad.h>
#include <algorithm>
#include <cmath>
#include <array>
#include <limits>

namespace LGE {
namespace Luminite {

ShadowSystem::ShadowSystem()
    : m_ShadowResolution(2048)
    , m_ShadowBias(0.005f)
    , m_ShadowNormalBias(0.02f)
    , m_CascadeDistances({10.0f, 25.0f, 50.0f, 100.0f})
    , m_PCFEnabled(true)
    , m_PCFSize(3)
    , m_Initialized(false)
{
}

ShadowSystem::~ShadowSystem() {
    Shutdown();
}

bool ShadowSystem::Initialize() {
    if (m_Initialized) {
        return true;
    }

    // Load shadow map shaders
    std::string shadowVertSource = FileSystem::ReadFile("assets/shaders/Luminite/ShadowMap.vert");
    std::string shadowFragSource = FileSystem::ReadFile("assets/shaders/Luminite/ShadowMap.frag");

    if (shadowVertSource.empty() || shadowFragSource.empty()) {
        Log::Error("Failed to load shadow map shader files!");
        return false;
    }

    m_ShadowMapShader = std::make_shared<Shader>(shadowVertSource, shadowFragSource);
    if (m_ShadowMapShader->GetRendererID() == 0) {
        Log::Error("Failed to compile shadow map shader!");
        return false;
    }

    // Load point light shadow shader (with geometry shader)
    std::string pointVertSource = FileSystem::ReadFile("assets/shaders/Luminite/ShadowMapPoint.vert");
    std::string pointGeomSource = FileSystem::ReadFile("assets/shaders/Luminite/ShadowMapPoint.geom");
    std::string pointFragSource = FileSystem::ReadFile("assets/shaders/Luminite/ShadowMapPoint.frag");

    // Note: We'll need to extend Shader class to support geometry shaders
    // For now, just log that point light shadows need geometry shader support
    if (pointVertSource.empty() || pointGeomSource.empty() || pointFragSource.empty()) {
        Log::Warn("Point light shadow shaders loaded but geometry shader support needed");
    }

    m_Initialized = true;
    Log::Info("ShadowSystem initialized successfully!");
    return true;
}

void ShadowSystem::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    // Clean up shadow map textures
    for (auto& entry : m_ShadowMaps) {
        if (entry.shadowMapTextureID != 0) {
            glDeleteTextures(1, &entry.shadowMapTextureID);
        }
        if (entry.cubemapTextureID != 0) {
            glDeleteTextures(1, &entry.cubemapTextureID);
        }
    }

    m_ShadowMaps.clear();
    m_ShadowMapShader.reset();
    m_ShadowMapPointShader.reset();

    m_Initialized = false;
}

void ShadowSystem::UpdateShadowMaps(const Camera& camera, float deltaTime) {
    if (!m_Initialized) {
        return;
    }

    // Render shadow maps for all registered lights
    for (auto& entry : m_ShadowMaps) {
        if (entry.dirty) {
            RenderShadowMap(entry, camera);
            entry.dirty = false;
        }
    }
}

void ShadowSystem::RegisterDirectionalLightShadow(void* owner, const Math::Vector3& direction, const Math::Vector3& position, int cascadeCount) {
    // Check if already registered
    for (auto& entry : m_ShadowMaps) {
        if (entry.lightOwner == owner) {
            entry.lightDirection = direction;
            entry.dirty = true;
            return;
        }
    }

    ShadowMapEntry entry;
    entry.lightOwner = owner;
    entry.type = EShadowMapType::DirectionalCSM;
    entry.resolution = m_ShadowResolution;
    entry.lightDirection = direction;
    entry.dirty = true;

    // Create cascades
    entry.cascades.resize(cascadeCount);
    for (int i = 0; i < cascadeCount; ++i) {
        entry.cascades[i].resolution = m_ShadowResolution;
        entry.cascades[i].splitDistance = (i < m_CascadeDistances.size()) ? m_CascadeDistances[i] : m_CascadeDistances.back();
    }

    // Create shadow map texture (for now, single texture - will be expanded to texture array)
    glGenTextures(1, &entry.shadowMapTextureID);
    glBindTexture(GL_TEXTURE_2D, entry.shadowMapTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_ShadowResolution, m_ShadowResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glBindTexture(GL_TEXTURE_2D, 0);

    m_ShadowMaps.push_back(entry);
}

void ShadowSystem::RegisterPointLightShadow(void* owner, const Math::Vector3& position, float range) {
    // TODO: Implement point light shadow registration
    Log::Warn("Point light shadows not yet implemented");
}

void ShadowSystem::RegisterSpotLightShadow(void* owner, const Math::Vector3& position, const Math::Vector3& direction, float range, float innerCone, float outerCone) {
    // TODO: Implement spot light shadow registration
    Log::Warn("Spot light shadows not yet implemented");
}

void ShadowSystem::UnregisterLightShadow(void* owner) {
    m_ShadowMaps.erase(
        std::remove_if(m_ShadowMaps.begin(), m_ShadowMaps.end(),
            [owner](const ShadowMapEntry& entry) {
                if (entry.lightOwner == owner) {
                    if (entry.shadowMapTextureID != 0) {
                        glDeleteTextures(1, &entry.shadowMapTextureID);
                    }
                    if (entry.cubemapTextureID != 0) {
                        glDeleteTextures(1, &entry.cubemapTextureID);
                    }
                    return true;
                }
                return false;
            }),
        m_ShadowMaps.end()
    );
}

void ShadowSystem::RenderShadowMap(ShadowMapEntry& entry, const Camera& camera) {
    switch (entry.type) {
        case EShadowMapType::DirectionalCSM:
            RenderCSM(entry, camera);
            break;
        case EShadowMapType::PointCube:
            RenderPointLightShadow(entry);
            break;
        case EShadowMapType::Spot2D:
            RenderSpotLightShadow(entry);
            break;
        default:
            break;
    }
}

void ShadowSystem::RenderCSM(ShadowMapEntry& entry, const Camera& camera) {
    if (entry.cascades.empty() || !m_ShadowMapShader) {
        return;
    }
    
    // Get light direction from entry
    Math::Vector3 lightDir = entry.lightDirection;
    
    // Calculate CSM splits and light view-projection matrices
    CalculateCSMSplits(camera, lightDir, entry.cascades);
    
    // Save current OpenGL state
    GLint currentFBO;
    GLint currentViewport[4];
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    glGetIntegerv(GL_VIEWPORT, currentViewport);
    
    // Create framebuffer for shadow map rendering (if not exists)
    static GLuint shadowFBO = 0;
    if (shadowFBO == 0) {
        glGenFramebuffers(1, &shadowFBO);
    }
    
    // Render each cascade
    for (size_t i = 0; i < entry.cascades.size(); ++i) {
        auto& cascade = entry.cascades[i];
        
        // Bind shadow map framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, entry.shadowMapTextureID, 0);
        
        // Clear framebuffer
        glViewport(0, 0, cascade.resolution, cascade.resolution);
        glClear(GL_DEPTH_BUFFER_BIT);
        
        // Disable color writes (shadow maps only need depth)
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        
        // Use shadow map shader
        m_ShadowMapShader->Bind();
        m_ShadowMapShader->SetUniformMat4("u_LightViewProjection", cascade.lightViewProjection.GetData());
        
        // TODO: Render scene geometry here
        // This would iterate through all shadow-casting objects and render them
        // For now, this is a placeholder - the actual rendering would be done by the main renderer
        // calling a RenderShadowPass() method
        
        // Restore color writes
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }
    
    // Restore OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
    glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);
    m_ShadowMapShader->Unbind();
}

void ShadowSystem::RenderPointLightShadow(ShadowMapEntry& entry) {
    // TODO: Implement point light shadow rendering
    Log::Info("Point light shadow rendering not yet implemented");
}

void ShadowSystem::RenderSpotLightShadow(ShadowMapEntry& entry) {
    // TODO: Implement spot light shadow rendering
    Log::Info("Spot light shadow rendering not yet implemented");
}

void ShadowSystem::CalculateCSMSplits(const Camera& camera, const Math::Vector3& lightDir, std::vector<ShadowCascade>& cascades) {
    if (cascades.empty()) {
        return;
    }

    // Get camera parameters
    Math::Vector3 cameraPos = camera.GetPosition();
    Math::Vector3 cameraForward = camera.GetTarget() - cameraPos;
    float camLength = std::sqrt(cameraForward.x * cameraForward.x + cameraForward.y * cameraForward.y + cameraForward.z * cameraForward.z);
    cameraForward = Math::Vector3(cameraForward.x / camLength, cameraForward.y / camLength, cameraForward.z / camLength);
    
    Math::Vector3 cameraUp = camera.GetUp();
    Math::Vector3 cameraRight = Math::Vector3(
        cameraForward.y * cameraUp.z - cameraForward.z * cameraUp.y,
        cameraForward.z * cameraUp.x - cameraForward.x * cameraUp.z,
        cameraForward.x * cameraUp.y - cameraForward.y * cameraUp.x
    );
    float rightLength = std::sqrt(cameraRight.x * cameraRight.x + cameraRight.y * cameraRight.y + cameraRight.z * cameraRight.z);
    cameraRight = Math::Vector3(cameraRight.x / rightLength, cameraRight.y / rightLength, cameraRight.z / rightLength);
    
    // Get camera projection parameters
    float nearPlane = camera.GetNearPlane();
    float farPlane = camera.GetFarPlane();
    float fov = camera.GetFOV() * 3.14159f / 180.0f; // Convert to radians
    float aspectRatio = camera.GetAspectRatio();
    
    // Only support perspective cameras for CSM
    if (camera.GetProjectionType() != Camera::ProjectionType::Perspective) {
        Log::Warn("CSM only supports perspective cameras");
        return;
    }
    
    // Calculate frustum corners at near and far planes
    auto GetFrustumCorners = [&](float nearDist, float farDist) -> std::array<Math::Vector3, 8> {
        float nearHeight = 2.0f * std::tan(fov * 0.5f) * nearDist;
        float nearWidth = nearHeight * aspectRatio;
        float farHeight = 2.0f * std::tan(fov * 0.5f) * farDist;
        float farWidth = farHeight * aspectRatio;
        
        Math::Vector3 nearCenter = Math::Vector3(
            cameraPos.x + cameraForward.x * nearDist,
            cameraPos.y + cameraForward.y * nearDist,
            cameraPos.z + cameraForward.z * nearDist
        );
        Math::Vector3 farCenter = Math::Vector3(
            cameraPos.x + cameraForward.x * farDist,
            cameraPos.y + cameraForward.y * farDist,
            cameraPos.z + cameraForward.z * farDist
        );
        
        std::array<Math::Vector3, 8> corners;
        // Near plane corners
        corners[0] = Math::Vector3(nearCenter.x - cameraRight.x * nearWidth * 0.5f - cameraUp.x * nearHeight * 0.5f,
                                  nearCenter.y - cameraRight.y * nearWidth * 0.5f - cameraUp.y * nearHeight * 0.5f,
                                  nearCenter.z - cameraRight.z * nearWidth * 0.5f - cameraUp.z * nearHeight * 0.5f);
        corners[1] = Math::Vector3(nearCenter.x + cameraRight.x * nearWidth * 0.5f - cameraUp.x * nearHeight * 0.5f,
                                  nearCenter.y + cameraRight.y * nearWidth * 0.5f - cameraUp.y * nearHeight * 0.5f,
                                  nearCenter.z + cameraRight.z * nearWidth * 0.5f - cameraUp.z * nearHeight * 0.5f);
        corners[2] = Math::Vector3(nearCenter.x + cameraRight.x * nearWidth * 0.5f + cameraUp.x * nearHeight * 0.5f,
                                  nearCenter.y + cameraRight.y * nearWidth * 0.5f + cameraUp.y * nearHeight * 0.5f,
                                  nearCenter.z + cameraRight.z * nearWidth * 0.5f + cameraUp.z * nearHeight * 0.5f);
        corners[3] = Math::Vector3(nearCenter.x - cameraRight.x * nearWidth * 0.5f + cameraUp.x * nearHeight * 0.5f,
                                  nearCenter.y - cameraRight.y * nearWidth * 0.5f + cameraUp.y * nearHeight * 0.5f,
                                  nearCenter.z - cameraRight.z * nearWidth * 0.5f + cameraUp.z * nearHeight * 0.5f);
        // Far plane corners
        corners[4] = Math::Vector3(farCenter.x - cameraRight.x * farWidth * 0.5f - cameraUp.x * farHeight * 0.5f,
                                   farCenter.y - cameraRight.y * farWidth * 0.5f - cameraUp.y * farHeight * 0.5f,
                                   farCenter.z - cameraRight.z * farWidth * 0.5f - cameraUp.z * farHeight * 0.5f);
        corners[5] = Math::Vector3(farCenter.x + cameraRight.x * farWidth * 0.5f - cameraUp.x * farHeight * 0.5f,
                                   farCenter.y + cameraRight.y * farWidth * 0.5f - cameraUp.y * farHeight * 0.5f,
                                   farCenter.z + cameraRight.z * farWidth * 0.5f - cameraUp.z * farHeight * 0.5f);
        corners[6] = Math::Vector3(farCenter.x + cameraRight.x * farWidth * 0.5f + cameraUp.x * farHeight * 0.5f,
                                   farCenter.y + cameraRight.y * farWidth * 0.5f + cameraUp.y * farHeight * 0.5f,
                                   farCenter.z + cameraRight.z * farWidth * 0.5f + cameraUp.z * farHeight * 0.5f);
        corners[7] = Math::Vector3(farCenter.x - cameraRight.x * farWidth * 0.5f + cameraUp.x * farHeight * 0.5f,
                                   farCenter.y - cameraRight.y * farWidth * 0.5f + cameraUp.y * farHeight * 0.5f,
                                   farCenter.z - cameraRight.z * farWidth * 0.5f + cameraUp.z * farHeight * 0.5f);
        return corners;
    };
    
    // Calculate light space view matrix
    Math::Vector3 lightDirNormalized = Math::Vector3(
        lightDir.x / std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z),
        lightDir.y / std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z),
        lightDir.z / std::sqrt(lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z)
    );
    
    Math::Vector3 lightUp = (std::abs(lightDirNormalized.y) > 0.9f) ? 
        Math::Vector3(1.0f, 0.0f, 0.0f) : Math::Vector3(0.0f, 1.0f, 0.0f);
    
    // Calculate light space center (average of all cascade corners)
    Math::Vector3 lightSpaceCenter(0.0f, 0.0f, 0.0f);
    int totalCorners = 0;
    
    float prevSplit = nearPlane;
    for (size_t i = 0; i < cascades.size(); ++i) {
        float splitDist = cascades[i].splitDistance;
        auto corners = GetFrustumCorners(prevSplit, splitDist);
        
        for (const auto& corner : corners) {
            lightSpaceCenter = Math::Vector3(
                lightSpaceCenter.x + corner.x,
                lightSpaceCenter.y + corner.y,
                lightSpaceCenter.z + corner.z
            );
            totalCorners++;
        }
        prevSplit = splitDist;
    }
    
    if (totalCorners > 0) {
        lightSpaceCenter = Math::Vector3(
            lightSpaceCenter.x / totalCorners,
            lightSpaceCenter.y / totalCorners,
            lightSpaceCenter.z / totalCorners
        );
    }
    
    Math::Vector3 lightTarget = Math::Vector3(
        lightSpaceCenter.x - lightDirNormalized.x * 10.0f,
        lightSpaceCenter.y - lightDirNormalized.y * 10.0f,
        lightSpaceCenter.z - lightDirNormalized.z * 10.0f
    );
    
    Math::Matrix4 lightView = Math::Matrix4::LookAt(lightSpaceCenter, lightTarget, lightUp);
    
    // Calculate orthographic projection for each cascade
    prevSplit = nearPlane;
    for (size_t i = 0; i < cascades.size(); ++i) {
        float splitDist = cascades[i].splitDistance;
        auto corners = GetFrustumCorners(prevSplit, splitDist);
        
        // Transform corners to light space
        float minX = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float minY = std::numeric_limits<float>::max();
        float maxY = std::numeric_limits<float>::lowest();
        float minZ = std::numeric_limits<float>::max();
        float maxZ = std::numeric_limits<float>::lowest();
        
        for (const auto& corner : corners) {
            Math::Vector4 lightSpaceCorner = lightView * Math::Vector4(corner.x, corner.y, corner.z, 1.0f);
            
            minX = std::min(minX, lightSpaceCorner.x);
            maxX = std::max(maxX, lightSpaceCorner.x);
            minY = std::min(minY, lightSpaceCorner.y);
            maxY = std::max(maxY, lightSpaceCorner.y);
            minZ = std::min(minZ, lightSpaceCorner.z);
            maxZ = std::max(maxZ, lightSpaceCorner.z);
        }
        
        // Stabilize cascade (snap to texel grid)
        float texelSize = (maxX - minX) / static_cast<float>(cascades[i].resolution);
        minX = std::floor(minX / texelSize) * texelSize;
        maxX = std::floor(maxX / texelSize) * texelSize;
        minY = std::floor(minY / texelSize) * texelSize;
        maxY = std::floor(maxY / texelSize) * texelSize;
        
        // Create orthographic projection for this cascade
        Math::Matrix4 lightProj = Math::Matrix4::Orthographic(minX, maxX, minY, maxY, minZ, maxZ);
        
        // Store light view-projection matrix
        cascades[i].lightViewProjection = lightProj * lightView;
        
        prevSplit = splitDist;
    }
}

void ShadowSystem::StabilizeCascade(ShadowCascade& cascade, float texelSize) {
    // Stabilization is now done in CalculateCSMSplits
    // This function is kept for future use if needed
}

void ShadowSystem::SetCascadeDistances(const std::vector<float>& distances) {
    m_CascadeDistances = distances;
    // Update all CSM entries
    for (auto& entry : m_ShadowMaps) {
        if (entry.type == EShadowMapType::DirectionalCSM) {
            for (size_t i = 0; i < entry.cascades.size() && i < distances.size(); ++i) {
                entry.cascades[i].splitDistance = distances[i];
            }
            entry.dirty = true;
        }
    }
}

const std::vector<ShadowCascade>& ShadowSystem::GetDirectionalLightCascades(void* owner) const {
    for (const auto& entry : m_ShadowMaps) {
        if (entry.lightOwner == owner && entry.type == EShadowMapType::DirectionalCSM) {
            return entry.cascades;
        }
    }
    static std::vector<ShadowCascade> empty;
    return empty;
}

Math::Matrix4 ShadowSystem::GetDirectionalLightViewProjection(void* owner, int cascadeIndex) const {
    for (const auto& entry : m_ShadowMaps) {
        if (entry.lightOwner == owner && entry.type == EShadowMapType::DirectionalCSM) {
            if (cascadeIndex >= 0 && cascadeIndex < static_cast<int>(entry.cascades.size())) {
                return entry.cascades[cascadeIndex].lightViewProjection;
            }
        }
    }
    return Math::Matrix4::Identity();
}

uint32_t ShadowSystem::GetShadowMapTextureID(void* owner) const {
    for (const auto& entry : m_ShadowMaps) {
        if (entry.lightOwner == owner) {
            return entry.shadowMapTextureID;
        }
    }
    return 0;
}

Math::Matrix4 ShadowSystem::GetPointLightViewProjection(void* owner, int face) const {
    // TODO: Return view-projection matrix for specific cubemap face
    return Math::Matrix4();
}

Math::Matrix4 ShadowSystem::GetSpotLightViewProjection(void* owner) const {
    // TODO: Return spot light view-projection matrix
    return Math::Matrix4();
}

} // namespace Luminite
} // namespace LGE

