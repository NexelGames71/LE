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
namespace Math {
    struct Vector3;
    struct Matrix4;
}

class VertexArray;
class VertexBuffer;
class Shader;

class GridRenderer {
public:
    GridRenderer(int halfSize = 100, float spacing = 1.0f);
    ~GridRenderer();

    void Draw(const Math::Matrix4& viewProj);
    void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }
    std::shared_ptr<Shader> GetShader() const { return m_Shader; }

private:
    void GenerateGridVertices(int halfSize, float spacing);

    std::unique_ptr<VertexArray> m_VertexArray;
    std::unique_ptr<VertexBuffer> m_VertexBuffer;
    std::shared_ptr<Shader> m_Shader;
    int m_VertexCount;
};

} // namespace LGE

