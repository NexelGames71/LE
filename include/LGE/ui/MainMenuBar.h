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

namespace LGE {

class Preferences;
class ProjectSettings;

class MainMenuBar {
public:
    MainMenuBar();
    ~MainMenuBar();

    void OnUIRender();
    void SetPreferences(Preferences* preferences) { m_Preferences = preferences; }
    void SetProjectSettings(ProjectSettings* projectSettings) { m_ProjectSettings = projectSettings; }

private:
    Preferences* m_Preferences;
    ProjectSettings* m_ProjectSettings;
};

} // namespace LGE


