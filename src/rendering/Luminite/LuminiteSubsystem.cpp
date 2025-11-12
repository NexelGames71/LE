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

#include "LGE/rendering/Luminite/LuminiteSubsystem.h"
#include "LGE/rendering/Camera.h"
#include "LGE/core/Log.h"
#include <glad/glad.h>
#include <algorithm>
#include <cmath>

namespace LGE {
namespace Luminite {

LuminiteSubsystem::LuminiteSubsystem()
    : m_FeatureFlags(0)
    , m_LightBufferID(0)
    , m_FrameUBOID(0)
    , m_Initialized(false)
    , m_CurrentExposure(1.0f)
    , m_AverageLuminance(0.18f)  // Middle gray
{
    // Initialize with default feature flags
    m_FeatureFlags = static_cast<uint32_t>(FeatureFlag::LUMA_LIT);
    
    // Initialize default environment
    m_Environment = LuminiteEnvironment();
    
    // Initialize luminance history for auto exposure
    m_LuminanceHistory.resize(64, 0.18f);  // 64 samples, initialized to middle gray
}

LuminiteSubsystem::~LuminiteSubsystem() {
    Shutdown();
}

bool LuminiteSubsystem::Initialize() {
    if (m_Initialized) {
        return true;
    }
    
    CreateGPUBuffers();
    
    m_Initialized = true;
    Log::Info("LuminiteSubsystem initialized");
    return true;
}

void LuminiteSubsystem::Shutdown() {
    if (!m_Initialized) {
        return;
    }
    
    DestroyGPUBuffers();
    m_Lights.clear();
    m_LightsGPU.clear();
    
    m_Initialized = false;
    Log::Info("LuminiteSubsystem shut down");
}

void LuminiteSubsystem::Update(float deltaTime) {
    if (!m_Initialized) {
        return;
    }
    
    // Update exposure
    UpdateExposure(deltaTime);
    
    // Update light GPU data if needed
    UpdateLightGPUData();
    UpdateGPUBuffers();
    
    // Tick precompute jobs
    TickPrecomputeJobs(deltaTime);
}

void LuminiteSubsystem::RegisterDirectionalLight(void* owner, const DirectionalLight& light, const Math::Vector3& position, const Math::Vector3& direction) {
    if (!light.enabled) {
        return;
    }
    
    LightEntry entry;
    entry.owner = owner;
    entry.type = LightType::Directional;
    entry.dirty = true;
    
    // Fill GPU data
    entry.gpuData.position = position;
    entry.gpuData.direction = Math::Vector3(
        direction.x / std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z),
        direction.y / std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z),
        direction.z / std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z)
    );
    entry.gpuData.color = light.color;
    entry.gpuData.intensity = light.intensity;
    entry.gpuData.range = 0.0f;  // Directional lights have infinite range
    entry.gpuData.innerCone = 0.0f;
    entry.gpuData.outerCone = 0.0f;
    entry.gpuData.type = static_cast<uint32_t>(LightType::Directional);
    entry.gpuData.flags = light.castShadows ? static_cast<uint32_t>(LightFlags::CastShadows) : 0;
    
    m_Lights.push_back(entry);
    UpdateLightGPUData();
}

void LuminiteSubsystem::RegisterPointLight(void* owner, const PointLight& light, const Math::Vector3& position) {
    if (!light.enabled) {
        return;
    }
    
    LightEntry entry;
    entry.owner = owner;
    entry.type = LightType::Point;
    entry.dirty = true;
    
    // Fill GPU data
    entry.gpuData.position = position;
    entry.gpuData.direction = Math::Vector3(0.0f, 0.0f, 0.0f);
    entry.gpuData.color = light.color;
    entry.gpuData.intensity = light.intensity;
    entry.gpuData.range = light.range;
    entry.gpuData.innerCone = 0.0f;
    entry.gpuData.outerCone = 0.0f;
    entry.gpuData.type = static_cast<uint32_t>(LightType::Point);
    entry.gpuData.flags = light.castShadows ? static_cast<uint32_t>(LightFlags::CastShadows) : 0;
    
    m_Lights.push_back(entry);
    UpdateLightGPUData();
}

