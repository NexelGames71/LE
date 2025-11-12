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

#include "LGE/rendering/GridRenderer.h"
#include "LGE/rendering/VertexArray.h"
#include "LGE/rendering/VertexBuffer.h"
#include "LGE/rendering/Shader.h"
#include "LGE/math/Vector.h"
#include "LGE/math/Matrix.h"
#include <glad/glad.h>
#include <vector>
#include <cstring>
#include <cstddef> // for offsetof

namespace LGE {

struct GridVertex {
    Math::Vector3 position;
    Math::Vector3 color;
};

GridRenderer::GridRenderer(int halfSize, float spacing)
    : m_VertexCount(0)
{
    GenerateGridVertices(halfSize, spacing);
}

GridRenderer::~GridRenderer() {
    // Smart pointers will handle cleanup
}

void GridRenderer::GenerateGridVertices(int halfSize, float spacing) {
    std::vector<GridVertex> vertices;

    // Generate grid lines
    // Grid is on XZ plane (Y=0)
    // X-axis line: extends along X direction (parallel to X), at Z=0 - should be RED
    // Z-axis line: extends along Z direction (parallel to Z), at X=0 - should be BLUE
    for (int i = -halfSize; i <= halfSize; i++) {
        float coord = i * spacing;

        // Lines parallel to X (Z-axis lines) - these are lines at Z=coord extending in X direction
        // When coord=0, this is the X-axis line (Z=0 line) - should be RED
        Math::Vector3 color = (i == 0) ? Math::Vector3(1.0f, 0.0f, 0.0f) : Math::Vector3(0.3f, 0.3f, 0.3f);
        vertices.push_back({ Math::Vector3(-halfSize * spacing, 0.0f, coord), color });
        vertices.push_back({ Math::Vector3(halfSize * spacing, 0.0f, coord), color });

        // Lines parallel to Z (X-axis lines) - these are lines at X=coord extending in Z direction
        // When coord=0, this is the Z-axis line (X=0 line) - should be BLUE
        color = (i == 0) ? Math::Vector3(0.0f, 0.0f, 1.0f) : Math::Vector3(0.3f, 0.3f, 0.3f);
        vertices.push_back({ Math::Vector3(coord, 0.0f, -halfSize * spacing), color });
        vertices.push_back({ Math::Vector3(coord, 0.0f, halfSize * spacing), color });
    }

    m_VertexCount = static_cast<int>(vertices.size());

    // Create vertex buffer
    m_VertexBuffer = std::make_unique<VertexBuffer>(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(GridVertex)));

    // Create vertex array
    m_VertexArray = std::make_unique<VertexArray>();
    m_VertexArray->Bind();
    m_VertexBuffer->Bind();

    // Set up vertex attributes: position (0), color (1)
    glEnableVertexAttribArray(0); // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GridVertex), (void*)0);
    glEnableVertexAttribArray(1); // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GridVertex), (void*)offsetof(GridVertex, color));

    m_VertexArray->Unbind();
    m_VertexBuffer->Unbind();
}

void GridRenderer::Draw(const Math::Matrix4& viewProj) {
    if (!m_Shader) {
        return;
    }

    // Save OpenGL state
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    GLint currentDepthFunc;
    glGetIntegerv(GL_DEPTH_FUNC, &currentDepthFunc);

    // Enable depth testing so grid is occluded by objects
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    m_Shader->Bind();
    m_Shader->SetUniformMat4("u_ViewProj", viewProj.GetData());

    m_VertexArray->Bind();
    glDrawArrays(GL_LINES, 0, m_VertexCount);
    m_VertexArray->Unbind();

    m_Shader->Unbind();

    // Restore OpenGL state
    if (!depthTestEnabled) {
        glDisable(GL_DEPTH_TEST);
    }
    glDepthFunc(currentDepthFunc);
}

} // namespace LGE

