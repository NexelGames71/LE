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

#include "LGE/rendering/Shader.h"
#include "LGE/core/Log.h"
#include "LGE/core/filesystem/FileSystem.h"
#include <glad/glad.h>
#include <sstream>
#include <fstream>

namespace LGE {

Shader::Shader(const std::string& vertexSrc, const std::string& fragmentSrc)
    : m_IsCompute(false)
{
    m_RendererID = CreateProgram(vertexSrc, fragmentSrc);
}

Shader::Shader(const std::string& computeSrc)
    : m_IsCompute(true)
{
    m_RendererID = CreateComputeProgram(computeSrc);
}

Shader::~Shader() {
    glDeleteProgram(m_RendererID);
}

void Shader::Bind() const {
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int value) {
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1f(const std::string& name, float value) {
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2) {
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniformMat4(const std::string& name, const float* matrix) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, matrix);
}

void Shader::SetTexture(const std::string& name, uint32_t textureID, uint32_t slot) {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, textureID);
    SetUniform1i(name, static_cast<int>(slot));
}

void Shader::Dispatch(uint32_t numGroupsX, uint32_t numGroupsY, uint32_t numGroupsZ) const {
    if (!m_IsCompute) {
        Log::Error("Dispatch called on non-compute shader!");
        return;
    }
    glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ);
}

uint32_t Shader::CompileShader(uint32_t type, const std::string& source) {
    uint32_t shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        Log::Error("Shader compilation failed: " + std::string(infoLog));
        glDeleteShader(shader);
        return 0;
    }

    return shader;
}

uint32_t Shader::CreateProgram(const std::string& vertexSrc, const std::string& fragmentSrc) {
    uint32_t program = glCreateProgram();
    uint32_t vs = CompileShader(GL_VERTEX_SHADER, vertexSrc);
    uint32_t fs = CompileShader(GL_FRAGMENT_SHADER, fragmentSrc);

    if (vs == 0 || fs == 0) {
        return 0;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        Log::Error("Shader program linking failed: " + std::string(infoLog));
        glDeleteProgram(program);
        glDeleteShader(vs);
        glDeleteShader(fs);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

uint32_t Shader::CreateComputeProgram(const std::string& computeSrc) {
    uint32_t program = glCreateProgram();
    uint32_t cs = CompileShader(GL_COMPUTE_SHADER, computeSrc);

    if (cs == 0) {
        return 0;
    }

    glAttachShader(program, cs);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        Log::Error("Compute shader program linking failed: " + std::string(infoLog));
        glDeleteProgram(program);
        glDeleteShader(cs);
        return 0;
    }

    glDeleteShader(cs);

    return program;
}

int Shader::GetUniformLocation(const std::string& name) {
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end()) {
        return m_UniformLocationCache[name];
    }

    int location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location == -1) {
        Log::Warn("Uniform '" + name + "' not found in shader!");
    }
    m_UniformLocationCache[name] = location;
    return location;
}

// Static factory method to load shader from files
std::shared_ptr<Shader> Shader::CreateFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSrc = FileSystem::ReadFile(vertexPath);
    std::string fragmentSrc = FileSystem::ReadFile(fragmentPath);
    
    if (vertexSrc.empty()) {
        Log::Error("Failed to load vertex shader: " + vertexPath);
        return nullptr;
    }
    
    if (fragmentSrc.empty()) {
        Log::Error("Failed to load fragment shader: " + fragmentPath);
        return nullptr;
    }
    
    auto shader = std::make_shared<Shader>(vertexSrc, fragmentSrc);
    if (shader->GetRendererID() == 0) {
        Log::Error("Failed to compile shader from files: " + vertexPath + ", " + fragmentPath);
        return nullptr;
    }
    
    return shader;
}

// Static factory method to load compute shader from file
std::shared_ptr<Shader> Shader::CreateFromFiles(const std::string& computePath) {
    std::string computeSrc = FileSystem::ReadFile(computePath);
    
    if (computeSrc.empty()) {
        Log::Error("Failed to load compute shader: " + computePath);
        return nullptr;
    }
    
    auto shader = std::make_shared<Shader>(computeSrc);
    if (shader->GetRendererID() == 0) {
        Log::Error("Failed to compile compute shader from file: " + computePath);
        return nullptr;
    }
    
    return shader;
}

} // namespace LGE