void LuminiteSubsystem::RegisterSpotLight(void* owner, const SpotLight& light, const Math::Vector3& position, const Math::Vector3& direction) {
    if (!light.enabled) {
        return;
    }
    
    LightEntry entry;
    entry.owner = owner;
    entry.type = LightType::Spot;
    entry.dirty = true;
    
    // Normalize direction
    float dirLen = std::sqrt(direction.x * direction.x + direction.y * direction.y + direction.z * direction.z);
    Math::Vector3 normalizedDir = Math::Vector3(
        direction.x / dirLen,
        direction.y / dirLen,
        direction.z / dirLen
    );
    
    // Fill GPU data
    entry.gpuData.position = position;
    entry.gpuData.direction = normalizedDir;
    entry.gpuData.color = light.color;
    entry.gpuData.intensity = light.intensity;
    entry.gpuData.range = light.range;
    entry.gpuData.innerCone = std::cos(light.innerConeAngle * 3.14159f / 180.0f);
    entry.gpuData.outerCone = std::cos(light.outerConeAngle * 3.14159f / 180.0f);
    entry.gpuData.type = static_cast<uint32_t>(LightType::Spot);
    entry.gpuData.flags = light.castShadows ? static_cast<uint32_t>(LightFlags::CastShadows) : 0;
    
    m_Lights.push_back(entry);
    UpdateLightGPUData();
}

void LuminiteSubsystem::UnregisterLight(void* owner) {
    m_Lights.erase(
        std::remove_if(m_Lights.begin(), m_Lights.end(),
            [owner](const LightEntry& entry) { return entry.owner == owner; }),
        m_Lights.end()
    );
    UpdateLightGPUData();
}

void LuminiteSubsystem::UpdateFrameLightingUBO(const LGE::Camera& camera) {
    // Update view/projection matrices
    m_FrameLightingUBO.viewMatrix = camera.GetViewMatrix();
    m_FrameLightingUBO.projectionMatrix = camera.GetProjectionMatrix();
    m_FrameLightingUBO.viewProjectionMatrix = camera.GetViewProjectionMatrix();
    
    // Update camera position
    m_FrameLightingUBO.cameraPosition = camera.GetPosition();
    
    // Update exposure (computed by UpdateExposure)
    m_FrameLightingUBO.exposure = m_CurrentExposure;
    
    // Update ambient
    m_FrameLightingUBO.ambientColor = m_Environment.ambientColor;
    m_FrameLightingUBO.ambientIntensity = m_Environment.ambientIntensity;
    
    // Update feature flags
    m_FrameLightingUBO.featureFlags = m_FeatureFlags;
    
    // Count lights by type
    m_FrameLightingUBO.directionalLightCount = 0;
    m_FrameLightingUBO.pointLightCount = 0;
    m_FrameLightingUBO.spotLightCount = 0;
    
    for (const auto& light : m_LightsGPU) {
        if (light.type == static_cast<uint32_t>(LightType::Directional)) {
            m_FrameLightingUBO.directionalLightCount++;
        } else if (light.type == static_cast<uint32_t>(LightType::Point)) {
            m_FrameLightingUBO.pointLightCount++;
        } else if (light.type == static_cast<uint32_t>(LightType::Spot)) {
            m_FrameLightingUBO.spotLightCount++;
        }
    }
    
    m_FrameLightingUBO.totalLightCount = static_cast<uint32_t>(m_LightsGPU.size());
    
    // Update shadow settings from environment
    m_FrameLightingUBO.shadowBias = m_Environment.shadowBias;
    m_FrameLightingUBO.shadowNormalBias = m_Environment.shadowNormalBias;
    m_FrameLightingUBO.shadowDistance = m_Environment.shadowDistance;
}

void LuminiteSubsystem::HotReloadShaders() {
    // Placeholder for shader hot-reload
    // This will reload lighting shaders at runtime
    Log::Info("Luminite: Hot-reloading shaders...");
    // TODO: Implement shader reloading
}

