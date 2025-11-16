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

#include "LGE/rendering/Mesh.h"
#include "LGE/rendering/VertexArray.h"
#include "LGE/rendering/IndexBuffer.h"
#include "LGE/rendering/VertexBuffer.h"
#include <glad/glad.h>
#include <vector>
#include <cmath>

namespace LGE {

// Basic Mesh implementation
class BasicMesh : public Mesh {
public:
    BasicMesh(std::shared_ptr<VertexArray> vertexArray, std::shared_ptr<IndexBuffer> indexBuffer, uint32_t vertexCount, uint32_t indexCount)
        : m_VertexArray(vertexArray)
        , m_IndexBuffer(indexBuffer)
        , m_VertexCount(vertexCount)
        , m_IndexCount(indexCount)
    {
    }
    
    std::shared_ptr<VertexArray> GetVertexArray() const override { return m_VertexArray; }
    std::shared_ptr<IndexBuffer> GetIndexBuffer() const override { return m_IndexBuffer; }
    uint32_t GetVertexCount() const override { return m_VertexCount; }
    uint32_t GetIndexCount() const override { return m_IndexCount; }

private:
    std::shared_ptr<VertexArray> m_VertexArray;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
    uint32_t m_VertexCount;
    uint32_t m_IndexCount;
};

Mesh::Mesh() : m_Name("Mesh") {
}

// Primitive mesh factory implementations
std::shared_ptr<Mesh> PrimitiveMesh::CreateCube() {
    // Cube vertices: position(3) + color(3) + normal(3) = 9 floats per vertex
    // 6 faces * 2 triangles * 3 vertices = 36 vertices
    float vertices[] = {
        // Front face (Red) - normal: 0, 0, 1
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom right
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Top right
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom left
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Top right
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Top left
        
        // Back face (Green) - normal: 0, 0, -1
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Bottom left
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Top right
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Bottom right
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Bottom left
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Top left
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Top right
        
        // Left face (Blue) - normal: -1, 0, 0
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Bottom back
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Bottom front
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Top front
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Bottom back
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Top front
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Top back
        
        // Right face (Yellow) - normal: 1, 0, 0
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Bottom back
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top back
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Bottom front
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Bottom front
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top back
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top front
        
        // Top face (Cyan) - normal: 0, 1, 0
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Back left
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Front right
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Back right
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Back left
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Front left
         0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Front right
        
        // Bottom face (Magenta) - normal: 0, -1, 0
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Back left
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Back right
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Front right
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Back left
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Front right
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f   // Front left
    };
    
    // Create vertex buffer
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, static_cast<uint32_t>(sizeof(vertices)));
    
    // Create vertex array and set up attributes manually
    auto vertexArray = std::make_shared<VertexArray>();
    vertexArray->Bind();
    vertexBuffer->Bind();
    
    // Set up vertex attributes: position (0), color (1), normal (2)
    // Stride: 9 floats (3 pos + 3 color + 3 normal)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    
    vertexArray->Unbind();
    vertexBuffer->Unbind();
    
    // Create mesh (no index buffer, using triangle list)
    auto mesh = std::make_shared<BasicMesh>(vertexArray, nullptr, 36, 0);
    mesh->SetName("Cube");
    return mesh;
}

std::shared_ptr<Mesh> PrimitiveMesh::CreateSphere() {
    // Create a sphere with 32 segments
    int segments = 32;
    float radius = 0.5f;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Generate vertices: position(3) + color(3) + normal(3) = 9 floats per vertex
    for (int y = 0; y <= segments; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float xSegment = static_cast<float>(x) / static_cast<float>(segments);
            float ySegment = static_cast<float>(y) / static_cast<float>(segments);
            float xPos = std::cos(xSegment * 2.0f * 3.14159f) * std::sin(ySegment * 3.14159f);
            float yPos = std::cos(ySegment * 3.14159f);
            float zPos = std::sin(xSegment * 2.0f * 3.14159f) * std::sin(ySegment * 3.14159f);
            
            // Position
            vertices.push_back(xPos * radius);
            vertices.push_back(yPos * radius);
            vertices.push_back(zPos * radius);
            
            // Color (white)
            vertices.push_back(1.0f);
            vertices.push_back(1.0f);
            vertices.push_back(1.0f);
            
            // Normal (same as position for unit sphere)
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
        }
    }
    
    // Generate indices
    for (int y = 0; y < segments; ++y) {
        for (int x = 0; x < segments; ++x) {
            int first = (y * (segments + 1)) + x;
            int second = first + segments + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    // Create vertex buffer
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));
    
    // Create index buffer
    auto indexBuffer = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
    
    // Create vertex array and set up attributes
    auto vertexArray = std::make_shared<VertexArray>();
    vertexArray->Bind();
    vertexBuffer->Bind();
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    
    vertexArray->Unbind();
    vertexBuffer->Unbind();
    
    // Create mesh
    auto mesh = std::make_shared<BasicMesh>(vertexArray, indexBuffer, static_cast<uint32_t>((segments + 1) * (segments + 1)), static_cast<uint32_t>(indices.size()));
    mesh->SetName("Sphere");
    return mesh;
}

