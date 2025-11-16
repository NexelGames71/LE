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

#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include <string>
#include "LGE/core/scene/Component.h"

namespace LGE {

class ComponentFactory {
private:
    using CreatorFunc = std::function<std::unique_ptr<Component>()>;
    static std::unordered_map<std::string, CreatorFunc> s_Creators;
    static std::unordered_map<std::string, CreatorFunc>& GetCreators();

public:
    // Register a component type
    template<typename T>
    static void Register(const std::string& typeName) {
        static_assert(std::is_base_of<Component, T>::value, "T must inherit from Component");
        GetCreators()[typeName] = []() -> std::unique_ptr<Component> {
            return std::make_unique<T>();
        };
    }
    
    // Create a component by type name
    static std::unique_ptr<Component> Create(const std::string& typeName) {
        auto& creators = GetCreators();
        auto it = creators.find(typeName);
        if (it != creators.end()) {
            return it->second();
        }
        return nullptr;
    }
    
    // Get all registered component type names
    static std::vector<std::string> GetRegisteredTypes() {
        auto& creators = GetCreators();
        std::vector<std::string> types;
        types.reserve(creators.size());
        for (const auto& [name, creator] : creators) {
            types.push_back(name);
        }
        return types;
    }
    
    // Check if a type is registered
    static bool IsRegistered(const std::string& typeName) {
        return GetCreators().find(typeName) != GetCreators().end();
    }
    
    // Clear all registrations (useful for testing)
    static void Clear() {
        GetCreators().clear();
    }
};

} // namespace LGE

// Macro for easy component registration
// Usage: REGISTER_COMPONENT(MyComponent) - use just the class name, call inside LGE namespace
#define REGISTER_COMPONENT(Type) \
    namespace { \
        struct AutoRegister_##Type { \
            AutoRegister_##Type() { \
                LGE::ComponentFactory::Register<LGE::Type>(#Type); \
            } \
        }; \
        static AutoRegister_##Type g_AutoRegister_##Type##_instance; \
    }

