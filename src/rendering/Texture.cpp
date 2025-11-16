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

#include "LGE/rendering/Texture.h"
#include "LGE/core/Log.h"
#include <glad/glad.h>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <vector>
#include <cstring>
#include <cctype>

// Provide minimal zlib-compatible stubs for tinyexr when miniz is disabled
#define TINYEXR_USE_MINIZ 0
// Stub zlib types and functions
typedef unsigned long uLong;
typedef unsigned long uLongf;
typedef unsigned char Bytef;
#define Z_OK 0
#define Z_STREAM_END 1
#define Z_NEED_DICT 2
#define Z_ERRNO (-1)
#define Z_STREAM_ERROR (-2)
#define Z_DATA_ERROR (-3)
#define Z_MEM_ERROR (-4)
#define Z_BUF_ERROR (-5)
#define Z_VERSION_ERROR (-6)

// Stub functions - these won't work for compressed EXR, but will allow uncompressed EXR to load
static inline uLong compressBound(uLong sourceLen) { return sourceLen + (sourceLen >> 12) + (sourceLen >> 14) + 11; }
static inline int compress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen) { return Z_DATA_ERROR; }
static inline int uncompress(Bytef *dest, uLongf *destLen, const Bytef *source, uLong sourceLen) { return Z_DATA_ERROR; }

#define TINYEXR_IMPLEMENTATION
#include "../../third_party/tinyexr.h"

// Include stb_image at global scope (before namespace)
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#endif
#include "../../third_party/stb_image.h"