std::shared_ptr<Mesh> PrimitiveMesh::CreatePlane() {
    // Plane vertices: position(3) + color(3) + normal(3) = 9 floats per vertex
    // 2 triangles = 6 vertices
    // Fixed winding order to face up correctly
    float vertices[] = {
        // Position          Color           Normal
        -0.5f,  0.0f,  0.5f,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f,  // Top left
         0.5f,  0.0f,  0.5f,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f,  // Top right
         0.5f,  0.0f, -0.5f,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f,  // Bottom right
        -0.5f,  0.0f,  0.5f,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f,  // Top left
         0.5f,  0.0f, -0.5f,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f,  // Bottom right
        -0.5f,  0.0f, -0.5f,  0.8f, 0.8f, 0.8f,  0.0f, 1.0f, 0.0f   // Bottom left
    };
    
    // Create vertex buffer
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices, static_cast<uint32_t>(sizeof(vertices)));
    
    // Create vertex array and set up attributes
    auto vertexArray = std::make_shared<VertexArray>();
    vertexArray->Bind();
    vertexBuffer->Bind();
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    
    vertexArray->Unbind();
    vertexBuffer->Unbind();
    
    // Create mesh
    auto mesh = std::make_shared<BasicMesh>(vertexArray, nullptr, 6, 0);
    mesh->SetName("Plane");
    return mesh;
}

std::shared_ptr<Mesh> PrimitiveMesh::CreateCylinder() {
    // Create a cylinder with 32 segments
    int segments = 32;
    float radius = 0.5f;
    float height = 1.0f;
    float halfHeight = height * 0.5f;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Generate vertices for the cylinder body and caps
    // Position(3) + Color(3) + Normal(3) = 9 floats per vertex
    
    // Bottom cap center
    vertices.push_back(0.0f); vertices.push_back(-halfHeight); vertices.push_back(0.0f);  // Position
    vertices.push_back(0.8f); vertices.push_back(0.8f); vertices.push_back(0.8f);  // Color
    vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);  // Normal
    
    // Top cap center
    vertices.push_back(0.0f); vertices.push_back(halfHeight); vertices.push_back(0.0f);  // Position
    vertices.push_back(0.8f); vertices.push_back(0.8f); vertices.push_back(0.8f);  // Color
    vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);  // Normal
    
    // Generate bottom cap vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * 3.14159f;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        
        vertices.push_back(x); vertices.push_back(-halfHeight); vertices.push_back(z);  // Position
        vertices.push_back(0.8f); vertices.push_back(0.8f); vertices.push_back(0.8f);  // Color
        vertices.push_back(0.0f); vertices.push_back(-1.0f); vertices.push_back(0.0f);  // Normal
    }
    
    // Generate top cap vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * 3.14159f;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        
        vertices.push_back(x); vertices.push_back(halfHeight); vertices.push_back(z);  // Position
        vertices.push_back(0.8f); vertices.push_back(0.8f); vertices.push_back(0.8f);  // Color
        vertices.push_back(0.0f); vertices.push_back(1.0f); vertices.push_back(0.0f);  // Normal
    }
    
    // Generate body vertices (bottom and top rings)
    for (int i = 0; i <= segments; ++i) {
        float angle = (static_cast<float>(i) / static_cast<float>(segments)) * 2.0f * 3.14159f;
        float x = std::cos(angle) * radius;
        float z = std::sin(angle) * radius;
        float nx = std::cos(angle);
        float nz = std::sin(angle);
        
        // Bottom ring
        vertices.push_back(x); vertices.push_back(-halfHeight); vertices.push_back(z);  // Position
        vertices.push_back(0.8f); vertices.push_back(0.8f); vertices.push_back(0.8f);  // Color
        vertices.push_back(nx); vertices.push_back(0.0f); vertices.push_back(nz);  // Normal
        
        // Top ring
        vertices.push_back(x); vertices.push_back(halfHeight); vertices.push_back(z);  // Position
        vertices.push_back(0.8f); vertices.push_back(0.8f); vertices.push_back(0.8f);  // Color
        vertices.push_back(nx); vertices.push_back(0.0f); vertices.push_back(nz);  // Normal
    }
    
    // Bottom cap indices
    int bottomCenterIdx = 0;
    int bottomRingStart = 2;
    for (int i = 0; i < segments; ++i) {
        indices.push_back(bottomCenterIdx);
        indices.push_back(bottomRingStart + i);
        indices.push_back(bottomRingStart + i + 1);
    }
    
    // Top cap indices
    int topCenterIdx = 1;
    int topRingStart = 2 + segments + 1;
    for (int i = 0; i < segments; ++i) {
        indices.push_back(topCenterIdx);
        indices.push_back(topRingStart + i + 1);
        indices.push_back(topRingStart + i);
    }
    
    // Body indices
    int bodyStart = topRingStart + segments + 1;
    for (int i = 0; i < segments; ++i) {
        int bottom = bodyStart + i * 2;
        int top = bottom + 1;
        int nextBottom = bodyStart + ((i + 1) % (segments + 1)) * 2;
        int nextTop = nextBottom + 1;
        
        // First triangle
        indices.push_back(bottom);
        indices.push_back(top);
        indices.push_back(nextBottom);
        
        // Second triangle
        indices.push_back(nextBottom);
        indices.push_back(top);
        indices.push_back(nextTop);
    }
    
    // Create vertex buffer
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));
    
    // Create index buffer
    auto indexBuffer = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
    
    // Create vertex array and set up attributes
    auto vertexArray = std::make_shared<VertexArray>();
    vertexArray->Bind();
    vertexBuffer->Bind();
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    
    vertexArray->Unbind();
    vertexBuffer->Unbind();
    
    // Create mesh
    auto mesh = std::make_shared<BasicMesh>(vertexArray, indexBuffer, static_cast<uint32_t>(vertices.size() / 9), static_cast<uint32_t>(indices.size()));
    mesh->SetName("Cylinder");
    return mesh;
}