void LuminiteSubsystem::UpdateExposure(float deltaTime) {
    const ExposureSettings& exposureSettings = m_Environment.exposure;
    
    if (exposureSettings.mode == ExposureMode::Manual) {
        // Manual exposure: calculate from EV100, shutter, and ISO
        // EV100 = log2((shutter * ISO) / 100.0)
        // exposure = 1.0 / (shutter * ISO / 100.0)
        // Or use EV100 directly: exposure = pow(2.0, EV100)
        
        if (exposureSettings.ev100 != 0.0f) {
            // Use EV100 directly
            m_CurrentExposure = std::pow(2.0f, exposureSettings.ev100);
        } else {
            // Calculate from shutter and ISO
            float ev100 = std::log2((exposureSettings.shutter * exposureSettings.iso) / 100.0f);
            m_CurrentExposure = std::pow(2.0f, ev100);
        }
        
        // Clamp to min/max
        m_CurrentExposure = std::max(exposureSettings.minExposure, 
                                     std::min(exposureSettings.maxExposure, m_CurrentExposure));
        
        // Update exposure in settings
        m_Environment.exposure.exposure = m_CurrentExposure;
    } else {
        // Auto exposure: interpolate towards target luminance based on average luminance
        // For now, we'll use a simple approach with the average luminance
        // In a full implementation, this would use a log-luminance histogram from the rendered scene
        
        float targetExposure = exposureSettings.targetLuma / std::max(m_AverageLuminance, 0.001f);
        
        // Smooth interpolation
        float speed = exposureSettings.autoExposureSpeed * deltaTime;
        m_CurrentExposure = m_CurrentExposure * (1.0f - speed) + targetExposure * speed;
        
        // Clamp to min/max exposure
        m_CurrentExposure = std::max(exposureSettings.minExposure, 
                                     std::min(exposureSettings.maxExposure, m_CurrentExposure));
        
        // Update exposure in settings
        m_Environment.exposure.exposure = m_CurrentExposure;
    }
}

void LuminiteSubsystem::TickPrecomputeJobs(float deltaTime) {
    // Placeholder for async precompute jobs
    // This will handle IBL precomputation, reflection probe updates, etc.
    // TODO: Implement precompute job system
}

void LuminiteSubsystem::UpdateLightGPUData() {
    m_LightsGPU.clear();
    m_LightsGPU.reserve(m_Lights.size());
    
    for (const auto& entry : m_Lights) {
        m_LightsGPU.push_back(entry.gpuData);
    }
}

void LuminiteSubsystem::CreateGPUBuffers() {
    // Create light buffer (SSBO or UBO)
    glGenBuffers(1, &m_LightBufferID);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightBufferID);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(LightGPU) * 256, nullptr, GL_DYNAMIC_DRAW);  // Max 256 lights
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    
    // Create frame lighting UBO
    glGenBuffers(1, &m_FrameUBOID);
    glBindBuffer(GL_UNIFORM_BUFFER, m_FrameUBOID);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(FrameLightingUBO), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void LuminiteSubsystem::DestroyGPUBuffers() {
    if (m_LightBufferID != 0) {
        glDeleteBuffers(1, &m_LightBufferID);
        m_LightBufferID = 0;
    }
    
    if (m_FrameUBOID != 0) {
        glDeleteBuffers(1, &m_FrameUBOID);
        m_FrameUBOID = 0;
    }
}

void LuminiteSubsystem::UpdateGPUBuffers() {
    // Update light buffer
    if (m_LightBufferID != 0 && !m_LightsGPU.empty()) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_LightBufferID);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(LightGPU) * m_LightsGPU.size(), m_LightsGPU.data());
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    }
    
    // Update frame UBO
    if (m_FrameUBOID != 0) {
        glBindBuffer(GL_UNIFORM_BUFFER, m_FrameUBOID);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(FrameLightingUBO), &m_FrameLightingUBO);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }
}

} // namespace Luminite
} // namespace LGE

