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

#include <memory>
#include <cstdint>

namespace LGE {

class Shader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;

// Tone mapper types
enum class EToneMapperType : int {
    ACES = 0,
    Hable = 1,
    Reinhard = 2,
    None = 3
};

// Post-processor for tone mapping and exposure
class PostProcessor {
public:
    PostProcessor();
    ~PostProcessor();

    bool Initialize();
    void Shutdown();

    // Render HDR texture to screen with tone mapping
    void Render(uint32_t hdrTextureID, uint32_t width, uint32_t height, float exposure, EToneMapperType toneMapper = EToneMapperType::ACES);
    
    // Render HDR texture to a framebuffer with tone mapping
    void RenderToFramebuffer(uint32_t hdrTextureID, uint32_t targetFramebufferID, uint32_t width, uint32_t height, float exposure, EToneMapperType toneMapper = EToneMapperType::ACES);
    
    bool IsInitialized() const { return m_Initialized; }

    // Get/Set exposure
    void SetExposure(float exposure) { m_Exposure = exposure; }
    float GetExposure() const { return m_Exposure; }

    // Get/Set tone mapper
    void SetToneMapper(EToneMapperType type) { m_ToneMapper = type; }
    EToneMapperType GetToneMapper() const { return m_ToneMapper; }

private:
    void CreateFullscreenQuad();

    std::shared_ptr<Shader> m_ToneMappingShader;
    std::unique_ptr<VertexArray> m_FullscreenQuadVAO;
    std::unique_ptr<VertexBuffer> m_FullscreenQuadVBO;
    std::unique_ptr<IndexBuffer> m_FullscreenQuadIBO;
    
    float m_Exposure;
    EToneMapperType m_ToneMapper;
    bool m_Initialized;
};

} // namespace LGE

