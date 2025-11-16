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

#include "LGE/rendering/Texture.h"
#include "LGE/rendering/TextureManager.h"
#include <memory>
#include <string>

namespace LGE {

class TextureImporter {
public:
    TextureImporter();
    ~TextureImporter();

    void OnUIRender();
    
    void SetTextureManager(TextureManager* manager) { m_TextureManager = manager; }
    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }
    void Toggle() { m_Visible = !m_Visible; }

private:
    void HandleDragDrop();
    void RenderTexturePreview();
    void RenderTextureMetadata();
    void RenderTextureSettings();
    void SaveTextureAsset(const std::string& filepath, const TextureSpec& spec);
    bool LoadTextureAsset(const std::string& assetPath, TextureSpec& spec);
    
    TextureManager* m_TextureManager;
    bool m_Visible;
    
    // Current texture being imported
    std::string m_CurrentFilePath;
    std::shared_ptr<Texture> m_PreviewTexture;
    TextureSpec m_CurrentSpec;
    bool m_HasTexture;
};

} // namespace LGE