std::shared_ptr<Mesh> PrimitiveMesh::CreateCapsule() {
    // Create a capsule (cylinder with rounded hemispherical ends)
    // Industry-standard approach used by Unity, Unreal, and Bullet Physics
    int segments = 32;
    int hemisphereRings = 8; // Rings for each hemisphere
    float radius = 0.5f;
    float height = 2.0f; // Total height including hemispheres (must be > radius * 2 to have a cylinder body)
    float cylinderHeight = std::max(0.0f, height - radius * 2.0f); // Height of the middle cylinder part
    float halfCylinderHeight = cylinderHeight * 0.5f;
    
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<int> ringStartIndices; // Track where each ring starts in the vertex array
    
    // Generate vertices: Position(3) + Color(3) + Normal(3) = 9 floats per vertex
    // Each ring has (segments + 1) vertices
    
    // Bottom hemisphere (from bottom to start of cylinder)
    float bottomSphereCenterY = -halfCylinderHeight - radius;
    for (int ring = 0; ring <= hemisphereRings; ++ring) {
        ringStartIndices.push_back(static_cast<int>(vertices.size() / 9)); // Track ring start
        
        float theta = (static_cast<float>(ring) / static_cast<float>(hemisphereRings)) * 3.14159f * 0.5f; // 0 to PI/2
        float y = -halfCylinderHeight - radius * std::cos(theta); // Correct formula
        float ringRadius = radius * std::sin(theta);
        
        for (int seg = 0; seg <= segments; ++seg) {
            float phi = (static_cast<float>(seg) / static_cast<float>(segments)) * 2.0f * 3.14159f;
            float x = std::cos(phi) * ringRadius;
            float z = std::sin(phi) * ringRadius;
            
            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // Color
            vertices.push_back(0.8f);
            vertices.push_back(0.8f);
            vertices.push_back(0.8f);
            
            // Normal (pointing outward from sphere center)
            float yLocal = y - bottomSphereCenterY;
            float len = std::sqrt(x * x + yLocal * yLocal + z * z);
            if (len > 0.0001f) {
                vertices.push_back(x / len);
                vertices.push_back(yLocal / len);
                vertices.push_back(z / len);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(-1.0f);
                vertices.push_back(0.0f);
            }
        }
    }
    
    // Cylinder body (middle section) - always generate at least one ring for smooth connection
    int cylinderRings = 1;
    if (cylinderHeight > 0.01f) {
        cylinderRings = 2; // At least 2 rings for the cylinder
    }
    for (int ring = 1; ring <= cylinderRings; ++ring) {
        ringStartIndices.push_back(static_cast<int>(vertices.size() / 9)); // Track ring start
        
        float t = static_cast<float>(ring) / static_cast<float>(cylinderRings + 1);
        float y = -halfCylinderHeight + t * cylinderHeight;
        
        for (int seg = 0; seg <= segments; ++seg) {
            float phi = (static_cast<float>(seg) / static_cast<float>(segments)) * 2.0f * 3.14159f;
            float x = std::cos(phi) * radius;
            float z = std::sin(phi) * radius;
            
            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // Color
            vertices.push_back(0.8f);
            vertices.push_back(0.8f);
            vertices.push_back(0.8f);
            
            // Normal (pointing outward horizontally)
            vertices.push_back(std::cos(phi));
            vertices.push_back(0.0f);
            vertices.push_back(std::sin(phi));
        }
    }
    
    // Top hemisphere (from end of cylinder to top)
    // Generate from bottom of hemisphere (at cylinder top) to top pole
    float topSphereCenterY = halfCylinderHeight + radius;
    for (int ring = 0; ring <= hemisphereRings; ++ring) {
        ringStartIndices.push_back(static_cast<int>(vertices.size() / 9)); // Track ring start
        
        // For top hemisphere, we go from theta = PI/2 (at cylinder connection) to theta = 0 (at top pole)
        // So we need to reverse the theta calculation
        float theta = (static_cast<float>(hemisphereRings - ring) / static_cast<float>(hemisphereRings)) * 3.14159f * 0.5f; // PI/2 to 0
        float y = halfCylinderHeight + radius * std::cos(theta); // Start at cylinder top, go up to pole
        float ringRadius = radius * std::sin(theta);
        
        for (int seg = 0; seg <= segments; ++seg) {
            float phi = (static_cast<float>(seg) / static_cast<float>(segments)) * 2.0f * 3.14159f;
            float x = std::cos(phi) * ringRadius;
            float z = std::sin(phi) * ringRadius;
            
            // Position
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
            
            // Color
            vertices.push_back(0.8f);
            vertices.push_back(0.8f);
            vertices.push_back(0.8f);
            
            // Normal (pointing outward from sphere center)
            // For top hemisphere, normals point upward and outward
            float yLocal = y - topSphereCenterY;
            float len = std::sqrt(x * x + yLocal * yLocal + z * z);
            if (len > 0.0001f) {
                vertices.push_back(x / len);
                vertices.push_back(yLocal / len);
                vertices.push_back(z / len);
            } else {
                vertices.push_back(0.0f);
                vertices.push_back(1.0f);
                vertices.push_back(0.0f);
            }
        }
    }
    
    // Generate indices using explicit ring start offsets
    for (int r = 0; r < static_cast<int>(ringStartIndices.size()) - 1; ++r) {
        int startA = ringStartIndices[r];
        int startB = ringStartIndices[r + 1];
        
        for (int s = 0; s < segments; ++s) {
            int a0 = startA + s;
            int a1 = startA + ((s + 1) % (segments + 1)); // Wrap around for last segment
            int b0 = startB + s;
            int b1 = startB + ((s + 1) % (segments + 1)); // Wrap around for last segment
            
            // First triangle
            indices.push_back(a0);
            indices.push_back(b0);
            indices.push_back(a1);
            
            // Second triangle
            indices.push_back(a1);
            indices.push_back(b0);
            indices.push_back(b1);
        }
    }
    
    // Create vertex buffer
    auto vertexBuffer = std::make_shared<VertexBuffer>(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(float)));
    
    // Create index buffer
    auto indexBuffer = std::make_shared<IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
    
    // Create vertex array and set up attributes
    auto vertexArray = std::make_shared<VertexArray>();
    vertexArray->Bind();
    vertexBuffer->Bind();
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    
    vertexArray->Unbind();
    vertexBuffer->Unbind();
    
    // Create mesh
    auto mesh = std::make_shared<BasicMesh>(vertexArray, indexBuffer, static_cast<uint32_t>(vertices.size() / 9), static_cast<uint32_t>(indices.size()));
    mesh->SetName("Capsule");
    return mesh;
}

} // namespace LGE

