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

#include "LGE/core/project/ProjectDescriptor.h"
#include <string>
#include <filesystem>
#include <memory>

namespace LGE {

// Luma Engine version constant
#define LUMA_ENGINE_VERSION "0.1.0"

class ProjectManager {
public:
    ProjectManager();
    ~ProjectManager();

    // Create a new project
    bool CreateNewProject(const std::string& name, const std::filesystem::path& location);
    
    // Open an existing project
    bool OpenProject(const std::filesystem::path& projectFile);
    
    // Close current project
    bool CloseProject();
    
    // Get current project descriptor
    ProjectDescriptor* GetCurrentProject();
    const ProjectDescriptor* GetCurrentProject() const;
    
    // Check if a project is currently open
    bool IsProjectOpen() const;
    
    // Get current project root path
    std::filesystem::path GetProjectRoot() const;

private:
    std::unique_ptr<ProjectDescriptor> m_CurrentProject;
    std::filesystem::path m_ProjectRoot;
    
    // Helper methods
    bool CreateDirectoryStructure(const std::filesystem::path& projectPath);
};

} // namespace LGE

