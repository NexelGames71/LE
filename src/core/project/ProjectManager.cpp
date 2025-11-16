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

#include "LGE/core/project/ProjectManager.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <filesystem>

namespace LGE {

ProjectManager::ProjectManager()
    : m_ProjectRoot()
{
}

ProjectManager::~ProjectManager() {
    CloseProject();
}

bool ProjectManager::CreateNewProject(const std::string& name, const std::filesystem::path& location) {
    namespace fs = std::filesystem;
    
    // Create root directory
    fs::path projectPath = location / name;
    
    if (!fs::exists(projectPath)) {
        if (!fs::create_directories(projectPath)) {
            Log::Error("Failed to create project directory: " + projectPath.string());
            return false;
        }
    }
    
    // Create standard subdirectories
    if (!CreateDirectoryStructure(projectPath)) {
        Log::Error("Failed to create project directory structure");
        return false;
    }
    
    // Create project descriptor
    ProjectDescriptor descriptor;
    descriptor.projectName = name;
    descriptor.engineVersion = LUMA_ENGINE_VERSION;
    descriptor.defaultScene = "/Assets/Scenes/DefaultScene.lscene";
    
    // Save descriptor
    fs::path descriptorPath = projectPath / (name + ".luma");
    if (!descriptor.Save(descriptorPath)) {
        Log::Error("Failed to save project descriptor");
        return false;
    }
    
    // Open the newly created project
    return OpenProject(descriptorPath);
}

bool ProjectManager::OpenProject(const std::filesystem::path& projectFile) {
    namespace fs = std::filesystem;
    
    if (!fs::exists(projectFile)) {
        Log::Error("Project file not found: " + projectFile.string());
        return false;
    }
    
    // Close current project if open
    CloseProject();
    
    // Load project descriptor
    m_CurrentProject = std::make_unique<ProjectDescriptor>();
    if (!m_CurrentProject->Load(projectFile)) {
        Log::Error("Failed to load project descriptor");
        m_CurrentProject.reset();
        return false;
    }
    
    // Set project root (parent directory of .luma file)
    m_ProjectRoot = projectFile.parent_path();
    
    Log::Info("Opened project: " + m_CurrentProject->projectName + " at " + m_ProjectRoot.string());
    return true;
}

bool ProjectManager::CloseProject() {
    if (m_CurrentProject) {
        Log::Info("Closing project: " + m_CurrentProject->projectName);
        m_CurrentProject.reset();
        m_ProjectRoot.clear();
        return true;
    }
    return false;
}

ProjectDescriptor* ProjectManager::GetCurrentProject() {
    return m_CurrentProject.get();
}

const ProjectDescriptor* ProjectManager::GetCurrentProject() const {
    return m_CurrentProject.get();
}

bool ProjectManager::IsProjectOpen() const {
    return m_CurrentProject != nullptr;
}

std::filesystem::path ProjectManager::GetProjectRoot() const {
    return m_ProjectRoot;
}

bool ProjectManager::CreateDirectoryStructure(const std::filesystem::path& projectPath) {
    namespace fs = std::filesystem;
    
    try {
        // Create main directories
        fs::create_directories(projectPath / "Assets");
        fs::create_directories(projectPath / "Assets" / "Textures");
        fs::create_directories(projectPath / "Assets" / "Models");
        fs::create_directories(projectPath / "Assets" / "Materials");
        fs::create_directories(projectPath / "Assets" / "Scripts");
        fs::create_directories(projectPath / "Assets" / "Audio");
        fs::create_directories(projectPath / "Assets" / "Scenes");
        
        // Create Config/Packages structure
        fs::create_directories(projectPath / "Config" / "Packages" / "Source");
        fs::create_directories(projectPath / "Config" / "Packages" / "Plugins");
        fs::create_directories(projectPath / "Config" / "Packages" / "Intermediate" / "AssetCache");
        fs::create_directories(projectPath / "Config" / "Packages" / "Intermediate" / "Thumbnails");
        fs::create_directories(projectPath / "Config" / "Packages" / "Saved" / "Logs");
        fs::create_directories(projectPath / "Config" / "Packages" / "Saved" / "SaveGames");
        
        Log::Info("Created project directory structure");
        return true;
    } catch (const std::exception& e) {
        Log::Error("Exception while creating directory structure: " + std::string(e.what()));
        return false;
    }
}

} // namespace LGE