namespace LGE {

Texture::Texture()
    : m_RendererID(0)
    , m_Width(0)
    , m_Height(0)
    , m_IsHDR(false)
    , m_IsCubemap(false)
    , m_GammaCorrected(true)
    , m_MinFilter(TextureFilter::Linear)
    , m_MagFilter(TextureFilter::Linear)
    , m_WrapS(TextureWrap::Repeat)
    , m_WrapT(TextureWrap::Repeat)
{
}

Texture::~Texture() {
    if (m_RendererID != 0) {
        glDeleteTextures(1, &m_RendererID);
    }
}

// Helper function to read RLE scanline (standard RGBE RLE format)
static bool ReadRLEScanline(std::ifstream& file, unsigned char* scanline, int width) {
    unsigned char byte1, byte2;
    if (!file.read(reinterpret_cast<char*>(&byte1), 1) || !file.read(reinterpret_cast<char*>(&byte2), 1)) {
        return false;
    }
    
    // Check if RLE encoded (first two bytes should be 2, 2)
    if (byte1 != 2 || byte2 != 2) {
        // Not RLE encoded, read directly
        file.seekg(-2, std::ios::cur);
        if (!file.read(reinterpret_cast<char*>(scanline), width * 4)) {
            return false;
        }
        return file.gcount() == width * 4;
    }
    
    // RLE encoded - read the width bytes (should match our width)
    unsigned char widthBytes[2];
    if (!file.read(reinterpret_cast<char*>(widthBytes), 2)) {
        return false;
    }
    
    int scanlineWidth = (widthBytes[0] << 8) | widthBytes[1];
    if (scanlineWidth != width) {
        return false; // Width mismatch
    }
    
    // Read all RLE data for this scanline into a temporary buffer
    std::vector<unsigned char> rleData;
    rleData.reserve(width * 4 * 2);
    
    // Read RLE data until we have decoded all channels
    for (int channel = 0; channel < 4; ++channel) {
        int pixelCount = 0;
        while (pixelCount < width) {
            unsigned char byte;
            if (!file.read(reinterpret_cast<char*>(&byte), 1)) {
                return false;
            }
            
            if (byte > 128) {
                // Run: repeat the next byte (byte - 128) times
                int runLength = byte - 128;
                unsigned char value;
                if (!file.read(reinterpret_cast<char*>(&value), 1)) {
                    return false;
                }
                
                rleData.push_back(byte);
                rleData.push_back(value);
                pixelCount += runLength;
            } else {
                // Non-run: read (byte) bytes directly
                int nonRunLength = byte;
                if (nonRunLength == 0) {
                    return false;
                }
                
                rleData.push_back(byte);
                for (int i = 0; i < nonRunLength; ++i) {
                    unsigned char value;
                    if (!file.read(reinterpret_cast<char*>(&value), 1)) {
                        return false;
                    }
                    rleData.push_back(value);
                }
                pixelCount += nonRunLength;
            }
        }
    }
    
    // Now decode the RLE data into the scanline
    size_t rleIndex = 0;
    for (int channel = 0; channel < 4; ++channel) {
        int pos = channel;
        int pixelCount = 0;
        
        while (pixelCount < width && rleIndex < rleData.size()) {
            unsigned char byte = rleData[rleIndex++];
            
            if (byte > 128) {
                int runLength = byte - 128;
                unsigned char value = rleData[rleIndex++];
                for (int i = 0; i < runLength && pixelCount < width; ++i) {
                    scanline[pos] = value;
                    pos += 4;
                    pixelCount++;
                }
            } else {
                int nonRunLength = byte;
                for (int i = 0; i < nonRunLength && pixelCount < width && rleIndex < rleData.size(); ++i) {
                    scanline[pos] = rleData[rleIndex++];
                    pos += 4;
                    pixelCount++;
                }
            }
        }
    }
    
    return true;
}

// EXR loader using tinyexr
bool Texture::LoadEXR(const std::string& filepath) {
    float* out = nullptr; // width * height * RGBA
    int width, height;
    const char* err = nullptr;
    
    int ret = ::LoadEXR(&out, &width, &height, filepath.c_str(), &err);
    
    if (ret != 0) { // 0 means success in tinyexr
        if (err) {
            Log::Error("Failed to load EXR file: " + filepath + " - " + std::string(err));
        } else {
            Log::Error("Failed to load EXR file: " + filepath);
        }
        return false;
    }
    
    if (!out) {
        Log::Error("LoadEXR returned null data");
        return false;
    }
    
    m_Width = width;
    m_Height = height;
    
    // Convert RGBA to RGB (drop alpha channel)
    std::vector<float> rgbData(m_Width * m_Height * 3);
    for (int i = 0; i < m_Width * m_Height; ++i) {
        rgbData[i * 3] = out[i * 4];     // R
        rgbData[i * 3 + 1] = out[i * 4 + 1]; // G
        rgbData[i * 3 + 2] = out[i * 4 + 2]; // B
    }
    
    free(out); // Free the EXR data
    
    // Create OpenGL texture
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, rgbData.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    m_IsHDR = true;
    Log::Info("Loaded EXR texture: " + filepath + " (" + std::to_string(m_Width) + "x" + std::to_string(m_Height) + ")");
    return true;
}

// Auto-detect and load HDR or EXR
bool Texture::LoadHDRImage(const std::string& filepath) {
    // Check file extension
    std::string ext = filepath.substr(filepath.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "exr") {
        return LoadEXR(filepath);
    } else if (ext == "hdr") {
        return LoadHDR(filepath);
    } else {
        Log::Error("Unsupported HDR format: " + filepath + " (expected .hdr or .exr)");
        return false;
    }
}

// HDR loader for RGBE format with RLE support
bool Texture::LoadHDR(const std::string& filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        Log::Error("Failed to open HDR file: " + filepath);
        return false;
    }

    // Read header
    std::string line;
    std::getline(file, line);
    if (line != "#?RADIANCE" && line != "#?RGBE") {
        Log::Error("Invalid HDR file format: " + filepath);
        return false;
    }

