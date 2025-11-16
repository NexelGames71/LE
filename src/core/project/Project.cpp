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

#include "LGE/core/project/Project.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include "LGE/core/scene/World.h"
#include "LGE/core/scene/SceneManager.h"
#include <nlohmann/json.hpp>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace LGE {

Project::Project()
    : m_IsLoaded(false)
    , m_DefaultSceneGUID(GUID::Invalid())
    , m_DefaultScenePath("SampleScene.lscene")
{
    m_Info.version = "1.0.0";
    m_Info.engineVersion = "0.1.0";
    m_Info.createdDate = 0;
    m_Info.lastModified = 0;
}

Project::~Project() {
}

std::shared_ptr<Project> Project::CreateNew(const std::string& projectPath, const std::string& projectName) {
    auto project = std::make_shared<Project>();
    
    // Validate project name is not empty
    if (projectName.empty()) {
        Log::Error("Project name cannot be empty");
        return nullptr;
    }
    
    // Normalize project path
    std::string normalizedPath = FileSystem::NormalizePath(projectPath);
    
    // Check if a project file with the same name already exists
    std::string projectFilePath = FileSystem::JoinPath(normalizedPath, projectName + ".luma");
    if (FileSystem::Exists(projectFilePath)) {
        Log::Error("A project with the name \"" + projectName + "\" already exists at " + normalizedPath);
        return nullptr;
    }
    
    if (!FileSystem::Exists(normalizedPath)) {
        if (!FileSystem::CreateDirectory(normalizedPath)) {
            Log::Error("Failed to create project directory: " + normalizedPath);
            return nullptr;
        }
    }
    
    // Set project info
    project->m_Info.name = projectName;
    project->m_Info.path = normalizedPath;
    project->m_Info.description = "A new LGE project";
    
    // Set timestamps
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    project->m_Info.createdDate = static_cast<uint64_t>(timeT);
    project->m_Info.lastModified = static_cast<uint64_t>(timeT);
    
    // Create directory structure
    if (!project->CreateDirectoryStructure()) {
        Log::Error("Failed to create project directory structure");
        return nullptr;
    }
    
    // Create project file
    if (!project->CreateProjectFile()) {
        Log::Error("Failed to create project file");
        return nullptr;
    }
    
    // Create default SampleScene
    if (!project->CreateDefaultScene()) {
        Log::Warn("Failed to create default scene, but continuing...");
    }
    
    project->m_IsLoaded = true;
    Log::Info("Created new project: " + projectName + " at " + normalizedPath);
    
    return project;
}

