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

#include <cstdint>
#include <vector>
#include "LGE/math/Matrix.h"

namespace LGE {

// Structure to hold data for a directional light's shadow map
struct DirectionalLightShadow {
    Math::Matrix4 LightViewProj;    // Light space view-projection matrix
    uint32_t ShadowMapTextureID;    // OpenGL texture ID of the depth map
    uint32_t ShadowMapFBO;          // OpenGL FBO ID for rendering to the depth map
    uint32_t ShadowMapSize;         // Resolution of the shadow map (e.g., 2048)
    bool IsValid;                   // True if shadow map is successfully created and ready to use
};

// Cascaded Shadow Map (CSM) - multiple shadow maps for different distances
struct CascadedShadowMap {
    static constexpr int MAX_CASCADES = 4;
    
    struct Cascade {
        Math::Matrix4 LightViewProj;    // Light space view-projection for this cascade
        float SplitDistance;            // Distance where this cascade ends
        uint32_t ShadowMapTextureID;     // Texture ID (if using separate textures)
        bool IsValid;
    };
    
    Cascade Cascades[MAX_CASCADES];
    int CascadeCount = 4;                // Number of active cascades
    uint32_t ShadowMapSize = 2048;       // Resolution per cascade
    bool IsValid = false;
};

// Shadow map atlas - combines multiple shadow maps into one texture
struct ShadowMapAtlas {
    uint32_t AtlasTextureID;            // Combined texture containing all shadow maps
    uint32_t AtlasFBO;                   // Framebuffer for rendering to atlas
    uint32_t AtlasSize = 4096;           // Total atlas size (e.g., 4096x4096)
    uint32_t TileSize = 1024;            // Size of each shadow map tile
    
    struct Tile {
        uint32_t X, Y;                   // Position in atlas (in tiles)
        uint32_t LightIndex;             // Index of light using this tile
        bool IsUsed;
    };
    
    std::vector<Tile> Tiles;
    int MaxTiles = 16;                   // Maximum number of tiles (e.g., 4x4 grid)
    bool IsValid = false;
};

} // namespace LGE
