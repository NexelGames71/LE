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

#include "LGE/rendering/Shader.h"
#include "LGE/rendering/Texture.h"
#include "LGE/math/Vector.h"
#include <memory>
#include <string>
#include <unordered_map>

namespace LGE {

class Material {
public:
    Material();
    Material(const std::string& name);
    ~Material();

    void SetShader(std::shared_ptr<Shader> shader) { m_Shader = shader; }
    std::shared_ptr<Shader> GetShader() const { return m_Shader; }

    // Parameter setters
    void SetFloat(const std::string& name, float value);
    void SetVector3(const std::string& name, const Math::Vector3& value);
    void SetVector4(const std::string& name, const Math::Vector4& value);
    void SetColor(const std::string& name, const Math::Vector3& color);
    void SetTexture(const std::string& name, std::shared_ptr<Texture> texture, uint32_t slot = 0);

    // Parameter getters
    float GetFloat(const std::string& name) const;
    Math::Vector3 GetVector3(const std::string& name) const;
    Math::Vector4 GetVector4(const std::string& name) const;
    std::shared_ptr<Texture> GetTexture(const std::string& name) const;

    // Remove parameter
    void RemoveFloat(const std::string& name);
    void RemoveVector3(const std::string& name);
    void RemoveVector4(const std::string& name);
    void RemoveTexture(const std::string& name);

    void Bind() const;
    void Unbind() const;

    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    // Static factory methods for common materials
    static std::shared_ptr<Material> CreateDefaultGridMaterial();
    static std::shared_ptr<Material> CreateDefaultLitMaterial();
    static std::shared_ptr<Material> CreateUnlitMaterial();
    static std::shared_ptr<Material> CreatePBRMaterial();
    static std::shared_ptr<Material> CreateSkyboxMaterial();

private:
    std::string m_Name;
    std::shared_ptr<Shader> m_Shader;
    std::unordered_map<std::string, float> m_FloatProperties;
    std::unordered_map<std::string, Math::Vector3> m_Vector3Properties;
    std::unordered_map<std::string, Math::Vector4> m_Vector4Properties;
    std::unordered_map<std::string, std::shared_ptr<Texture>> m_TextureProperties;
    std::unordered_map<std::string, uint32_t> m_TextureSlots;  // Track texture slot assignments
    uint32_t m_NextTextureSlot;  // Next available texture slot
};

} // namespace LGE


