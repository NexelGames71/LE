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

#include "LGE/core/scene/ComponentFactory.h"
#include <mutex>

namespace LGE {

// Use a function-local static to ensure initialization order (Meyer's singleton)
// This prevents static initialization order fiasco
std::unordered_map<std::string, ComponentFactory::CreatorFunc>& ComponentFactory::GetCreators() {
    static std::unordered_map<std::string, CreatorFunc> s_Creators;
    return s_Creators;
}

// Keep the old static member for backward compatibility, but it's not used
std::unordered_map<std::string, ComponentFactory::CreatorFunc> ComponentFactory::s_Creators;

} // namespace LGE

