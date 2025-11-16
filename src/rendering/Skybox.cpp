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

#include "LGE/rendering/Skybox.h"
#include "LGE/rendering/Camera.h"
#include "LGE/rendering/Texture.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <glad/glad.h>
#include <vector>
#include <cmath>

namespace LGE {

Skybox::Skybox()
    : m_TopColor(0.5f, 0.7f, 1.0f)  // Light blue
    , m_BottomColor(0.8f, 0.9f, 1.0f)  // Light gray-blue
    , m_Initialized(false)
    , m_HasTexture(false)
    , m_Scale(50.0f)  // Make skybox much larger (50x the original size)
{
}

Skybox::~Skybox() {
    // Resources will be cleaned up by unique_ptr
}

void Skybox::Init() {
    if (m_Initialized) {
        return;
    }

    // Load shaders
    std::string vertexSrc = FileSystem::ReadFile("assets/shaders/Skybox.vert");
    std::string fragmentSrc = FileSystem::ReadFile("assets/shaders/Skybox.frag");

    if (vertexSrc.empty() || fragmentSrc.empty()) {
        Log::Error("Failed to load skybox shader files!");
        return;
    }

    m_Shader = std::make_unique<Shader>(vertexSrc, fragmentSrc);

    // Create skybox cube (large cube, inside-out)
    // We only need positions for the skybox
    float skyboxVertices[] = {
        // Positions (large cube)
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    m_VertexBuffer = std::make_unique<VertexBuffer>(skyboxVertices, static_cast<uint32_t>(sizeof(skyboxVertices)));
    
    m_VertexArray = std::make_unique<VertexArray>();
    m_VertexArray->Bind();
    m_VertexBuffer->Bind();
    
    // Only position attribute (3 floats)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    m_VertexArray->Unbind();
    m_VertexBuffer->Unbind();

    m_Initialized = true;
    Log::Info("Skybox initialized successfully!");
}

bool Skybox::LoadHDR(const std::string& filepath) {
    if (!m_Initialized) {
        Log::Error("Skybox must be initialized before loading HDR texture!");
        return false;
    }

    m_Texture = std::make_unique<Texture>();
    // Use LoadHDRImage to auto-detect format (HDR or EXR)
    if (!m_Texture->LoadHDRImage(filepath)) {
        m_Texture.reset();
        return false;
    }

    m_HasTexture = true;
    Log::Info("HDR skybox texture loaded: " + filepath);
    return true;
}

void Skybox::Render(const Camera& camera, float cloudOffset) {
    if (!m_Initialized || !m_Shader) {
        return;
    }

    // Save current state
    GLint depthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &depthFunc);
    
    // Change depth function so skybox passes depth test at 1.0
    glDepthFunc(GL_LEQUAL);
    
    // Disable face culling for skybox
    glDisable(GL_CULL_FACE);

    m_Shader->Bind();
    
    // Set view-projection matrix (remove translation from view matrix for skybox)
    // We need to create a view matrix without translation
    Math::Matrix4 viewMatrix = camera.GetViewMatrix();
    // Remove translation from view matrix (set translation to 0)
    viewMatrix.GetData()[12] = 0.0f;
    viewMatrix.GetData()[13] = 0.0f;
    viewMatrix.GetData()[14] = 0.0f;
    
    Math::Matrix4 viewProjection = camera.GetProjectionMatrix() * viewMatrix;
    m_Shader->SetUniformMat4("u_ViewProjection", viewProjection.GetData());
    
    // Set skybox scale
    m_Shader->SetUniform1f("u_SkyboxScale", m_Scale);
    
    // Set cloud offset directly (simple horizontal movement)
    m_Shader->SetUniform1f("u_CloudOffset", cloudOffset);
    
    // Set texture or colors
    if (m_HasTexture && m_Texture) {
        m_Texture->Bind(0);
        m_Shader->SetUniform1i("u_SkyboxTexture", 0);
        m_Shader->SetUniform1i("u_UseTexture", 1);
    } else {
        m_Shader->SetUniform1i("u_UseTexture", 0);
        // Note: u_TopColor and u_BottomColor are not in the shader when using texture mode
        // They're only used in gradient mode, but shader doesn't have them anymore
    }
    
    m_VertexArray->Bind();
    glDrawArrays(GL_TRIANGLES, 0, 36);
    m_VertexArray->Unbind();
    
    m_Shader->Unbind();
    
    // Restore state
    glDepthFunc(depthFunc);
    glEnable(GL_CULL_FACE);
}

} // namespace LGE

