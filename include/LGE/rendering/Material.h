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

    void SetFloat(const std::string& name, float value);
    void SetVector3(const std::string& name, const Math::Vector3& value);
    void SetVector4(const std::string& name, const Math::Vector4& value);
    void SetColor(const std::string& name, const Math::Vector3& color);

    float GetFloat(const std::string& name) const;
    Math::Vector3 GetVector3(const std::string& name) const;
    Math::Vector4 GetVector4(const std::string& name) const;

    void Bind() const;
    void Unbind() const;

    const std::string& GetName() const { return m_Name; }
    void SetName(const std::string& name) { m_Name = name; }

    // Static factory methods for common materials
    static std::shared_ptr<Material> CreateDefaultGridMaterial();
    static std::shared_ptr<Material> CreateDefaultLitMaterial();

private:
    std::string m_Name;
    std::shared_ptr<Shader> m_Shader;
    std::unordered_map<std::string, float> m_FloatProperties;
    std::unordered_map<std::string, Math::Vector3> m_Vector3Properties;
    std::unordered_map<std::string, Math::Vector4> m_Vector4Properties;
};

} // namespace LGE


