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

#include "LGE/core/GUID.h"
#include "LGE/core/assets/AssetReferenceFinder.h"
#include <vector>
#include <string>

namespace LGE {

class ReferenceViewerWindow {
private:
    AssetReferenceFinder* m_Finder;
    GUID m_CurrentAsset;
    std::vector<AssetReference> m_References;
    bool m_IsOpen;
    std::string m_WindowTitle;

public:
    ReferenceViewerWindow(AssetReferenceFinder* finder);
    
    // Show the window for a specific asset
    void Show(const GUID& asset);
    
    // Close the window
    void Close() { m_IsOpen = false; }
    
    // Check if window is open
    bool IsOpen() const { return m_IsOpen; }
    
    // Render the window (ImGui)
    void Render();
    
    // Get current asset being viewed
    GUID GetCurrentAsset() const { return m_CurrentAsset; }
    
    // Get reference count
    size_t GetReferenceCount() const { return m_References.size(); }
};

} // namespace LGE