std::shared_ptr<Project> Project::Load(const std::string& projectFilePath) {
    if (!FileSystem::Exists(projectFilePath)) {
        Log::Error("Project file not found: " + projectFilePath);
        return nullptr;
    }
    
    auto project = std::make_shared<Project>();
    
    // Read project file (JSON format)
    std::string content = FileSystem::ReadFile(projectFilePath);
    if (content.empty()) {
        Log::Error("Failed to read project file: " + projectFilePath);
        return nullptr;
    }
    
    // Simple JSON parsing (basic implementation)
    // In a real engine, you'd use a proper JSON library like nlohmann/json
    // For now, we'll do basic parsing
    std::string projectPath = FileSystem::GetDirectory(projectFilePath);
    project->m_Info.path = projectPath;
    
    // Extract project name from file path
    std::string fileName = FileSystem::GetFileNameWithoutExtension(projectFilePath);
    project->m_Info.name = fileName;
    
    // Parse JSON content (simplified - just extract key fields)
    // Look for "name", "version", "description" fields
    size_t namePos = content.find("\"name\"");
    if (namePos != std::string::npos) {
        size_t colonPos = content.find(':', namePos);
        size_t quoteStart = content.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = content.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                project->m_Info.name = content.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    size_t versionPos = content.find("\"version\"");
    if (versionPos != std::string::npos) {
        size_t colonPos = content.find(':', versionPos);
        size_t quoteStart = content.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = content.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                project->m_Info.version = content.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    size_t descPos = content.find("\"description\"");
    if (descPos != std::string::npos) {
        size_t colonPos = content.find(':', descPos);
        size_t quoteStart = content.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = content.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                project->m_Info.description = content.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Try to load using nlohmann/json for better parsing
    try {
        auto j = nlohmann::json::parse(content);
        
        // Load GUID (preferred)
        if (j.contains("defaultSceneGUID") && j["defaultSceneGUID"].is_string()) {
            GUID guid = GUID::FromString(j["defaultSceneGUID"]);
            if (guid.IsValid()) {
                project->m_DefaultSceneGUID = guid;
            }
        }
        
        // Load path (for backward compatibility and fallback)
        if (j.contains("defaultScene") && j["defaultScene"].is_string()) {
            project->m_DefaultScenePath = j["defaultScene"];
        }
        
        // If no GUID was loaded but path exists, try to resolve GUID from file
        if (!project->m_DefaultSceneGUID.IsValid() && !project->m_DefaultScenePath.empty()) {
            project->SetDefaultScene(project->m_DefaultScenePath);
        }
    } catch (...) {
        // Fallback to manual parsing for backward compatibility
        size_t defaultScenePos = content.find("\"defaultScene\"");
        if (defaultScenePos != std::string::npos) {
            size_t colonPos = content.find(':', defaultScenePos);
            size_t quoteStart = content.find('"', colonPos);
            if (quoteStart != std::string::npos) {
                size_t quoteEnd = content.find('"', quoteStart + 1);
                if (quoteEnd != std::string::npos) {
                    project->m_DefaultScenePath = content.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    // Try to resolve GUID
                    project->SetDefaultScene(project->m_DefaultScenePath);
                }
            }
        } else {
            // Default to SampleScene if not found (for backward compatibility)
            project->m_DefaultScenePath = "SampleScene.lscene";
        }
    }
    
    // Update last modified timestamp
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    project->m_Info.lastModified = static_cast<uint64_t>(timeT);
    
    project->m_IsLoaded = true;
    Log::Info("Loaded project: " + project->m_Info.name + " from " + projectFilePath);
    
    return project;
}

bool Project::Save() {
    if (!m_IsLoaded) {
        Log::Error("Cannot save: No project loaded");
        return false;
    }
    
    return CreateProjectFile();
}

// Assets paths
std::string Project::GetAssetsPath() const {
    return FileSystem::JoinPath(m_Info.path, "Assets");
}

std::string Project::GetTexturesPath() const {
    return FileSystem::JoinPath(GetAssetsPath(), "Textures");
}

std::string Project::GetModelsPath() const {
    return FileSystem::JoinPath(GetAssetsPath(), "Models");
}

std::string Project::GetMaterialsPath() const {
    return FileSystem::JoinPath(GetAssetsPath(), "Materials");
}

std::string Project::GetScriptsPath() const {
    return FileSystem::JoinPath(GetAssetsPath(), "Scripts");
}

std::string Project::GetAudioPath() const {
    return FileSystem::JoinPath(GetAssetsPath(), "Audio");
}

std::string Project::GetScenesPath() const {
    return FileSystem::JoinPath(GetAssetsPath(), "Scenes");
}

// Config paths
std::string Project::GetConfigPath() const {
    return FileSystem::JoinPath(m_Info.path, "Config");
}

std::string Project::GetPackagesPath() const {
    return FileSystem::JoinPath(GetConfigPath(), "Packages");
}

std::string Project::GetSourcePath() const {
    return FileSystem::JoinPath(GetPackagesPath(), "Source");
}

std::string Project::GetPluginsPath() const {
    return FileSystem::JoinPath(GetPackagesPath(), "Plugins");
}

std::string Project::GetEngineConfigPath() const {
    return FileSystem::JoinPath(GetPackagesPath(), "engine.config");
}

std::string Project::GetProjectConfigPath() const {
    return FileSystem::JoinPath(GetPackagesPath(), "project.config");
}

// Intermediate paths
std::string Project::GetIntermediatePath() const {
    return FileSystem::JoinPath(GetPackagesPath(), "Intermediate");
}

std::string Project::GetAssetCachePath() const {
    return FileSystem::JoinPath(GetIntermediatePath(), "AssetCache");
}

std::string Project::GetThumbnailsPath() const {
    return FileSystem::JoinPath(GetIntermediatePath(), "Thumbnails");
}

// Saved paths
std::string Project::GetSavedPath() const {
    return FileSystem::JoinPath(GetPackagesPath(), "Saved");
}

std::string Project::GetLogsPath() const {
    return FileSystem::JoinPath(GetSavedPath(), "Logs");
}

std::string Project::GetSaveGamesPath() const {
    return FileSystem::JoinPath(GetSavedPath(), "SaveGames");
}

std::string Project::GetProjectFilePath() const {
    return FileSystem::JoinPath(m_Info.path, GetProjectFileName());
}

std::string Project::GetProjectFileName() const {
    return m_Info.name + ".luma";
}

bool Project::CreateProjectFile() {
    std::string projectFilePath = GetProjectFilePath();
    
    // Update last modified timestamp
    auto now = std::chrono::system_clock::now();
    auto timeT = std::chrono::system_clock::to_time_t(now);
    m_Info.lastModified = static_cast<uint64_t>(timeT);
    
    // Create JSON content
    std::ostringstream json;
    json << "{\n";
    json << "  \"name\": \"" << m_Info.name << "\",\n";
    json << "  \"version\": \"" << m_Info.version << "\",\n";
    json << "  \"engineVersion\": \"" << m_Info.engineVersion << "\",\n";
    json << "  \"description\": \"" << m_Info.description << "\",\n";
    // Save default scene GUID (preferred) and path (for backward compatibility)
    json << "  \"defaultSceneGUID\": \"" << m_DefaultSceneGUID.ToString() << "\",\n";
    json << "  \"defaultScene\": \"" << m_DefaultScenePath << "\",\n";  // Legacy support
    json << "  \"createdDate\": " << m_Info.createdDate << ",\n";
    json << "  \"lastModified\": " << m_Info.lastModified << "\n";
    json << "}\n";
    
    if (!FileSystem::WriteFile(projectFilePath, json.str())) {
        Log::Error("Failed to write project file: " + projectFilePath);
        return false;
    }
    
    Log::Info("Saved project file: " + projectFilePath);
    return true;
}

bool Project::CreateDirectoryStructure() {
    // Create all directories according to the new structure
    std::vector<std::string> directories = {
        // Assets directories
        GetAssetsPath(),
        GetTexturesPath(),
        GetModelsPath(),
        GetMaterialsPath(),
        GetScriptsPath(),
        GetAudioPath(),
        GetScenesPath(),
        
        // Config directories
        GetConfigPath(),
        GetPackagesPath(),
        GetSourcePath(),
        GetPluginsPath(),
        
        // Intermediate directories
        GetIntermediatePath(),
        GetAssetCachePath(),
        GetThumbnailsPath(),
        
        // Saved directories
        GetSavedPath(),
        GetLogsPath(),
        GetSaveGamesPath()
    };
    
    for (const auto& dir : directories) {
        if (!FileSystem::Exists(dir)) {
            if (!FileSystem::CreateDirectory(dir)) {
                Log::Error("Failed to create directory: " + dir);
                return false;
            }
            Log::Info("Created directory: " + dir);
        }
    }
    
    // Create default config files
    CreateDefaultConfigFiles();
    
    return true;
}

void Project::CreateDefaultConfigFiles() {
    // Create engine.config
    std::string engineConfig = GetEngineConfigPath();
    if (!FileSystem::Exists(engineConfig)) {
        std::ostringstream content;
        content << "{\n";
        content << "  \"engineVersion\": \"" << m_Info.engineVersion << "\",\n";
        content << "  \"projectName\": \"" << m_Info.name << "\"\n";
        content << "}\n";
        if (FileSystem::WriteFile(engineConfig, content.str())) {
            Log::Info("Created engine.config");
        }
    }
    
    // Create project.config
    std::string projectConfig = GetProjectConfigPath();
    if (!FileSystem::Exists(projectConfig)) {
        std::ostringstream content;
        content << "{\n";
        content << "  \"name\": \"" << m_Info.name << "\",\n";
        content << "  \"version\": \"" << m_Info.version << "\",\n";
        content << "  \"description\": \"" << m_Info.description << "\"\n";
        content << "}\n";
        if (FileSystem::WriteFile(projectConfig, content.str())) {
            Log::Info("Created project.config");
        }
    }
}

bool Project::CreateDefaultScene() {
    // Create a default SampleScene
    auto world = std::make_shared<World>("SampleScene");
    
    // Create a default GameObject in the scene
    auto defaultObject = world->CreateGameObject("Main Camera");
    
    // Save the scene to the scenes folder
    std::string scenesPath = GetScenesPath();
    std::string scenePath = FileSystem::JoinPath(scenesPath, "SampleScene.lscene");
    
    if (world->SaveToFile(scenePath)) {
        // Store the GUID of the created scene
        m_DefaultSceneGUID = world->GetGUID();
        m_DefaultScenePath = "SampleScene.lscene";
        Log::Info("Created default scene: " + scenePath + " (GUID: " + m_DefaultSceneGUID.ToString() + ")");
        return true;
    } else {
        Log::Error("Failed to create default scene: " + scenePath);
        return false;
    }
}

bool Project::InitializeDirectoryStructure() {
    return CreateDirectoryStructure();
}

std::string Project::GetDefaultScene() const {
    // If GUID is valid, try to resolve it to a path
    if (m_DefaultSceneGUID.IsValid()) {
        std::string scenesPath = GetScenesPath();
        
        // Scan scenes directory for matching GUID
        if (FileSystem::Exists(scenesPath)) {
            auto files = FileSystem::ListDirectory(scenesPath);
            for (const auto& entry : files) {
                if (!entry.isDirectory && entry.extension == ".lscene") {
                    std::string scenePath = FileSystem::JoinPath(scenesPath, entry.name);
                    try {
                        std::string json = FileSystem::ReadFile(scenePath);
                        if (!json.empty()) {
                            auto j = nlohmann::json::parse(json);
                            if (j.contains("guid") && j["guid"].is_string()) {
                                GUID sceneGUID = GUID::FromString(j["guid"]);
                                if (sceneGUID == m_DefaultSceneGUID) {
                                    return entry.name;  // Return filename
                                }
                            }
                        }
                    } catch (...) {
                        // Skip invalid JSON files
                        continue;
                    }
                }
            }
        }
        
        // If GUID not found, fall back to cached path
        if (!m_DefaultScenePath.empty()) {
            return m_DefaultScenePath;
        }
    }
    
    // Fallback to cached path or default
    return m_DefaultScenePath.empty() ? "SampleScene.lscene" : m_DefaultScenePath;
}

void Project::SetDefaultScene(const std::string& scenePath) {
    m_DefaultScenePath = scenePath;
    
    // Try to resolve GUID from scene file
    std::string scenesPath = GetScenesPath();
    std::string fullPath = FileSystem::JoinPath(scenesPath, scenePath);
    
    if (FileSystem::Exists(fullPath)) {
        try {
            std::string json = FileSystem::ReadFile(fullPath);
            if (!json.empty()) {
                auto j = nlohmann::json::parse(json);
                if (j.contains("guid") && j["guid"].is_string()) {
                    m_DefaultSceneGUID = GUID::FromString(j["guid"]);
                    Log::Info("Set default scene to: " + scenePath + " (GUID: " + m_DefaultSceneGUID.ToString() + ")");
                    return;
                }
            }
        } catch (...) {
            Log::Warn("Failed to read GUID from scene file: " + fullPath);
        }
    }
    
    // If we can't resolve GUID, mark as invalid
    m_DefaultSceneGUID = GUID::Invalid();
    Log::Warn("Could not resolve GUID for scene: " + scenePath + ", using path-based reference");
}

} // namespace LGE

