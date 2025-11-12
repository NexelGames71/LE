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

#pragma once

#include "LGE/rendering/Luminite/LuminiteDataContracts.h"
#include "LGE/rendering/Luminite/LuminiteComponents.h"
#include "LGE/rendering/Luminite/LuminiteFeatureFlags.h"
#include <vector>
#include <memory>
#include <cstdint>

namespace LGE {
class Camera;

namespace Luminite {

// Light registry entry
struct LightEntry {
    void* owner;  // Pointer to owning GameObject or component
    LightType type;
    LightGPU gpuData;
    bool dirty;  // Needs GPU update
};

// LuminiteSubsystem - Engine-side manager for lighting system
class LuminiteSubsystem {
public:
    LuminiteSubsystem();
    ~LuminiteSubsystem();

    // Initialization and shutdown
    bool Initialize();
    void Shutdown();

    // Update (called each frame)
    void Update(float deltaTime);
    
    // Light registration
    void RegisterDirectionalLight(void* owner, const DirectionalLight& light, const Math::Vector3& position, const Math::Vector3& direction);
    void RegisterPointLight(void* owner, const PointLight& light, const Math::Vector3& position);
    void RegisterSpotLight(void* owner, const SpotLight& light, const Math::Vector3& position, const Math::Vector3& direction);
    void UnregisterLight(void* owner);
    
    // Environment
    void SetEnvironment(const LuminiteEnvironment& env) { m_Environment = env; }
    const LuminiteEnvironment& GetEnvironment() const { return m_Environment; }
    
    // Feature flags
    void SetFeatureFlags(uint32_t flags) { m_FeatureFlags = flags; }
    uint32_t GetFeatureFlags() const { return m_FeatureFlags; }
    bool IsFeatureEnabled(FeatureFlag flag) const { return HasFeature(m_FeatureFlags, flag); }
    
    // GPU data access
    const std::vector<LightGPU>& GetLightsGPU() const { return m_LightsGPU; }
    const FrameLightingUBO& GetFrameLightingUBO() const { return m_FrameLightingUBO; }
    
    // Update frame lighting UBO (called with camera data)
    void UpdateFrameLightingUBO(const LGE::Camera& camera);
    
    // Exposure system
    void UpdateExposure(float deltaTime);
    float GetCurrentExposure() const { return m_CurrentExposure; }
    
    // Shader hot-reload
    void HotReloadShaders();
    
    // Precompute jobs (placeholder for future async work)
    void TickPrecomputeJobs(float deltaTime);

private:
    // Light registries
    std::vector<LightEntry> m_Lights;
    std::vector<LightGPU> m_LightsGPU;  // GPU-ready data
    
    // Environment
    LuminiteEnvironment m_Environment;
    
    // Frame lighting data
    FrameLightingUBO m_FrameLightingUBO;
    
    // Feature flags
    uint32_t m_FeatureFlags;
    
    // GPU buffers (to be implemented)
    uint32_t m_LightBufferID;  // OpenGL buffer ID for lights
    uint32_t m_FrameUBOID;     // OpenGL UBO ID for frame data
    
    // Initialization state
    bool m_Initialized;
    
    // Exposure system
    float m_CurrentExposure;
    float m_AverageLuminance;
    std::vector<float> m_LuminanceHistory;  // For auto exposure averaging
    
    // Helper methods
    void UpdateLightGPUData();
    void CreateGPUBuffers();
    void DestroyGPUBuffers();
    void UpdateGPUBuffers();
};

} // namespace Luminite
} // namespace LGE

