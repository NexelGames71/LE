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
#include <memory>
#include "LGE/core/GUID.h"

namespace LGE {

// Project metadata structure
struct ProjectInfo {
    std::string name;
    std::string path;           // Root directory of the project
    std::string version;        // Project version (e.g., "1.0.0")
    std::string engineVersion;  // LGE version used
    std::string description;    // Project description
    uint64_t createdDate;       // Timestamp when project was created
    uint64_t lastModified;     // Timestamp of last modification
};

class Project {
public:
    Project();
    ~Project();

    // Create a new blank project
    static std::shared_ptr<Project> CreateNew(const std::string& projectPath, const std::string& projectName);
    
    // Load an existing project from a .luma file
    static std::shared_ptr<Project> Load(const std::string& projectFilePath);
    
    // Save the current project
    bool Save();
    
    // Get project info
    const ProjectInfo& GetInfo() const { return m_Info; }
    ProjectInfo& GetInfo() { return m_Info; }
    
    // Get project paths
    std::string GetProjectPath() const { return m_Info.path; }
    
    // Assets paths
    std::string GetAssetsPath() const;
    std::string GetTexturesPath() const;
    std::string GetModelsPath() const;
    std::string GetMaterialsPath() const;
    std::string GetScriptsPath() const;
    std::string GetAudioPath() const;
    std::string GetScenesPath() const;
    
    // Config paths
    std::string GetConfigPath() const;
    std::string GetPackagesPath() const;
    std::string GetSourcePath() const;
    std::string GetPluginsPath() const;
    std::string GetEngineConfigPath() const;
    std::string GetProjectConfigPath() const;
    
    // Intermediate paths
    std::string GetIntermediatePath() const;
    std::string GetAssetCachePath() const;
    std::string GetThumbnailsPath() const;
    
    // Saved paths
    std::string GetSavedPath() const;
    std::string GetLogsPath() const;
    std::string GetSaveGamesPath() const;
    
    std::string GetProjectFilePath() const;  // Path to .luma file
    
    // Check if project is loaded
    bool IsLoaded() const { return m_IsLoaded; }
    
    // Default scene management (by GUID)
    GUID GetDefaultSceneGUID() const { return m_DefaultSceneGUID; }
    std::string GetDefaultScene() const;  // Returns path resolved from GUID
    void SetDefaultScene(const std::string& scenePath);  // Accepts path, resolves to GUID
    void SetDefaultSceneByGUID(const GUID& guid) { m_DefaultSceneGUID = guid; }
    
    // Initialize project directory structure
    bool InitializeDirectoryStructure();

private:
    ProjectInfo m_Info;
    bool m_IsLoaded;
    GUID m_DefaultSceneGUID;  // GUID of default scene
    std::string m_DefaultScenePath;  // Cached path (for backward compatibility during migration)
    
    // Helper methods
    bool CreateProjectFile();
    bool CreateDirectoryStructure();
    void CreateDefaultConfigFiles();
    bool CreateDefaultScene();
    std::string GetProjectFileName() const;
};

} // namespace LGE

