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
#include <vector>
#include <memory>
#include <functional>

namespace LGE {

// Forward declarations
class Project;

// Project browser entry (represents a project in the browser)
struct ProjectBrowserEntry {
    std::string name;
    std::string path;
    std::string projectFilePath;
    std::string description;
    std::string version;
    uint64_t lastModified;
    bool isValid;
    
    ProjectBrowserEntry()
        : lastModified(0), isValid(false) {}
};

class ProjectBrowser {
public:
    ProjectBrowser();
    ~ProjectBrowser();

    // Show the project browser (returns true if a project was selected/created)
    bool OnUIRender();
    
    // Get the selected/created project
    std::shared_ptr<Project> GetSelectedProject() const { return m_SelectedProject; }
    
    // Check if a project was selected
    bool HasSelectedProject() const { return m_SelectedProject != nullptr; }
    
    // Set callback for when a project is opened
    void SetOnProjectOpened(std::function<void(std::shared_ptr<Project>)> callback) {
        m_OnProjectOpened = callback;
    }
    
    // Refresh the project list
    void RefreshProjectList();
    
    // Set the projects directory (where to look for projects)
    void SetProjectsDirectory(const std::string& path) { m_ProjectsDirectory = path; }

private:
    // UI rendering methods
    void RenderLeftPanel();  // Recent projects and categories
    void RenderRightPanel(); // Template/project grid and details
    void RenderBottomBar();  // Project location, name, and buttons
    
    void RenderRecentProjects();
    void RenderCategories();
    void RenderProjectTemplates();
    void RenderProjectList();
    void RenderTemplateDetails();
    void RenderProjectDetails();
    
    // Project operations
    void OpenProject(const ProjectBrowserEntry& entry);
    void CreateNewProject(const std::string& projectName, const std::string& projectPath);
    void ScanForProjects();
    
    // Helper methods
    std::string FormatDate(uint64_t timestamp) const;
    std::string GetProjectThumbnailPath(const std::string& projectPath) const;
    void LoadThumbnails();
    
    std::vector<ProjectBrowserEntry> m_Projects;
    std::shared_ptr<Project> m_SelectedProject;
    std::string m_ProjectsDirectory;
    
    // Thumbnails
    std::shared_ptr<class Texture> m_FolderThumbnail;
    std::shared_ptr<class Texture> m_FirstPersonThumbnail;
    std::shared_ptr<class Texture> m_ThirdPersonThumbnail;
    std::shared_ptr<class Texture> m_TopDownThumbnail;
    std::shared_ptr<class Texture> m_VehicleThumbnail;
    std::shared_ptr<class Texture> m_VRThumbnail;
    std::shared_ptr<class Texture> m_BlankThumbnail;
    std::shared_ptr<class Texture> m_BlankPreview;
    // Category thumbnails
    std::shared_ptr<class Texture> m_GamesThumbnail;
    
    // UI state
    enum class ViewMode { Templates, Projects };
    ViewMode m_ViewMode;
    enum class Category { Games, Recent, Learning };
    Category m_SelectedCategory;
    
    int m_SelectedTemplateIndex;
    int m_SelectedProjectIndex;
    
    // Project creation
    char m_NewProjectNameBuffer[256];
    char m_NewProjectPathBuffer[512];
    bool m_IncludeStarterContent;
    
    // Callbacks
    std::function<void(std::shared_ptr<Project>)> m_OnProjectOpened;
};

} // namespace LGE

