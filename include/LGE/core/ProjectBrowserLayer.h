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

#include "LGE/core/Layer.h"
#include <memory>
#include <functional>

namespace LGE {

class ProjectBrowser;
class Project;

/**
 * @brief Layer that displays the project browser UI
 * 
 * This layer is shown when no project is loaded. It allows users to:
 * - Select an existing project
 * - Create a new project from templates
 * 
 * When a project is selected/created, this layer can notify the application
 * to load editor layers and transition to the editor.
 */
class ProjectBrowserLayer : public Layer {
public:
    ProjectBrowserLayer();
    ~ProjectBrowserLayer() override;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;

    /**
     * @brief Check if a project has been selected/created
     * @return True if a project is ready
     */
    bool HasProject() const;

    /**
     * @brief Get the selected/created project
     * @return Shared pointer to the project, or nullptr if none
     */
    std::shared_ptr<Project> GetProject() const;

    /**
     * @brief Set callback for when a project is opened
     * @param callback Function to call when project is opened
     */
    void SetOnProjectOpened(std::function<void(std::shared_ptr<Project>)> callback);

    /**
     * @brief Set the projects directory (where to look for projects)
     * @param path Path to projects directory
     */
    void SetProjectsDirectory(const std::string& path);

    /**
     * @brief Refresh the project list
     */
    void RefreshProjectList();

    /**
     * @brief Check if this layer should be active (no project loaded)
     * @return True if should be active
     */
    bool ShouldBeActive() const { return !HasProject(); }

private:
    std::unique_ptr<ProjectBrowser> m_ProjectBrowser;
    std::function<void(std::shared_ptr<Project>)> m_OnProjectOpened;
};

} // namespace LGE

