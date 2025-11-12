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

#include "LGE/math/Vector.h"
#include "LGE/math/Matrix.h"
#include <cstdint>
#include <vector>
#include <memory>

namespace LGE {
class Camera;
class Texture;
class Framebuffer;
class Shader;

namespace Luminite {

// Shadow map type
enum class EShadowMapType {
    DirectionalCSM,  // Cascaded Shadow Maps for directional lights
    PointCube,       // Cubemap shadows for point lights
    Spot2D,          // 2D depth map for spot lights
    Rect2D           // 2D depth map for rect area lights
};

// Shadow cascade data
struct ShadowCascade {
    Math::Matrix4 lightViewProjection;  // Light space view-projection matrix
    float splitDistance;                // Distance from camera where this cascade ends
    uint32_t shadowMapTextureID;        // Texture ID for this cascade
    uint32_t resolution;                // Resolution of this cascade
};

// Shadow map entry
struct ShadowMapEntry {
    void* lightOwner;                   // Pointer to owning light
    EShadowMapType type;
    uint32_t shadowMapTextureID;        // Texture ID
    uint32_t resolution;
    bool dirty;                         // Needs update
    
    // For CSM
    std::vector<ShadowCascade> cascades;
    Math::Vector3 lightDirection;       // Light direction for CSM
    
    // For point lights
    uint32_t cubemapTextureID;
    Math::Vector3 lightPosition;        // Light position for point/spot lights
    
    // For spot/rect lights
    Math::Matrix4 lightViewProjection;
    float lightRange;                   // Range for point/spot lights
};

// Shadow system - manages shadow map rendering and sampling
class ShadowSystem {
public:
    ShadowSystem();
    ~ShadowSystem();

    bool Initialize();
    void Shutdown();

    // Update shadow maps (called each frame before main rendering)
    void UpdateShadowMaps(const Camera& camera, float deltaTime);

    // Register shadow-casting lights
    void RegisterDirectionalLightShadow(void* owner, const Math::Vector3& direction, const Math::Vector3& position, int cascadeCount = 4);
    void RegisterPointLightShadow(void* owner, const Math::Vector3& position, float range);
    void RegisterSpotLightShadow(void* owner, const Math::Vector3& position, const Math::Vector3& direction, float range, float innerCone, float outerCone);
    
    void UnregisterLightShadow(void* owner);

    // Get shadow data for shader
    const std::vector<ShadowCascade>& GetDirectionalLightCascades(void* owner) const;
    Math::Matrix4 GetDirectionalLightViewProjection(void* owner, int cascadeIndex = 0) const;
    Math::Matrix4 GetPointLightViewProjection(void* owner, int face) const;
    Math::Matrix4 GetSpotLightViewProjection(void* owner) const;
    
    // Get shadow map texture ID
    uint32_t GetShadowMapTextureID(void* owner) const;

    // Shadow settings
    void SetShadowResolution(uint32_t resolution) { m_ShadowResolution = resolution; }
    uint32_t GetShadowResolution() const { return m_ShadowResolution; }
    
    void SetShadowBias(float bias) { m_ShadowBias = bias; }
    float GetShadowBias() const { return m_ShadowBias; }
    
    void SetShadowNormalBias(float normalBias) { m_ShadowNormalBias = normalBias; }
    float GetShadowNormalBias() const { return m_ShadowNormalBias; }
    
    void SetCascadeDistances(const std::vector<float>& distances);
    const std::vector<float>& GetCascadeDistances() const { return m_CascadeDistances; }

    // Shadow filtering
    void SetPCFEnabled(bool enabled) { m_PCFEnabled = enabled; }
    bool IsPCFEnabled() const { return m_PCFEnabled; }
    
    void SetPCFSize(int size) { m_PCFSize = size; }
    int GetPCFSize() const { return m_PCFSize; }

private:
    void RenderShadowMap(ShadowMapEntry& entry, const Camera& camera);
    void RenderCSM(ShadowMapEntry& entry, const Camera& camera);
    void RenderPointLightShadow(ShadowMapEntry& entry);
    void RenderSpotLightShadow(ShadowMapEntry& entry);
    
    void CalculateCSMSplits(const Camera& camera, const Math::Vector3& lightDir, std::vector<ShadowCascade>& cascades);
    void StabilizeCascade(ShadowCascade& cascade, float texelSize);
    
    std::vector<ShadowMapEntry> m_ShadowMaps;
    
    // Shadow settings
    uint32_t m_ShadowResolution;
    float m_ShadowBias;
    float m_ShadowNormalBias;
    std::vector<float> m_CascadeDistances;
    
    // PCF settings
    bool m_PCFEnabled;
    int m_PCFSize;
    
    // Shadow rendering resources
    std::shared_ptr<Shader> m_ShadowMapShader;
    std::shared_ptr<Shader> m_ShadowMapPointShader;
    
    bool m_Initialized;
};

} // namespace Luminite
} // namespace LGE

