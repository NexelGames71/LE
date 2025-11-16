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

#include <memory>
#include <string>
#include "LGE/rendering/LightingSettings.h"

namespace LGE {

class Project;
class LightSystem;
class SceneManager;

class ProjectSettings {
public:
    ProjectSettings();
    ~ProjectSettings();

    void OnUIRender();
    void SetVisible(bool visible) { m_Visible = visible; }
    bool IsVisible() const { return m_Visible; }
    void Toggle() { m_Visible = !m_Visible; }
    
    void SetProject(std::shared_ptr<Project> project) { m_Project = project; }
    void SetLightSystem(class LightSystem* lightSystem) { m_LightSystem = lightSystem; }
    void SetSceneManager(class SceneManager* sceneManager) { m_SceneManager = sceneManager; }
    
    // Load lighting settings from project
    void LoadLightingSettings();
    // Save lighting settings to project
    void SaveLightingSettings();

private:
    void RenderLightingSettings();
    
    bool m_Visible;
    std::shared_ptr<Project> m_Project;
    std::string m_DefaultSceneBuffer;  // Buffer for input field
    
    // Lighting settings
    LightingSettings m_LightingSettings;
    bool m_LightingSettingsDirty = false;
    
    // References to systems (for applying settings)
    class LightSystem* m_LightSystem = nullptr;
    class SceneManager* m_SceneManager = nullptr;
};

} // namespace LGE

