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

#include "LGE/rendering/Luminite/SkyLightSubsystem.h"
#include "LGE/core/Log.h"
#include "LGE/core/FileSystem.h"
#include "LGE/rendering/Texture.h"
#include <glad/glad.h>
#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace LGE {
namespace Luminite {

SkyLightSubsystem::SkyLightSubsystem()
    : m_IsBaking(false)
    , m_BakeProgress(0.0f)
    , m_CacheDirectory("assets/cache/ibl/")
    , m_Initialized(false)
{
}

SkyLightSubsystem::~SkyLightSubsystem() {
    Shutdown();
}

bool SkyLightSubsystem::Initialize() {
    if (m_Initialized) {
        return true;
    }
    
    // Load compute shaders
    std::string equirectSrc = FileSystem::ReadFile("assets/shaders/Luminite/EquirectToCubemap.compute");
    if (equirectSrc.empty()) {
        Log::Error("Failed to load EquirectToCubemap.compute shader");
        return false;
    }
    m_EquirectToCubemapShader = std::make_unique<Shader>(equirectSrc);
    
    std::string irradianceSrc = FileSystem::ReadFile("assets/shaders/Luminite/IrradianceConvolution.compute");
    if (irradianceSrc.empty()) {
        Log::Error("Failed to load IrradianceConvolution.compute shader");
        return false;
    }
    m_IrradianceConvolutionShader = std::make_unique<Shader>(irradianceSrc);
    
    std::string prefilterSrc = FileSystem::ReadFile("assets/shaders/Luminite/SpecularPrefilter.compute");
    if (prefilterSrc.empty()) {
        Log::Error("Failed to load SpecularPrefilter.compute shader");
        return false;
    }
    m_SpecularPrefilterShader = std::make_unique<Shader>(prefilterSrc);
    
    std::string brdfLUTSrc = FileSystem::ReadFile("assets/shaders/Luminite/BRDFLUT.compute");
    if (brdfLUTSrc.empty()) {
        Log::Error("Failed to load BRDFLUT.compute shader");
        return false;
    }
    m_BRDFLUTShader = std::make_unique<Shader>(brdfLUTSrc);
    
    m_Initialized = true;
    Log::Info("SkyLightSubsystem initialized");
    return true;
}

void SkyLightSubsystem::Shutdown() {
    if (!m_Initialized) {
        return;
    }
    
    m_EquirectToCubemapShader.reset();
    m_IrradianceConvolutionShader.reset();
    m_SpecularPrefilterShader.reset();
    m_BRDFLUTShader.reset();
    
    m_SkyLights.clear();
    m_Initialized = false;
    Log::Info("SkyLightSubsystem shut down");
}

void SkyLightSubsystem::RegisterSkyLight(void* owner, SkyLight& skyLight) {
    // Check if already registered
    for (auto& entry : m_SkyLights) {
        if (entry.owner == owner) {
            entry.skyLight = &skyLight;
            return;
        }
    }
    
    SkyLightEntry entry;
    entry.owner = owner;
    entry.skyLight = &skyLight;
    m_SkyLights.push_back(entry);
}

void SkyLightSubsystem::UnregisterSkyLight(void* owner) {
    m_SkyLights.erase(
        std::remove_if(m_SkyLights.begin(), m_SkyLights.end(),
            [owner](const SkyLightEntry& entry) { return entry.owner == owner; }),
        m_SkyLights.end());
}

bool SkyLightSubsystem::BakeIBL(SkyLight& skyLight, std::function<void(float)> progressCallback) {
    if (m_IsBaking) {
        Log::Warn("IBL baking already in progress");
        return false;
    }
    
    if (!m_Initialized) {
        Log::Error("SkyLightSubsystem not initialized");
        return false;
    }
    
    if (skyLight.hdrTexturePath.empty()) {
        Log::Error("No HDR texture path specified for SkyLight");
        return false;
    }
    
    m_IsBaking = true;
    m_BakeProgress = 0.0f;
    
    // Step 1: Load HDR equirectangular texture
    auto equirectTexture = std::make_shared<Texture>();
    if (!equirectTexture->LoadHDRImage(skyLight.hdrTexturePath)) {
        Log::Error("Failed to load HDR texture: " + skyLight.hdrTexturePath);
        m_IsBaking = false;
        return false;
    }
    
    if (progressCallback) progressCallback(0.1f);
    m_BakeProgress = 0.1f;
    
    // Step 2: Convert equirectangular to cubemap
    auto environmentMap = std::make_shared<Texture>();
    environmentMap->CreateCubemap(skyLight.environmentMapResolution, true);
    
    if (!ConvertEquirectToCubemap(equirectTexture, environmentMap, skyLight.environmentMapResolution)) {
        Log::Error("Failed to convert equirectangular to cubemap");
        m_IsBaking = false;
        return false;
    }
    
    skyLight.environmentMapID = environmentMap->GetRendererID();
    if (progressCallback) progressCallback(0.3f);
    m_BakeProgress = 0.3f;
    
    // Step 3: Generate irradiance map
    auto irradianceMap = std::make_shared<Texture>();
    irradianceMap->CreateCubemap(skyLight.irradianceResolution, true);
    
    if (!GenerateIrradianceMap(environmentMap, irradianceMap, skyLight.irradianceResolution)) {
        Log::Error("Failed to generate irradiance map");
        m_IsBaking = false;
        return false;
    }
    
    skyLight.irradianceMapID = irradianceMap->GetRendererID();
    if (progressCallback) progressCallback(0.5f);
    m_BakeProgress = 0.5f;
    
    // Step 4: Generate prefilter map
    auto prefilterMap = std::make_shared<Texture>();
    prefilterMap->CreateCubemap(skyLight.prefilterResolution, true);
    
    if (!GeneratePrefilterMap(environmentMap, prefilterMap, skyLight.prefilterResolution)) {
        Log::Error("Failed to generate prefilter map");
        m_IsBaking = false;
        return false;
    }
    
    skyLight.prefilterMapID = prefilterMap->GetRendererID();
    if (progressCallback) progressCallback(0.8f);
    m_BakeProgress = 0.8f;
    
    // Step 5: Generate BRDF LUT
    glGenTextures(1, &skyLight.brdfLUTID);
    glBindTexture(GL_TEXTURE_2D, skyLight.brdfLUTID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, skyLight.brdfLUTResolution, skyLight.brdfLUTResolution, 
                 0, GL_RG, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    if (!GenerateBRDFLUT(skyLight.brdfLUTID, skyLight.brdfLUTResolution)) {
        Log::Error("Failed to generate BRDF LUT");
        m_IsBaking = false;
        return false;
    }
    
    if (progressCallback) progressCallback(0.95f);
    m_BakeProgress = 0.95f;
    
    // Step 6: Cache to disk (optional, for now just mark as complete)
    skyLight.needsBaking = false;
    skyLight.bakeProgress = 1.0f;
    
    if (progressCallback) progressCallback(1.0f);
    m_BakeProgress = 1.0f;
    
    m_IsBaking = false;
    Log::Info("IBL baking completed successfully");
    
    return true;
}

bool SkyLightSubsystem::LoadCachedIBL(SkyLight& skyLight) {
    // TODO: Load cached IBL assets from disk
    return false;
}

std::string SkyLightSubsystem::GenerateCachePath(const std::string& sourcePath, const std::string& suffix) {
    // Generate cache path: cache_dir/hash_suffix.ext
    std::string filename = sourcePath.substr(sourcePath.find_last_of("/\\") + 1);
    std::string nameWithoutExt = filename.substr(0, filename.find_last_of("."));
    return m_CacheDirectory + nameWithoutExt + "_" + suffix + ".hdr";
}

// Helper: Create temporary 2D texture for compute shader output
static uint32_t CreateTemp2DTexture(uint32_t width, uint32_t height, bool isHDR) {
    uint32_t textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    GLenum internalFormat = isHDR ? GL_RGBA16F : GL_RGBA8;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, GL_RGBA, 
                 isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    return textureID;
}

// Helper: Copy 2D texture to cubemap face
static void Copy2DToCubemapFace(uint32_t srcTexture, uint32_t cubemap, uint32_t face, uint32_t resolution) {
    // Use glCopyImageSubData for efficient copying
    glCopyImageSubData(
        srcTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
        cubemap, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, 0, 0, 0,
        resolution, resolution, 1
    );
}

bool SkyLightSubsystem::ConvertEquirectToCubemap(std::shared_ptr<Texture> equirectTexture, 
                                                  std::shared_ptr<Texture> cubemap, 
                                                  uint32_t resolution) {
    if (!m_EquirectToCubemapShader || m_EquirectToCubemapShader->GetRendererID() == 0) {
        Log::Error("EquirectToCubemap shader not loaded");
        return false;
    }
    
    // Create temporary 2D texture for compute shader output
    uint32_t tempTexture = CreateTemp2DTexture(resolution, resolution, true);
    
    m_EquirectToCubemapShader->Bind();
    
    // Bind input equirectangular texture
    equirectTexture->Bind(0);
    
    // Process each cubemap face
    for (int face = 0; face < 6; ++face) {
        // Bind temporary texture as image for writing
        glBindImageTexture(0, tempTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        
        // Set uniforms
        m_EquirectToCubemapShader->SetUniform1f("u_Resolution", static_cast<float>(resolution));
        m_EquirectToCubemapShader->SetUniform1i("u_Face", face);
        
        // Dispatch compute shader
        uint32_t numGroupsX = (resolution + 31) / 32;
        uint32_t numGroupsY = (resolution + 31) / 32;
        m_EquirectToCubemapShader->Dispatch(numGroupsX, numGroupsY, 1);
        
        // Wait for compute shader to finish
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        // Copy temporary texture to cubemap face
        Copy2DToCubemapFace(tempTexture, cubemap->GetRendererID(), face, resolution);
    }
    
    // Generate mipmaps for cubemap
    cubemap->GenerateMipmaps();
    
    // Cleanup
    glDeleteTextures(1, &tempTexture);
    m_EquirectToCubemapShader->Unbind();
    
    return true;
}

bool SkyLightSubsystem::GenerateIrradianceMap(std::shared_ptr<Texture> environmentMap,
                                              std::shared_ptr<Texture> irradianceMap,
                                              uint32_t resolution) {
    if (!m_IrradianceConvolutionShader || m_IrradianceConvolutionShader->GetRendererID() == 0) {
        Log::Error("IrradianceConvolution shader not loaded");
        return false;
    }
    
    // Create temporary 2D texture for compute shader output
    uint32_t tempTexture = CreateTemp2DTexture(resolution, resolution, true);
    
    m_IrradianceConvolutionShader->Bind();
    
    // Bind input environment cubemap
    environmentMap->Bind(0);
    
    // Process each cubemap face
    for (int face = 0; face < 6; ++face) {
        // Bind temporary texture as image for writing
        glBindImageTexture(0, tempTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
        
        // Set uniforms
        m_IrradianceConvolutionShader->SetUniform1f("u_Resolution", static_cast<float>(resolution));
        m_IrradianceConvolutionShader->SetUniform1i("u_Face", face);
        
        // Dispatch compute shader
        uint32_t numGroupsX = (resolution + 7) / 8;
        uint32_t numGroupsY = (resolution + 7) / 8;
        m_IrradianceConvolutionShader->Dispatch(numGroupsX, numGroupsY, 1);
        
        // Wait for compute shader to finish
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        
        // Copy temporary texture to cubemap face
        Copy2DToCubemapFace(tempTexture, irradianceMap->GetRendererID(), face, resolution);
    }
    
    // Cleanup
    glDeleteTextures(1, &tempTexture);
    m_IrradianceConvolutionShader->Unbind();
    
    return true;
}

bool SkyLightSubsystem::GeneratePrefilterMap(std::shared_ptr<Texture> environmentMap,
                                              std::shared_ptr<Texture> prefilterMap,
                                              uint32_t resolution) {
    if (!m_SpecularPrefilterShader || m_SpecularPrefilterShader->GetRendererID() == 0) {
        Log::Error("SpecularPrefilter shader not loaded");
        return false;
    }
    
    // Create temporary 2D texture for compute shader output
    uint32_t tempTexture = CreateTemp2DTexture(resolution, resolution, true);
    
    m_SpecularPrefilterShader->Bind();
    
    // Bind input environment cubemap
    environmentMap->Bind(0);
    
    // Generate mip levels (roughness levels)
    const uint32_t maxMipLevels = 5;
    for (uint32_t mip = 0; mip < maxMipLevels; ++mip) {
        uint32_t mipResolution = resolution * std::pow(0.5f, static_cast<float>(mip));
        if (mipResolution < 1) break;
        
        float roughness = static_cast<float>(mip) / static_cast<float>(maxMipLevels - 1);
        roughness = std::max(roughness, 0.05f);  // Clamp to avoid division by zero
        
        // Resize temp texture if needed
        if (mipResolution != resolution) {
            glDeleteTextures(1, &tempTexture);
            tempTexture = CreateTemp2DTexture(mipResolution, mipResolution, true);
        }
        
        // Process each cubemap face for this mip level
        for (int face = 0; face < 6; ++face) {
            // Bind temporary texture as image for writing
            glBindImageTexture(0, tempTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            
            // Set uniforms
            m_SpecularPrefilterShader->SetUniform1f("u_Resolution", static_cast<float>(mipResolution));
            m_SpecularPrefilterShader->SetUniform1f("u_Roughness", roughness);
            m_SpecularPrefilterShader->SetUniform1i("u_Face", face);
            
            // Dispatch compute shader
            uint32_t numGroupsX = (mipResolution + 7) / 8;
            uint32_t numGroupsY = (mipResolution + 7) / 8;
            m_SpecularPrefilterShader->Dispatch(numGroupsX, numGroupsY, 1);
            
            // Wait for compute shader to finish
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            
            // Copy temporary texture to cubemap face mip level
            glCopyImageSubData(
                tempTexture, GL_TEXTURE_2D, 0, 0, 0, 0,
                prefilterMap->GetRendererID(), GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, mip, 0, 0, 0,
                mipResolution, mipResolution, 1
            );
        }
    }
    
    // Cleanup
    glDeleteTextures(1, &tempTexture);
    m_SpecularPrefilterShader->Unbind();
    
    return true;
}

bool SkyLightSubsystem::GenerateBRDFLUT(uint32_t brdfLUTTextureID, uint32_t resolution) {
    if (!m_BRDFLUTShader || m_BRDFLUTShader->GetRendererID() == 0) {
        Log::Error("BRDFLUT shader not loaded");
        return false;
    }
    
    m_BRDFLUTShader->Bind();
    
    // Bind BRDF LUT texture as image for writing
    glBindImageTexture(0, brdfLUTTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG16F);
    
    // Set uniforms
    m_BRDFLUTShader->SetUniform1f("u_Resolution", static_cast<float>(resolution));
    
    // Dispatch compute shader
    uint32_t numGroupsX = (resolution + 31) / 32;
    uint32_t numGroupsY = (resolution + 31) / 32;
    m_BRDFLUTShader->Dispatch(numGroupsX, numGroupsY, 1);
    
    // Wait for compute shader to finish
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    m_BRDFLUTShader->Unbind();
    
    return true;
}

bool SkyLightSubsystem::SaveCubemapToDisk(std::shared_ptr<Texture> cubemap, const std::string& filepath) {
    // TODO: Implement
    return false;
}

bool SkyLightSubsystem::LoadCubemapFromDisk(std::shared_ptr<Texture> cubemap, const std::string& filepath) {
    // TODO: Implement
    return false;
}

bool SkyLightSubsystem::SaveLUTToDisk(std::shared_ptr<Texture> lut, const std::string& filepath) {
    // TODO: Implement
    return false;
}

bool SkyLightSubsystem::LoadLUTFromDisk(std::shared_ptr<Texture> lut, const std::string& filepath) {
    // TODO: Implement
    return false;
}

} // namespace Luminite
} // namespace LGE

