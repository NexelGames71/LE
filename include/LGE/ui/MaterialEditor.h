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

#include "LGE/rendering/Material.h"
#include "LGE/rendering/TextureManager.h"
#include <memory>
#include <string>
#include <vector>

namespace LGE {

class MaterialEditor {
public:
    MaterialEditor();
    ~MaterialEditor();

    void OnUIRender();
    
    void SetTextureManager(TextureManager* manager) { m_TextureManager = manager; }
    void SetMaterial(std::shared_ptr<Material> material) { m_CurrentMaterial = material; }
    std::shared_ptr<Material> GetMaterial() const { return m_CurrentMaterial; }
    
    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }
    void Toggle() { m_Visible = !m_Visible; }

private:
    void RenderShaderSelection();
    void RenderParameters();
    void RenderAddParameterMenu();
    void SaveMaterial();
    
    TextureManager* m_TextureManager;
    std::shared_ptr<Material> m_CurrentMaterial;
    bool m_Visible;
    
    // Available shaders list
    std::vector<std::pair<std::string, std::pair<std::string, std::string>>> m_AvailableShaders;
};

} // namespace LGE