    // Skip header lines until we find the resolution
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }
        if (line.find("Y ") != std::string::npos || line.find("-Y ") != std::string::npos || 
            line.find("+Y ") != std::string::npos) {
            // Parse resolution: "-Y height +X width" or "Y height X width"
            std::istringstream iss(line);
            std::string token;
            int width = 0, height = 0;
            
            while (iss >> token) {
                if (token == "+X" || token == "X") {
                    iss >> width;
                } else if (token == "-Y" || token == "+Y" || token == "Y") {
                    iss >> height;
                }
            }
            
            if (width > 0 && height > 0) {
                m_Width = width;
                m_Height = height;
                break;
            }
        }
    }

    if (m_Width == 0 || m_Height == 0) {
        Log::Error("Failed to parse HDR resolution");
        return false;
    }

    // Skip blank line after resolution
    std::getline(file, line);

    // Read RGBE data (with RLE support)
    std::vector<unsigned char> rgbeData(m_Width * m_Height * 4);
    std::vector<unsigned char> scanline(m_Width * 4);
    
    for (int y = 0; y < m_Height; ++y) {
        if (!ReadRLEScanline(file, scanline.data(), m_Width)) {
            Log::Error("Failed to read HDR scanline " + std::to_string(y) + " of " + std::to_string(m_Height));
            return false;
        }
        
        // Copy scanline to main buffer
        std::memcpy(&rgbeData[y * m_Width * 4], scanline.data(), m_Width * 4);
    }

    // Convert RGBE to float RGB
    std::vector<float> floatData(m_Width * m_Height * 3);
    for (size_t i = 0; i < m_Width * m_Height; ++i) {
        unsigned char r = rgbeData[i * 4];
        unsigned char g = rgbeData[i * 4 + 1];
        unsigned char b = rgbeData[i * 4 + 2];
        unsigned char e = rgbeData[i * 4 + 3];

        if (e == 0) {
            floatData[i * 3] = 0.0f;
            floatData[i * 3 + 1] = 0.0f;
            floatData[i * 3 + 2] = 0.0f;
        } else {
            float exp = std::ldexp(1.0f, static_cast<int>(e) - 128);
            floatData[i * 3] = (r + 0.5f) / 256.0f * exp;
            floatData[i * 3 + 1] = (g + 0.5f) / 256.0f * exp;
            floatData[i * 3 + 2] = (b + 0.5f) / 256.0f * exp;
        }
    }

    // Create OpenGL texture
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_Width, m_Height, 0, GL_RGB, GL_FLOAT, floatData.data());
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);

    m_IsHDR = true;
    Log::Info("Loaded HDR texture: " + filepath + " (" + std::to_string(m_Width) + "x" + std::to_string(m_Height) + ")");
    return true;
}

// Load standard image formats (PNG, JPG, etc.) using stb_image
bool Texture::LoadImageFile(const std::string& filepath) {
    // Undefine Windows LoadImage macro to avoid conflict with Windows.h
    #ifdef LoadImage
    #undef LoadImage
    #endif
    
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4); // Force RGBA
    
    if (!data) {
        Log::Error("Failed to load image: " + filepath + " - " + stbi_failure_reason());
        return false;
    }
    
    m_Width = width;
    m_Height = height;
    
    // Create OpenGL texture
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_2D, m_RendererID);
    
    // Use SRGB format if gamma correction is enabled (default)
    GLenum internalFormat = m_GammaCorrected ? GL_SRGB8_ALPHA8 : GL_RGBA8;
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    // Apply default parameters (will be overridden if Load() is used)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, WrapToGL(m_WrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, WrapToGL(m_WrapT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterToGL(m_MinFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterToGL(m_MagFilter));
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    m_FilePath = filepath;
    
    stbi_image_free(data);
    
    m_IsHDR = false;
    Log::Info("Loaded image texture: " + filepath + " (" + std::to_string(m_Width) + "x" + std::to_string(m_Height) + ")");
    return true;
}

void Texture::Bind(uint32_t slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    if (m_IsCubemap) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
    }
}

