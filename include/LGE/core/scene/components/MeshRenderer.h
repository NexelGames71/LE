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

#include "LGE/core/scene/Component.h"
#include "LGE/core/scene/ComponentFactory.h"
#include <memory>
#include <vector>

namespace LGE {

// Forward declarations
class Mesh;
class Material;

// MeshRenderer component - renders a mesh with materials
class MeshRenderer : public Component {
public:
    MeshRenderer();
    ~MeshRenderer() override = default;
    
    // Component interface
    const char* GetTypeName() const override { return "MeshRenderer"; }
    
    // Mesh
    void SetMesh(std::shared_ptr<Mesh> mesh);
    std::shared_ptr<Mesh> GetMesh() const { return m_Mesh; }
    
    // Materials (can have multiple for submeshes)
    void SetMaterial(std::shared_ptr<Material> material);
    void SetMaterial(size_t index, std::shared_ptr<Material> material);
    void AddMaterial(std::shared_ptr<Material> material);
    std::shared_ptr<Material> GetMaterial(size_t index = 0) const;
    const std::vector<std::shared_ptr<Material>>& GetMaterials() const { return m_Materials; }
    size_t GetMaterialCount() const { return m_Materials.size(); }
    
    // Shadow settings
    void SetCastShadows(bool castShadows) { m_CastShadows = castShadows; }
    bool GetCastShadows() const { return m_CastShadows; }
    
    void SetReceiveShadows(bool receiveShadows) { m_ReceiveShadows = receiveShadows; }
    bool GetReceiveShadows() const { return m_ReceiveShadows; }
    
    // Serialization
    std::string Serialize() const override;
    void Deserialize(const std::string& json) override;

private:
    std::shared_ptr<Mesh> m_Mesh;
    std::vector<std::shared_ptr<Material>> m_Materials;
    bool m_CastShadows;
    bool m_ReceiveShadows;
};

REGISTER_COMPONENT(MeshRenderer)

} // namespace LGE

