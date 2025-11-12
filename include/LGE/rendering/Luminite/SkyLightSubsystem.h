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

#include "LGE/rendering/Luminite/LuminiteComponents.h"
#include "LGE/rendering/Texture.h"
#include "LGE/rendering/Shader.h"
#include <memory>
#include <string>
#include <functional>

namespace LGE {
namespace Luminite {

// SkyLightSubsystem - Manages IBL generation and caching
class SkyLightSubsystem {
public:
    SkyLightSubsystem();
    ~SkyLightSubsystem();

    // Initialization
    bool Initialize();
    void Shutdown();

    // SkyLight management
    void RegisterSkyLight(void* owner, SkyLight& skyLight);
    void UnregisterSkyLight(void* owner);
    
    // Baking
    bool BakeIBL(SkyLight& skyLight, std::function<void(float)> progressCallback = nullptr);
    bool IsBaking() const { return m_IsBaking; }
    float GetBakeProgress() const { return m_BakeProgress; }
    
    // Asset loading
    bool LoadCachedIBL(SkyLight& skyLight);
    std::string GetCacheDirectory() const { return m_CacheDirectory; }
    void SetCacheDirectory(const std::string& dir) { m_CacheDirectory = dir; }
    
    // SkyLight registry entry (public for access)
    struct SkyLightEntry {
        void* owner;
        SkyLight* skyLight;
    };
    
    // Get registered SkyLights
    const std::vector<SkyLightEntry>& GetSkyLights() const { return m_SkyLights; }

private:
    // IBL generation steps
    bool ConvertEquirectToCubemap(std::shared_ptr<Texture> equirectTexture, 
                                   std::shared_ptr<Texture> cubemap, 
                                   uint32_t resolution);
    bool GenerateIrradianceMap(std::shared_ptr<Texture> environmentMap,
                               std::shared_ptr<Texture> irradianceMap,
                               uint32_t resolution);
    bool GeneratePrefilterMap(std::shared_ptr<Texture> environmentMap,
                              std::shared_ptr<Texture> prefilterMap,
                              uint32_t resolution);
    bool GenerateBRDFLUT(uint32_t brdfLUTTextureID, uint32_t resolution);
    
    // Asset caching
    bool SaveCubemapToDisk(std::shared_ptr<Texture> cubemap, const std::string& filepath);
    bool LoadCubemapFromDisk(std::shared_ptr<Texture> cubemap, const std::string& filepath);
    bool SaveLUTToDisk(std::shared_ptr<Texture> lut, const std::string& filepath);
    bool LoadLUTFromDisk(std::shared_ptr<Texture> lut, const std::string& filepath);
    
    // Helper: Generate cache path from source path
    std::string GenerateCachePath(const std::string& sourcePath, const std::string& suffix);
    
    // Compute shaders
    std::unique_ptr<Shader> m_EquirectToCubemapShader;
    std::unique_ptr<Shader> m_IrradianceConvolutionShader;
    std::unique_ptr<Shader> m_SpecularPrefilterShader;
    std::unique_ptr<Shader> m_BRDFLUTShader;
    
    // SkyLight registry
    std::vector<SkyLightEntry> m_SkyLights;
    
    // Baking state
    bool m_IsBaking;
    float m_BakeProgress;
    
    // Cache directory
    std::string m_CacheDirectory;
    
    // Initialization state
    bool m_Initialized;
};

} // namespace Luminite
} // namespace LGE

