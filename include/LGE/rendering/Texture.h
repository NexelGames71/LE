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

#include <cstdint>
#include <string>

namespace LGE {

// Texture filtering modes
enum class TextureFilter {
    Nearest = 0,
    Linear = 1,
    NearestMipmapNearest = 2,
    LinearMipmapNearest = 3,
    NearestMipmapLinear = 4,
    LinearMipmapLinear = 5
};

// Texture wrapping modes
enum class TextureWrap {
    Repeat = 0,
    ClampToEdge = 1,
    ClampToBorder = 2,
    MirroredRepeat = 3
};

// Texture specification for loading
struct TextureSpec {
    std::string filepath;
    bool gammaCorrected = true;  // SRGB/Gamma correction
    TextureFilter minFilter = TextureFilter::Linear;
    TextureFilter magFilter = TextureFilter::Linear;
    TextureWrap wrapS = TextureWrap::Repeat;
    TextureWrap wrapT = TextureWrap::Repeat;
    bool generateMipmaps = true;
};

class Texture {
public:
    Texture();
    ~Texture();

    // Load from file with specification
    bool Load(const TextureSpec& spec);
    
    // Legacy load methods (for backward compatibility)
    bool LoadHDR(const std::string& filepath);
    bool LoadEXR(const std::string& filepath);
    bool LoadHDRImage(const std::string& filepath); // Auto-detects HDR or EXR
    bool LoadImageFile(const std::string& filepath); // Load PNG/JPG images
    
    // Set texture parameters
    void SetFilter(TextureFilter minFilter, TextureFilter magFilter);
    void SetWrap(TextureWrap wrapS, TextureWrap wrapT);
    void SetGammaCorrected(bool gammaCorrected);
    
    // Cubemap creation
    void CreateCubemap(uint32_t resolution, bool isHDR = true);
    void CreateCubemapFromData(uint32_t resolution, const float* data, bool isHDR = true);
    
    // 2D Array texture (for compute shader processing)
    void Create2DArray(uint32_t width, uint32_t height, uint32_t layers, bool isHDR = true);
    void Copy2DArrayToCubemap(const Texture& arrayTexture);
    
    // Bind for rendering
    void Bind(uint32_t slot = 0) const;
    void Unbind() const;
    
    // Bind as image for compute shaders
    void BindAsImage(uint32_t binding, uint32_t mipLevel = 0, bool write = true) const;
    
    // Generate mipmaps
    void GenerateMipmaps() const;

    // Getters
    uint32_t GetRendererID() const { return m_RendererID; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    bool IsCubemap() const { return m_IsCubemap; }
    bool IsHDR() const { return m_IsHDR; }
    bool IsGammaCorrected() const { return m_GammaCorrected; }
    TextureFilter GetMinFilter() const { return m_MinFilter; }
    TextureFilter GetMagFilter() const { return m_MagFilter; }
    TextureWrap GetWrapS() const { return m_WrapS; }
    TextureWrap GetWrapT() const { return m_WrapT; }
    const std::string& GetFilePath() const { return m_FilePath; }

private:
    // Internal helper to apply texture parameters
    void ApplyTextureParameters();
    
    // Convert enums to OpenGL constants
    uint32_t FilterToGL(TextureFilter filter) const;
    uint32_t WrapToGL(TextureWrap wrap) const;

    uint32_t m_RendererID;
    int m_Width;
    int m_Height;
    bool m_IsHDR;
    bool m_IsCubemap;
    bool m_GammaCorrected;
    TextureFilter m_MinFilter;
    TextureFilter m_MagFilter;
    TextureWrap m_WrapS;
    TextureWrap m_WrapT;
    std::string m_FilePath;
};

} // namespace LGE