void Texture::Unbind() const {
    if (m_IsCubemap) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    } else {
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

void Texture::CreateCubemap(uint32_t resolution, bool isHDR) {
    if (m_RendererID != 0) {
        glDeleteTextures(1, &m_RendererID);
    }
    
    m_Width = resolution;
    m_Height = resolution;
    m_IsHDR = isHDR;
    m_IsCubemap = true;
    
    glGenTextures(1, &m_RendererID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
    
    GLenum internalFormat = isHDR ? GL_RGB16F : GL_RGB8;
    GLenum format = GL_RGB;
    GLenum type = isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
    
    for (uint32_t i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internalFormat,
                     resolution, resolution, 0, format, type, nullptr);
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Texture::CreateCubemapFromData(uint32_t resolution, const float* data, bool isHDR) {
    CreateCubemap(resolution, isHDR);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
    
    GLenum format = GL_RGB;
    GLenum type = isHDR ? GL_FLOAT : GL_UNSIGNED_BYTE;
    int faceSize = resolution * resolution * 3 * (isHDR ? sizeof(float) : sizeof(unsigned char));
    
    for (uint32_t i = 0; i < 6; ++i) {
        glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0,
                       resolution, resolution, format, type,
                       data ? (data + i * faceSize / (isHDR ? sizeof(float) : sizeof(unsigned char))) : nullptr);
    }
    
    GenerateMipmaps();
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void Texture::BindAsImage(uint32_t binding, uint32_t mipLevel, bool write) const {
    GLenum access = write ? GL_WRITE_ONLY : GL_READ_ONLY;
    GLenum format = m_IsHDR ? GL_RGBA16F : GL_RGBA8;
    
    if (m_IsCubemap) {
        // For cubemaps, we bind each face separately in compute shaders
        // This is a simplified version - full implementation would handle layered images
        glBindImageTexture(binding, m_RendererID, mipLevel, GL_TRUE, 0, access, format);
    } else {
        glBindImageTexture(binding, m_RendererID, mipLevel, GL_FALSE, 0, access, format);
    }
}

void Texture::GenerateMipmaps() const {
    if (m_IsCubemap) {
        glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    } else {
        glBindTexture(GL_TEXTURE_2D, m_RendererID);
        glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

// Load from TextureSpec
bool Texture::Load(const TextureSpec& spec) {
    m_FilePath = spec.filepath;
    m_GammaCorrected = spec.gammaCorrected;
    m_MinFilter = spec.minFilter;
    m_MagFilter = spec.magFilter;
    m_WrapS = spec.wrapS;
    m_WrapT = spec.wrapT;
    
    // Determine file type and load
    std::string ext = spec.filepath.substr(spec.filepath.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    bool loaded = false;
    if (ext == "hdr" || ext == "exr") {
        loaded = LoadHDRImage(spec.filepath);
    } else {
        loaded = LoadImageFile(spec.filepath);
    }
    
    if (loaded) {
        ApplyTextureParameters();
        if (spec.generateMipmaps && !m_IsHDR) {
            GenerateMipmaps();
        }
    }
    
    return loaded;
}

// Set texture filtering
void Texture::SetFilter(TextureFilter minFilter, TextureFilter magFilter) {
    m_MinFilter = minFilter;
    m_MagFilter = magFilter;
    if (m_RendererID != 0) {
        ApplyTextureParameters();
    }
}

// Set texture wrapping
void Texture::SetWrap(TextureWrap wrapS, TextureWrap wrapT) {
    m_WrapS = wrapS;
    m_WrapT = wrapT;
    if (m_RendererID != 0) {
        ApplyTextureParameters();
    }
}

// Set gamma correction
void Texture::SetGammaCorrected(bool gammaCorrected) {
    m_GammaCorrected = gammaCorrected;
    // Note: Gamma correction is applied at load time via internal format
    // This flag is stored for reference but doesn't change existing texture
}

// Apply texture parameters to OpenGL
void Texture::ApplyTextureParameters() {
    if (m_RendererID == 0) return;
    
    GLenum target = m_IsCubemap ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    glBindTexture(target, m_RendererID);
    
    // Set filtering
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, FilterToGL(m_MinFilter));
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, FilterToGL(m_MagFilter));
    
    // Set wrapping
    glTexParameteri(target, GL_TEXTURE_WRAP_S, WrapToGL(m_WrapS));
    glTexParameteri(target, GL_TEXTURE_WRAP_T, WrapToGL(m_WrapT));
    if (m_IsCubemap) {
        glTexParameteri(target, GL_TEXTURE_WRAP_R, WrapToGL(m_WrapS)); // Use wrapS for R
    }
    
    glBindTexture(target, 0);
}

// Convert TextureFilter to OpenGL constant
uint32_t Texture::FilterToGL(TextureFilter filter) const {
    switch (filter) {
        case TextureFilter::Nearest: return GL_NEAREST;
        case TextureFilter::Linear: return GL_LINEAR;
        case TextureFilter::NearestMipmapNearest: return GL_NEAREST_MIPMAP_NEAREST;
        case TextureFilter::LinearMipmapNearest: return GL_LINEAR_MIPMAP_NEAREST;
        case TextureFilter::NearestMipmapLinear: return GL_NEAREST_MIPMAP_LINEAR;
        case TextureFilter::LinearMipmapLinear: return GL_LINEAR_MIPMAP_LINEAR;
        default: return GL_LINEAR;
    }
}

// Convert TextureWrap to OpenGL constant
uint32_t Texture::WrapToGL(TextureWrap wrap) const {
    switch (wrap) {
        case TextureWrap::Repeat: return GL_REPEAT;
        case TextureWrap::ClampToEdge: return GL_CLAMP_TO_EDGE;
        case TextureWrap::ClampToBorder: return GL_CLAMP_TO_BORDER;
        case TextureWrap::MirroredRepeat: return GL_MIRRORED_REPEAT;
        default: return GL_REPEAT;
    }
}

} // namespace LGE
