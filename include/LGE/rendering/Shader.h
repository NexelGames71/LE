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

#include <string>
#include <unordered_map>
#include <cstdint>

namespace LGE {

class Shader {
public:
    Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
    Shader(const std::string& computeSrc);  // Compute shader constructor
    ~Shader();

    void Bind() const;
    void Unbind() const;

    // Compute shader dispatch
    void Dispatch(uint32_t numGroupsX, uint32_t numGroupsY = 1, uint32_t numGroupsZ = 1) const;

    void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformMat4(const std::string& name, const float* matrix);

    uint32_t GetRendererID() const { return m_RendererID; }
    bool IsComputeShader() const { return m_IsCompute; }

private:
    uint32_t CompileShader(uint32_t type, const std::string& source);
    uint32_t CreateProgram(const std::string& vertexSrc, const std::string& fragmentSrc);
    uint32_t CreateComputeProgram(const std::string& computeSrc);
    int GetUniformLocation(const std::string& name);

    uint32_t m_RendererID;
    bool m_IsCompute;
    std::unordered_map<std::string, int> m_UniformLocationCache;
};

} // namespace LGE

