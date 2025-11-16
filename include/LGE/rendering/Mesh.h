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
#include <string>
#include <vector>
#include "LGE/math/Vector.h"

namespace LGE {

// Forward declarations
class VertexArray;
class IndexBuffer;

// Simple Mesh interface for rendering
class Mesh {
public:
    Mesh();
    virtual ~Mesh() = default;
    
    // Get vertex array for rendering
    virtual std::shared_ptr<VertexArray> GetVertexArray() const = 0;
    
    // Get index buffer (if indexed)
    virtual std::shared_ptr<IndexBuffer> GetIndexBuffer() const = 0;
    
    // Get vertex count
    virtual uint32_t GetVertexCount() const = 0;
    
    // Get index count (0 if not indexed)
    virtual uint32_t GetIndexCount() const = 0;
    
    // Get mesh name
    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

protected:
    std::string m_Name;
};

// Primitive mesh factory
class PrimitiveMesh {
public:
    static std::shared_ptr<Mesh> CreateCube();
    static std::shared_ptr<Mesh> CreateSphere();
    static std::shared_ptr<Mesh> CreatePlane();
    static std::shared_ptr<Mesh> CreateCylinder();
    static std::shared_ptr<Mesh> CreateCapsule();
};

} // namespace LGE

