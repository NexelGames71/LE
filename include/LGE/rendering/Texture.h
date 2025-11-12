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

class Texture {
public:
    Texture();
    ~Texture();

    bool LoadHDR(const std::string& filepath);
    bool LoadEXR(const std::string& filepath);
    bool LoadHDRImage(const std::string& filepath); // Auto-detects HDR or EXR
    bool LoadImageFile(const std::string& filepath); // Load PNG/JPG images
    
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

    uint32_t GetRendererID() const { return m_RendererID; }
    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }
    bool IsCubemap() const { return m_IsCubemap; }
    bool IsHDR() const { return m_IsHDR; }

private:
    uint32_t m_RendererID;
    int m_Width;
    int m_Height;
    bool m_IsHDR;
    bool m_IsCubemap;
};

} // namespace LGE

