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

#include "LGE/rendering/Material.h"
#include "LGE/rendering/Shader.h"
#include "LGE/core/FileSystem.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

Material::Material() : m_Name("DefaultMaterial") {
}

Material::Material(const std::string& name) : m_Name(name) {
}

Material::~Material() {
}

void Material::SetFloat(const std::string& name, float value) {
    m_FloatProperties[name] = value;
}

void Material::SetVector3(const std::string& name, const Math::Vector3& value) {
    m_Vector3Properties[name] = value;
}

void Material::SetVector4(const std::string& name, const Math::Vector4& value) {
    m_Vector4Properties[name] = value;
}

void Material::SetColor(const std::string& name, const Math::Vector3& color) {
    m_Vector3Properties[name] = color;
}

float Material::GetFloat(const std::string& name) const {
    auto it = m_FloatProperties.find(name);
    if (it != m_FloatProperties.end()) {
        return it->second;
    }
    return 0.0f;
}

Math::Vector3 Material::GetVector3(const std::string& name) const {
    auto it = m_Vector3Properties.find(name);
    if (it != m_Vector3Properties.end()) {
        return it->second;
    }
    return Math::Vector3(0.0f, 0.0f, 0.0f);
}

Math::Vector4 Material::GetVector4(const std::string& name) const {
    auto it = m_Vector4Properties.find(name);
    if (it != m_Vector4Properties.end()) {
        return it->second;
    }
    return Math::Vector4(0.0f, 0.0f, 0.0f, 1.0f);
}

void Material::Bind() const {
    if (!m_Shader) {
        return;
    }

    m_Shader->Bind();

    // Upload all float properties
    for (const auto& prop : m_FloatProperties) {
        m_Shader->SetUniform1f(prop.first, prop.second);
    }

    // Upload all Vector3 properties
    for (const auto& prop : m_Vector3Properties) {
        m_Shader->SetUniform3f(prop.first, prop.second.x, prop.second.y, prop.second.z);
    }

    // Upload all Vector4 properties
    for (const auto& prop : m_Vector4Properties) {
        m_Shader->SetUniform4f(prop.first, prop.second.x, prop.second.y, prop.second.z, prop.second.w);
    }
}

void Material::Unbind() const {
    if (m_Shader) {
        m_Shader->Unbind();
    }
}

std::shared_ptr<Material> Material::CreateDefaultGridMaterial() {
    auto material = std::make_shared<Material>("DefaultGridMaterial");
    
    // Load grid shader
    std::string vertSource = FileSystem::ReadFile("assets/shaders/GridMaterial.vert");
    std::string fragSource = FileSystem::ReadFile("assets/shaders/GridMaterial.frag");
    
    if (vertSource.empty()) {
        Log::Error("Failed to load GridMaterial.vert shader file!");
        return nullptr;
    }
    if (fragSource.empty()) {
        Log::Error("Failed to load GridMaterial.frag shader file!");
        return nullptr;
    }
    
    auto shader = std::make_shared<Shader>(vertSource, fragSource);
    if (shader->GetRendererID() == 0) {
        Log::Error("Failed to compile GridMaterial shader!");
        return nullptr;
    }
    
    material->SetShader(shader);
    
    // Set default grid material properties (Unreal Engine style)
    material->SetColor("u_GridColor1", Math::Vector3(0.3f, 0.3f, 0.3f));  // Dark grid lines
    material->SetColor("u_GridColor2", Math::Vector3(0.9f, 0.9f, 0.9f));  // Light grid lines (very bright)
    material->SetColor("u_BaseColor", Math::Vector3(0.15f, 0.15f, 0.15f)); // Base color (darker background)
    material->SetFloat("u_GridSize", 1.0f);      // Grid cell size
    material->SetFloat("u_GridThickness", 0.02f); // Grid line thickness
    material->SetFloat("u_GridIntensity", 1.0f);  // Grid visibility (full intensity)
    
    return material;
}

std::shared_ptr<Material> Material::CreateDefaultLitMaterial() {
    auto material = std::make_shared<Material>("DefaultLitMaterial");
    
    // Load basic lit shader
    std::string vertSource = FileSystem::ReadFile("assets/shaders/Basic.vert");
    std::string fragSource = FileSystem::ReadFile("assets/shaders/Basic.frag");
    
    if (vertSource.empty()) {
        Log::Error("Failed to load Basic.vert shader file!");
        return nullptr;
    }
    if (fragSource.empty()) {
        Log::Error("Failed to load Basic.frag shader file!");
        return nullptr;
    }
    
    auto shader = std::make_shared<Shader>(vertSource, fragSource);
    if (shader->GetRendererID() == 0) {
        Log::Error("Failed to compile Basic shader!");
        return nullptr;
    }
    
    material->SetShader(shader);
    
    // Set default material color (medium gray - darker to appear gray with bright lighting)
    material->SetColor("u_MaterialColor", Math::Vector3(0.5f, 0.5f, 0.5f));
    material->SetFloat("u_UseVertexColor", 0.0f); // Use material color, not vertex color
    
    return material;
}

} // namespace LGE

