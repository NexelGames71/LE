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

#include "LGE/core/scene/components/MeshRenderer.h"
#include "LGE/rendering/Mesh.h"
#include "LGE/rendering/Material.h"
#include <sstream>

namespace LGE {

MeshRenderer::MeshRenderer()
    : m_CastShadows(true)
    , m_ReceiveShadows(true)
{
}

void MeshRenderer::SetMesh(std::shared_ptr<Mesh> mesh) {
    m_Mesh = mesh;
}

void MeshRenderer::SetMaterial(std::shared_ptr<Material> material) {
    if (m_Materials.empty()) {
        m_Materials.push_back(material);
    } else {
        m_Materials[0] = material;
    }
}

void MeshRenderer::SetMaterial(size_t index, std::shared_ptr<Material> material) {
    if (index >= m_Materials.size()) {
        m_Materials.resize(index + 1);
    }
    m_Materials[index] = material;
}

void MeshRenderer::AddMaterial(std::shared_ptr<Material> material) {
    m_Materials.push_back(material);
}

std::shared_ptr<Material> MeshRenderer::GetMaterial(size_t index) const {
    if (index < m_Materials.size()) {
        return m_Materials[index];
    }
    return nullptr;
}

std::string MeshRenderer::Serialize() const {
    std::ostringstream json;
    json << "{\n";
    json << "  \"type\": \"MeshRenderer\",\n";
    json << "  \"castShadows\": " << (m_CastShadows ? "true" : "false") << ",\n";
    json << "  \"receiveShadows\": " << (m_ReceiveShadows ? "true" : "false") << ",\n";
    json << "  \"meshGUID\": \"" << (m_Mesh ? "ASSIGNED" : "NONE") << "\",\n";
    json << "  \"materialCount\": " << m_Materials.size() << "\n";
    json << "}";
    return json.str();
}

void MeshRenderer::Deserialize(const std::string& json) {
    // Manual JSON parsing (simplified)
    // In a full implementation, use proper JSON parsing
}

} // namespace LGE

