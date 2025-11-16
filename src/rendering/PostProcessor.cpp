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

#include "LGE/rendering/PostProcessor.h"
#include "LGE/rendering/Shader.h"
#include "LGE/rendering/VertexArray.h"
#include "LGE/rendering/VertexBuffer.h"
#include "LGE/rendering/IndexBuffer.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <glad/glad.h>

namespace LGE {

PostProcessor::PostProcessor()
    : m_Exposure(1.0f)
    , m_ToneMapper(EToneMapperType::ACES)
    , m_Initialized(false)
{
}

PostProcessor::~PostProcessor() {
    Shutdown();
}

bool PostProcessor::Initialize() {
    if (m_Initialized) {
        return true;
    }

    // Load tone mapping shader
    std::string vertSource = FileSystem::ReadFile("assets/shaders/PostProcess/ToneMapping.vert");
    std::string fragSource = FileSystem::ReadFile("assets/shaders/PostProcess/ToneMapping.frag");

    if (vertSource.empty() || fragSource.empty()) {
        Log::Error("Failed to load tone mapping shader files!");
        return false;
    }

    m_ToneMappingShader = std::make_shared<Shader>(vertSource, fragSource);
    if (m_ToneMappingShader->GetRendererID() == 0) {
        Log::Error("Failed to compile tone mapping shader!");
        return false;
    }

    // Create fullscreen quad
    CreateFullscreenQuad();

    m_Initialized = true;
    Log::Info("PostProcessor initialized successfully!");
    return true;
}

void PostProcessor::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    m_ToneMappingShader.reset();
    m_FullscreenQuadVAO.reset();
    m_FullscreenQuadVBO.reset();
    m_FullscreenQuadIBO.reset();

    m_Initialized = false;
}

void PostProcessor::CreateFullscreenQuad() {
    // Fullscreen quad vertices (positions and UVs)
    float quadVertices[] = {
        // Positions   // TexCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    uint32_t quadIndices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Create vertex array
    m_FullscreenQuadVAO = std::make_unique<VertexArray>();
    m_FullscreenQuadVAO->Bind();

    // Create vertex buffer
    m_FullscreenQuadVBO = std::make_unique<VertexBuffer>(quadVertices, static_cast<uint32_t>(sizeof(quadVertices)));
    m_FullscreenQuadVBO->Bind();

    // Set vertex attributes
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0); // Position
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))); // TexCoord

    // Create index buffer
    m_FullscreenQuadIBO = std::make_unique<IndexBuffer>(quadIndices, 6);
    m_FullscreenQuadIBO->Bind();

    m_FullscreenQuadVAO->Unbind();
}

void PostProcessor::Render(uint32_t hdrTextureID, uint32_t width, uint32_t height, float exposure, EToneMapperType toneMapper) {
    if (!m_Initialized || !m_ToneMappingShader) {
        return;
    }

    // Bind default framebuffer (render to screen)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);

    // Use tone mapping shader
    m_ToneMappingShader->Bind();
    m_ToneMappingShader->SetUniform1f("u_Exposure", exposure);
    m_ToneMappingShader->SetUniform1i("u_ToneMapperType", static_cast<int>(toneMapper));

    // Bind HDR texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTextureID);
    m_ToneMappingShader->SetUniform1i("u_HDRTexture", 0);

    // Draw fullscreen quad
    m_FullscreenQuadVAO->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    m_FullscreenQuadVAO->Unbind();

    // Restore state
    glEnable(GL_DEPTH_TEST);
    m_ToneMappingShader->Unbind();
}

void PostProcessor::RenderToFramebuffer(uint32_t hdrTextureID, uint32_t targetFramebufferID, uint32_t width, uint32_t height, float exposure, EToneMapperType toneMapper) {
    if (!m_Initialized || !m_ToneMappingShader) {
        return;
    }

    // Save current OpenGL state
    GLint currentFBO;
    GLint currentViewport[4];
    GLboolean depthTestEnabled;
    GLboolean cullFaceEnabled;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
    glGetIntegerv(GL_VIEWPORT, currentViewport);
    depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
    cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

    // Bind target framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, targetFramebufferID);
    glViewport(0, 0, width, height);
    
    // Clear the framebuffer before rendering
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // Disable depth test and cull face for fullscreen quad
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Use tone mapping shader
    m_ToneMappingShader->Bind();
    m_ToneMappingShader->SetUniform1f("u_Exposure", exposure);
    m_ToneMappingShader->SetUniform1i("u_ToneMapperType", static_cast<int>(toneMapper));

    // Bind HDR texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, hdrTextureID);
    m_ToneMappingShader->SetUniform1i("u_HDRTexture", 0);

    // Draw fullscreen quad
    m_FullscreenQuadVAO->Bind();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    m_FullscreenQuadVAO->Unbind();

    // Unbind shader
    m_ToneMappingShader->Unbind();

    // Restore OpenGL state
    if (depthTestEnabled) glEnable(GL_DEPTH_TEST);
    if (cullFaceEnabled) glEnable(GL_CULL_FACE);
    glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
    glViewport(currentViewport[0], currentViewport[1], currentViewport[2], currentViewport[3]);
}

} // namespace LGE

