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

#include "LGE/ui/MainMenuBar.h"
#include "LGE/ui/Preferences.h"
#include "LGE/ui/ProjectSettings.h"
#include "imgui.h"

namespace LGE {

MainMenuBar::MainMenuBar()
    : m_Preferences(nullptr)
    , m_ProjectSettings(nullptr)
{
}

MainMenuBar::~MainMenuBar() {
}

void MainMenuBar::OnUIRender() {
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene")) {
                // New scene
            }
            if (ImGui::MenuItem("Open Scene")) {
                // Open scene
            }
            if (ImGui::MenuItem("Save Scene")) {
                // Save scene
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                // Exit
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo")) {
                // Undo
            }
            if (ImGui::MenuItem("Redo")) {
                // Redo
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Preferences")) {
                if (m_Preferences) {
                    m_Preferences->Toggle();
                }
            }
            if (ImGui::MenuItem("Project Settings")) {
                if (m_ProjectSettings) {
                    m_ProjectSettings->Toggle();
                }
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Window")) {
            if (ImGui::MenuItem("Outliner")) {
                // Toggle outliner
            }
            if (ImGui::MenuItem("Details")) {
                // Toggle details
            }
            if (ImGui::MenuItem("Content Browser")) {
                // Toggle content browser
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Build")) {
                // Build
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Build")) {
            if (ImGui::MenuItem("Build All Levels")) {
                // Build all levels
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Select")) {
            if (ImGui::MenuItem("All")) {
                // Select all
            }
            if (ImGui::MenuItem("None")) {
                // Deselect all
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Actor")) {
            if (ImGui::MenuItem("Spawn Actor")) {
                // Spawn actor
            }
            ImGui::EndMenu();
        }
        
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) {
                // About
            }
            ImGui::EndMenu();
        }
        
        ImGui::EndMainMenuBar();
    }
}

} // namespace LGE


