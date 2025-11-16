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

#include "LGE/rendering/Lighting.h"
#include "LGE/rendering/ShadowMap.h"
#include "LGE/math/Matrix.h"
#include <vector>
#include <memory>

namespace LGE {

class World;
class Camera;
class GameObject;

class LightSystem {
public:
    LightSystem();
    ~LightSystem();
    
    // Initialize/Shutdown
    bool Initialize();
    void Shutdown();
    
    // Frame lifecycle
    void BeginFrame(World& world);
    void CollectLights(World& world);
    void UploadToGPU();
    
    // Get light data
    const std::vector<LightDataGPU>& GetLightBuffer() const { return m_Lights; }
    
    // Bind lighting buffers for rendering
    void BindLightingBuffers();
    
    // Get GPU buffer IDs
    unsigned int GetFrameUBOID() const { return m_FrameUBOID; }
    unsigned int GetLightBufferID() const { return m_LightBufferID; }
    
    // Shadow mapping
    void RenderShadowMaps(World& world, Camera* camera);
    const DirectionalLightShadow* GetDirectionalShadow() const { return m_DirectionalShadow.IsValid ? &m_DirectionalShadow : nullptr; }
    
    // Cascaded Shadow Maps (CSM) - future
    void RenderCascadedShadowMaps(World& world, Camera* camera);
    const CascadedShadowMap* GetCascadedShadowMap() const { return m_CascadedShadowMap.IsValid ? &m_CascadedShadowMap : nullptr; }
    
    // Shadow map atlas - future
    void InitializeShadowAtlas();
    const ShadowMapAtlas* GetShadowAtlas() const { return m_ShadowAtlas.IsValid ? &m_ShadowAtlas : nullptr; }
    
    // Get shadow caster shader (for rendering to shadow map)
    std::shared_ptr<class Shader> GetShadowCasterShader() const { return m_ShadowCasterShader; }

private:
    void CreateShadowMap();
    void CalculateLightViewProj(const Math::Vector3& lightDir, Camera* camera, Math::Matrix4& outViewProj);
    void RenderSceneToShadowMap(World& world, const Math::Matrix4& lightViewProj);
    
    std::vector<LightDataGPU> m_Lights;
    
    // GPU buffers
    unsigned int m_FrameUBOID;      // Uniform Buffer Object for frame lighting data
    unsigned int m_LightBufferID;   // Shader Storage Buffer Object for light array
    
    // Shadow mapping
    DirectionalLightShadow m_DirectionalShadow;
    
    // Advanced shadow features (future)
    CascadedShadowMap m_CascadedShadowMap;
    ShadowMapAtlas m_ShadowAtlas;
    
    std::shared_ptr<class Shader> m_ShadowCasterShader;
    
    static constexpr int MAX_LIGHTS = 64;
    static constexpr int FRAME_UBO_BINDING = 0;
    static constexpr int LIGHT_SSBO_BINDING = 3; // Changed to match spec
    static constexpr uint32_t SHADOW_MAP_SIZE = 2048;
};

} // namespace LGE
