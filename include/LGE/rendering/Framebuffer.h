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

namespace LGE {

// Framebuffer format options
enum class EFramebufferFormat {
    LDR,    // RGBA8 - Low Dynamic Range (legacy)
    HDR     // RGBA16F - High Dynamic Range
};

class Framebuffer {
public:
    Framebuffer(uint32_t width, uint32_t height, EFramebufferFormat format = EFramebufferFormat::LDR);
    ~Framebuffer();

    void Bind() const;
    void Unbind() const;
    void Resize(uint32_t width, uint32_t height);

    uint32_t GetColorAttachmentRendererID() const { return m_ColorAttachment; }
    uint32_t GetRendererID() const { return m_RendererID; }
    uint32_t GetWidth() const { return m_Width; }
    uint32_t GetHeight() const { return m_Height; }
    EFramebufferFormat GetFormat() const { return m_Format; }

private:
    uint32_t m_RendererID;
    uint32_t m_ColorAttachment;
    uint32_t m_DepthAttachment;
    uint32_t m_Width;
    uint32_t m_Height;
    EFramebufferFormat m_Format;

    void Invalidate();
    void Release();
};

} // namespace LGE





