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

#include "LGE/math/Vector.h"
#include <memory>
#include <string>

namespace LGE {
class Texture;

namespace Luminite {

// PBR Material with metal/rough workflow
struct PBRMaterial {
    // Base color (albedo for dielectrics, specular color for metals)
    Math::Vector3 baseColor = Math::Vector3(0.8f, 0.8f, 0.8f);
    std::shared_ptr<Texture> baseColorTexture = nullptr;
    
    // Metalness (0 = dielectric, 1 = metal)
    float metalness = 0.0f;
    std::shared_ptr<Texture> metalnessTexture = nullptr;
    
    // Roughness (0 = smooth, 1 = rough)
    float roughness = 0.5f;
    std::shared_ptr<Texture> roughnessTexture = nullptr;
    
    // Normal map
    std::shared_ptr<Texture> normalTexture = nullptr;
    float normalScale = 1.0f;
    
    // Ambient Occlusion
    float ao = 1.0f;
    std::shared_ptr<Texture> aoTexture = nullptr;
    
    // Emissive
    Math::Vector3 emissive = Math::Vector3(0.0f, 0.0f, 0.0f);
    std::shared_ptr<Texture> emissiveTexture = nullptr;
    float emissiveIntensity = 1.0f;
    
    // Texture paths (for serialization)
    std::string baseColorPath;
    std::string metalnessPath;
    std::string roughnessPath;
    std::string normalPath;
    std::string aoPath;
    std::string emissivePath;
};

} // namespace Luminite
} // namespace LGE













