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

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "LGE/core/Application.h"
#include "LGE/core/Log.h"
#include "LGE/core/FileSystem.h"
#include "LGE/core/Window.h"
#include "LGE/rendering/Shader.h"
#include "LGE/rendering/VertexBuffer.h"
#include "LGE/rendering/VertexArray.h"
#include "LGE/rendering/IndexBuffer.h"
#include "LGE/rendering/Camera.h"
#include "LGE/rendering/CameraController.h"
#include "LGE/rendering/Skybox.h"
#include "LGE/rendering/DirectionalLight.h"
#include "LGE/rendering/Material.h"
#include "LGE/rendering/GridRenderer.h"
#include "LGE/ui/UI.h"
#include "LGE/ui/SceneViewport.h"
#include "LGE/ui/Hierarchy.h"
#include "LGE/ui/Inspector.h"
#include "LGE/ui/ContentBrowser.h"
#include "LGE/ui/Toolbar.h"
#include "LGE/ui/MainMenuBar.h"
#include "LGE/ui/Details.h"
#include "LGE/ui/Profiler.h"
#include "LGE/ui/Preferences.h"
#include "LGE/ui/ProjectSettings.h"
#include "LGE/core/Input.h"
#include "LGE/core/GameObject.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <memory>
#include <cstdint>
#include <cmath>
#include <vector>
#include <map>

class ExampleApp : public LGE::Application {
public:
    ExampleApp() : LGE::Application("LGE Game Engine Example") {}

    bool Initialize() override {
        if (!LGE::Application::Initialize()) {
            return false;
        }

        // Setup materials
        m_LitMaterial = LGE::Material::CreateDefaultLitMaterial();
        
        if (!m_LitMaterial) {
            LGE::Log::Error("Failed to create default materials!");
            return false;
        }
        
        // Setup grid renderer
        m_GridRenderer = std::make_unique<LGE::GridRenderer>(100, 1.0f); // halfSize=100, spacing=1.0
        
        // Load grid shader
        std::string gridVertSource = LGE::FileSystem::ReadFile("assets/shaders/grid.vert");
        std::string gridFragSource = LGE::FileSystem::ReadFile("assets/shaders/grid.frag");
        
        if (gridVertSource.empty() || gridFragSource.empty()) {
            LGE::Log::Error("Failed to load grid shader files!");
            return false;
        }
        
        auto gridShader = std::make_shared<LGE::Shader>(gridVertSource, gridFragSource);
        if (gridShader->GetRendererID() == 0) {
            LGE::Log::Error("Failed to compile grid shader!");
            return false;
        }
        
        m_GridRenderer->SetShader(gridShader);
        
        // Keep shader reference for backward compatibility
        m_Shader = m_LitMaterial->GetShader();

        // Cube vertices with colors and normals
        // Format: x, y, z, r, g, b, nx, ny, nz
        // Each face has 2 triangles (6 vertices)
        float vertices[] = {
            // Front face (Red) - normal: 0, 0, 1
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom left
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom right
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Top right
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Bottom left
             0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Top right
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  // Top left
            
            // Back face (Green) - normal: 0, 0, -1
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Bottom left
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Top right
             0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Bottom right
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Bottom left
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Top left
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f, -1.0f,  // Top right
            
            // Left face (Blue) - normal: -1, 0, 0
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Bottom back
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Bottom front
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Top front
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Bottom back
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Top front
            -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  -1.0f, 0.0f, 0.0f,  // Top back
            
            // Right face (Yellow) - normal: 1, 0, 0
             0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Bottom back
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top back
             0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Bottom front
             0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Bottom front
             0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top back
             0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // Top front
            
            // Top face (Cyan) - normal: 0, 1, 0
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Back left
             0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Front right
             0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Back right
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Back left
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Front left
             0.5f,  0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  0.0f, 1.0f, 0.0f,  // Front right
            
            // Bottom face (Magenta) - normal: 0, -1, 0
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Back left
             0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Back right
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Front right
            -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Back left
             0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f,  // Front right
            -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, -1.0f, 0.0f   // Front left
        };

        // Create vertex buffer
        m_VertexBuffer = std::make_unique<LGE::VertexBuffer>(vertices, static_cast<uint32_t>(sizeof(vertices)));

        // Create vertex array and set up attributes manually (position + color + normal)
        m_VertexArray = std::make_unique<LGE::VertexArray>();
        m_VertexArray->Bind();
        m_VertexBuffer->Bind();
        
        // Set up vertex attributes: position (0), color (1), normal (2)
        // Stride: 9 floats (3 pos + 3 color + 3 normal)
        // Position
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
        // Color
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
        // Normal
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
        
        m_VertexArray->Unbind();
        m_VertexBuffer->Unbind();

        // Grid is now handled by GridRenderer (created above)

        // Create GameObject for the cube
        m_CubeObject = std::make_shared<LGE::GameObject>("Cube");
        m_CubeObject->SetPosition(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
        m_CubeObject->SetRotation(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
        m_CubeObject->SetScale(LGE::Math::Vector3(1.0f, 1.0f, 1.0f));
        m_CubeObject->SetSelected(true);
        
        // Add to GameObjects list
        m_GameObjects.push_back(m_CubeObject);

        // Setup UI panels
        m_MainMenuBar = std::make_unique<LGE::MainMenuBar>();
        m_Toolbar = std::make_unique<LGE::Toolbar>();
        m_Hierarchy = std::make_unique<LGE::Hierarchy>();
        m_Inspector = std::make_unique<LGE::Inspector>();
        m_ContentBrowser = std::make_unique<LGE::ContentBrowser>();
        m_Profiler = std::make_unique<LGE::Profiler>();
        m_SceneViewport = std::make_unique<LGE::SceneViewport>();
        m_Details = std::make_unique<LGE::Details>();
        m_Preferences = std::make_unique<LGE::Preferences>();
        m_ProjectSettings = std::make_unique<LGE::ProjectSettings>();
        
        // Connect preferences and project settings to menu bar
        m_MainMenuBar->SetPreferences(m_Preferences.get());
        m_MainMenuBar->SetProjectSettings(m_ProjectSettings.get());
        
        // Setup camera
        float aspectRatio = static_cast<float>(m_Window->GetWidth()) / static_cast<float>(m_Window->GetHeight());
        m_Camera = std::make_unique<LGE::Camera>();
        m_Camera->SetPerspective(45.0f, aspectRatio, 0.1f, 100.0f);
        m_Camera->SetPosition(LGE::Math::Vector3(0.0f, 0.0f, 3.0f));
        m_Camera->SetTarget(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
        
        // Set camera to viewport
        m_SceneViewport->SetCamera(m_Camera.get());
        
        // Set up Hierarchy callback for creating GameObjects
        m_Hierarchy->SetOnCreateGameObject([this](const std::string& type) {
            if (type == "Empty") {
                // Create empty GameObject
                auto emptyObj = std::make_shared<LGE::GameObject>("GameObject");
                emptyObj->SetPosition(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
                emptyObj->SetRotation(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
                emptyObj->SetScale(LGE::Math::Vector3(1.0f, 1.0f, 1.0f));
                
                m_GameObjects.push_back(emptyObj);
                
                // Update UI panels
                m_SceneViewport->SetGameObjects(m_GameObjects);
                m_Hierarchy->SetGameObjects(m_GameObjects);
                m_Hierarchy->SetSelectedObject(emptyObj.get());
                m_SceneViewport->SetSelectedObject(emptyObj.get());
                m_Inspector->SetSelectedObject(emptyObj.get());
            }
        });
        
        // Set cube as selected object in viewport and panels
        if (m_CubeObject) {
            m_SceneViewport->SetGameObjects(m_GameObjects);
            m_SceneViewport->SetSelectedObject(m_CubeObject.get());
            m_Hierarchy->SetGameObjects(m_GameObjects);
            m_Hierarchy->SetSelectedObject(m_CubeObject.get());
            m_Inspector->SetSelectedObject(m_CubeObject.get());
        }
        
        // Setup initial docking layout (only on first run)
        static bool firstTime = true;
        if (firstTime) {
            firstTime = false;
            
            // Wait for next frame to ensure ImGui is ready
            // We'll set up the layout in OnRender after first frame
        }

        // Setup camera controller
        m_CameraController = std::make_unique<LGE::CameraController>(m_Camera.get());
        m_CameraController->SetMovementSpeed(5.0f);
        m_CameraController->SetPanSpeed(1.0f);  // Reduced pan speed for smoother panning
        m_CameraController->SetZoomSpeed(5.0f);  // Increased base zoom speed
        m_CameraController->SetMouseSensitivity(0.05f);
        m_CameraController->SetZoomLimits(0.5f, 20.0f);  // Min 0.5 units, Max 20 units

        // Setup skybox
        m_Skybox = std::make_unique<LGE::Skybox>();
        m_Skybox->Init();
        
        // Load HDR skybox texture (supports both .hdr and .exr)
        if (!m_Skybox->LoadHDR("assets/EXR Sky/kloofendal_48d_partly_cloudy_puresky_4k.exr")) {
            // Fallback to gradient if HDR fails to load
            m_Skybox->SetTopColor(LGE::Math::Vector3(0.5f, 0.7f, 1.0f));      // Light blue
            m_Skybox->SetBottomColor(LGE::Math::Vector3(0.8f, 0.9f, 1.0f));   // Light gray-blue
            LGE::Log::Warn("Failed to load HDR skybox, using gradient fallback");
        }

        // Setup sphere mesh for PBR test balls
        int sphereSegments = 32;
        float sphereRadius = 0.5f;
        auto sphereVertices = GenerateSphereVertices(sphereRadius, sphereSegments);
        auto sphereIndices = GenerateSphereIndices(sphereSegments);
        m_SphereIndexCount = static_cast<uint32_t>(sphereIndices.size());
        
        // Create sphere vertex buffer
        m_SphereVertexBuffer = std::make_unique<LGE::VertexBuffer>(sphereVertices.data(), 
            static_cast<uint32_t>(sphereVertices.size() * sizeof(float)));
        
        // Create sphere vertex array
        m_SphereVertexArray = std::make_unique<LGE::VertexArray>();
        m_SphereVertexArray->Bind();
        
        // Set vertex attributes: position(0), normal(1), texCoord(2), tangent(3), bitangent(4)
        m_SphereVertexBuffer->Bind();
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);  // Position
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));  // Normal
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));  // TexCoord
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));  // Tangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));  // Bitangent
        
        // Create sphere index buffer
        m_SphereIndexBuffer = std::make_unique<LGE::IndexBuffer>(sphereIndices.data(), 
            static_cast<uint32_t>(sphereIndices.size()));
        
        m_SphereVertexArray->Unbind();
        
        // Load PBR shader
        std::string pbrVertSource = LGE::FileSystem::ReadFile("assets/shaders/Luminite/PBR.vert");
        std::string pbrFragSource = LGE::FileSystem::ReadFile("assets/shaders/Luminite/PBR.frag");
        
        if (pbrVertSource.empty() || pbrFragSource.empty()) {
            LGE::Log::Error("Failed to load PBR shader files!");
        } else {
            m_PBRShader = std::make_shared<LGE::Shader>(pbrVertSource, pbrFragSource);
            if (m_PBRShader->GetRendererID() == 0) {
                LGE::Log::Error("Failed to compile PBR shader!");
            } else {
                LGE::Log::Info("PBR shader loaded successfully!");
            }
        }
        
        // Create test ball GameObjects
        // Gray ball (dielectric, non-metallic)
        m_GrayBallObject = std::make_shared<LGE::GameObject>("GrayBall");
        m_GrayBallObject->SetPosition(LGE::Math::Vector3(-1.0f, 0.5f, 0.0f));
        m_GrayBallObject->SetRotation(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
        m_GrayBallObject->SetScale(LGE::Math::Vector3(1.0f, 1.0f, 1.0f));
        m_GameObjects.push_back(m_GrayBallObject);
        
        // Metal ball (metallic)
        m_MetalBallObject = std::make_shared<LGE::GameObject>("MetalBall");
        m_MetalBallObject->SetPosition(LGE::Math::Vector3(1.0f, 0.5f, 0.0f));
        m_MetalBallObject->SetRotation(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
        m_MetalBallObject->SetScale(LGE::Math::Vector3(1.0f, 1.0f, 1.0f));
        m_GameObjects.push_back(m_MetalBallObject);
        
        // Update GameObjects list in viewport
        if (m_SceneViewport) {
            m_SceneViewport->SetGameObjects(m_GameObjects);
        }
        
        LGE::Log::Info("Cube setup complete!");
        LGE::Log::Info("PBR test balls setup complete!");
        LGE::Log::Info("Camera initialized!");
        LGE::Log::Info("Skybox initialized!");
        LGE::Log::Info("Controls: WASD - Move, Right Click + Drag - Rotate, Mouse Wheel - Zoom, Middle Mouse + Drag - Pan");
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update animation time
        m_Time += deltaTime;
        
        // Update camera controller
        if (m_CameraController) {
            m_CameraController->OnUpdate(deltaTime);
        }
        
        // Update profiler
        if (m_Profiler) {
            m_Profiler->Update(deltaTime);
        }
    }

    void OnRender() override {
        // Render main menu bar
        if (m_MainMenuBar) {
            m_MainMenuBar->OnUIRender();
        }
        
        // Render toolbar before dock space
        float menuBarHeight = ImGui::GetFrameHeight();
        float toolbarHeight = 28.0f; // Fixed toolbar height
        if (m_Toolbar) {
            m_Toolbar->OnUIRender();
        }
        
        // Create dock space over viewport
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        // Account for menu bar and toolbar
        float topOffset = menuBarHeight + toolbarHeight;
        
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, viewport->WorkPos.y + topOffset));
        ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - topOffset));
        ImGui::SetNextWindowViewport(viewport->ID);
        
        ImGuiWindowFlags host_window_flags = 0;
        host_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking;
        host_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
        
        char label[32];
        snprintf(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);
        
        ImGui::Begin(label, NULL, host_window_flags);
        ImGui::PopStyleVar(5);
        
        // Create dock space (below toolbar)
        ImGuiID dockspace_id = ImGui::GetID("DockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
        
        // Setup initial docking layout (only on first frame)
        static bool firstTime = true;
        if (firstTime) {
            firstTime = false;
            
            // Build initial layout using DockBuilder API (internal API)
            ImGuiDockNode* node = ImGui::DockBuilderGetNode(dockspace_id);
            if (node == nullptr) {
                ImGui::DockBuilderRemoveNode(dockspace_id); // Clear any existing layout
                ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(viewport->WorkSize.x, viewport->WorkSize.y - topOffset));
                
                // Split the dock space - Unreal Engine layout: Outliner (left top), Details (left middle), Content Browser (left bottom), Place Actors (right)
                ImGuiID dockMain = dockspace_id;
                
                // Split left side for Outliner, Details, and Content Browser
                ImGuiID dockIdLeft = ImGui::DockBuilderSplitNode(dockMain, ImGuiDir_Left, 0.2f, nullptr, &dockMain);
                
                // Split left side vertically: Outliner (top), Details (middle), Content Browser (bottom)
                ImGuiID dockIdLeftTop = dockIdLeft;
                ImGuiID dockIdLeftMiddle = ImGui::DockBuilderSplitNode(dockIdLeftTop, ImGuiDir_Down, 0.4f, nullptr, &dockIdLeftTop);
                ImGuiID dockIdLeftBottom = ImGui::DockBuilderSplitNode(dockIdLeftMiddle, ImGuiDir_Down, 0.5f, nullptr, &dockIdLeftMiddle);
                
                // Dock windows
                ImGui::DockBuilderDockWindow("Outliner", dockIdLeftTop);  // Left top
                ImGui::DockBuilderDockWindow("Details", dockIdLeftMiddle);  // Left middle
                ImGui::DockBuilderDockWindow("Content Browser", dockIdLeftBottom);  // Left bottom
                ImGui::DockBuilderDockWindow("Scene Viewport", dockMain);  // Center
                ImGui::DockBuilderDockWindow("Profiler", dockIdLeftBottom);  // Dock with Content Browser (can be split)
                
                ImGui::DockBuilderFinish(dockspace_id);
            }
        }
        
        // Render all dockable panels
        
        // Update GameObjects list in viewport (in case new objects were added)
        if (m_SceneViewport) {
            m_SceneViewport->SetGameObjects(m_GameObjects);
        }
        
        if (m_Hierarchy) {
            m_Hierarchy->OnUIRender();
            // Sync selection from Hierarchy to other panels
            if (m_Hierarchy->GetSelectedObject() != m_SceneViewport->GetSelectedObject()) {
                m_SceneViewport->SetSelectedObject(m_Hierarchy->GetSelectedObject());
                m_Inspector->SetSelectedObject(m_Hierarchy->GetSelectedObject());
            }
        }
        
        if (m_SceneViewport) {
            m_SceneViewport->OnUIRender();
            // Sync selection from SceneViewport to other panels
            if (m_SceneViewport->GetSelectedObject() != m_Hierarchy->GetSelectedObject()) {
                m_Hierarchy->SetSelectedObject(m_SceneViewport->GetSelectedObject());
                m_Inspector->SetSelectedObject(m_SceneViewport->GetSelectedObject());
            }
        }
        
        if (m_Inspector) {
            m_Inspector->OnUIRender();
        }
        
        if (m_ContentBrowser) {
            m_ContentBrowser->OnUIRender();
        }
        
        if (m_Profiler) {
            m_Profiler->OnUIRender();
        }
        
        if (m_Preferences) {
            m_Preferences->OnUIRender();
        }
        
        if (m_ProjectSettings) {
            m_ProjectSettings->OnUIRender();
        }
        
        ImGui::End();
        
        // Render scene to framebuffer (everything should render here)
        if (m_SceneViewport && m_SceneViewport->GetWidth() > 0 && m_SceneViewport->GetHeight() > 0) {
            // Begin rendering to framebuffer
            m_SceneViewport->BeginRender();
            
            // Render skybox first (so it's behind everything) with cloud movement
            if (m_Skybox) {
                // Very slow horizontal cloud movement (simple linear offset)
                float cloudSpeed = 0.001f; // Extremely slow - subtle cloud drift
                float cloudOffset = m_Time * cloudSpeed;
                // Keep offset in 0-1 range for seamless looping
                cloudOffset = std::fmod(cloudOffset, 1.0f);
                m_Skybox->Render(*m_Camera, cloudOffset);
            }
            
            // Render grid using GridRenderer (before other objects so it's behind them)
            if (m_SceneViewport && m_SceneViewport->IsGridVisible() && m_GridRenderer && m_GridRenderer->GetShader()) {
                m_GridRenderer->Draw(m_Camera->GetViewProjectionMatrix());
            }
            
            // Render the cube with lit material
            if (m_LitMaterial && m_LitMaterial->GetShader()) {
                m_LitMaterial->Bind();
                auto shader = m_LitMaterial->GetShader();
                
                // Set view-projection matrix
                shader->SetUniformMat4("u_ViewProjection", m_Camera->GetViewProjectionMatrix().GetData());
                
                // Set model matrix from GameObject transform
                // Always use the cube's GameObject transform (cube is a test object)
                if (m_CubeObject) {
                    const LGE::Math::Matrix4& modelMatrix = m_CubeObject->GetTransformMatrix();
                    shader->SetUniformMat4("u_Model", modelMatrix.GetData());
                } else {
                    // Fallback: use identity matrix
                    LGE::Math::Matrix4 modelMatrix;
                    shader->SetUniformMat4("u_Model", modelMatrix.GetData());
                }
                
                // Set default lighting values
                shader->SetUniform3f("u_LightDirection", 0.0f, -1.0f, 0.0f);
                shader->SetUniform3f("u_LightColor", 1.0f, 1.0f, 1.0f);
                shader->SetUniform1f("u_LightIntensity", 1.0f);
                
                // Set view position for specular lighting
                LGE::Math::Vector3 viewPos = m_Camera->GetPosition();
                shader->SetUniform3f("u_ViewPos", viewPos.x, viewPos.y, viewPos.z);
                
                // Material color is already set by m_LitMaterial->Bind()
                // But we need to set the use vertex color flag
                shader->SetUniform1i("u_UseVertexColor", 0); // Use material color, not vertex color
                
                m_VertexArray->Bind();
                // Draw cube: 6 faces * 2 triangles * 3 vertices = 36 vertices
                glDrawArrays(GL_TRIANGLES, 0, 36);
                m_VertexArray->Unbind();
                m_LitMaterial->Unbind();
            }
            
            // Render PBR test balls
            if (m_PBRShader && m_PBRShader->GetRendererID() != 0 && m_SphereVertexArray && m_SphereIndexBuffer) {
                m_PBRShader->Bind();
                
                // Set matrices
                m_PBRShader->SetUniformMat4("u_Model", LGE::Math::Matrix4::Identity().GetData());
                m_PBRShader->SetUniformMat4("u_View", m_Camera->GetViewMatrix().GetData());
                m_PBRShader->SetUniformMat4("u_Projection", m_Camera->GetProjectionMatrix().GetData());
                m_PBRShader->SetUniformMat4("u_ViewProjection", m_Camera->GetViewProjectionMatrix().GetData());
                
                // Set camera position
                LGE::Math::Vector3 cameraPos = m_Camera->GetPosition();
                m_PBRShader->SetUniform3f("u_CameraPos", cameraPos.x, cameraPos.y, cameraPos.z);
                
                // Set default lighting values
                m_PBRShader->SetUniform3f("u_LightDirection", 0.0f, -1.0f, 0.0f);
                m_PBRShader->SetUniform3f("u_LightColor", 1.0f, 1.0f, 1.0f);
                m_PBRShader->SetUniform1f("u_LightIntensity", 1.0f);
                
                // Set default ambient
                m_PBRShader->SetUniform3f("u_AmbientColor", 0.1f, 0.1f, 0.1f);
                m_PBRShader->SetUniform1f("u_AmbientIntensity", 1.0f);
                
                // Set exposure and tonemapping
                m_PBRShader->SetUniform1f("u_Exposure", 1.0f);
                m_PBRShader->SetUniform1i("u_UseTonemapping", 0);  // Disable tonemapping
                m_PBRShader->SetUniform1i("u_UseIBL", 0);  // Disable IBL
                
                // Enable depth testing and face culling
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LEQUAL);
                glEnable(GL_CULL_FACE);
                glCullFace(GL_BACK);
                glFrontFace(GL_CCW);
                
                m_SphereVertexArray->Bind();
                m_SphereIndexBuffer->Bind();
                
                // Render gray ball (dielectric)
                if (m_GrayBallObject) {
                    const LGE::Math::Matrix4& modelMatrix = m_GrayBallObject->GetTransformMatrix();
                    m_PBRShader->SetUniformMat4("u_Model", modelMatrix.GetData());
                    
                    // Material properties: gray, non-metallic, medium roughness
                    m_PBRShader->SetUniform3f("u_BaseColor", 0.8f, 0.8f, 0.8f);
                    m_PBRShader->SetUniform1f("u_Metalness", 0.0f);
                    m_PBRShader->SetUniform1f("u_Roughness", 0.5f);
                    m_PBRShader->SetUniform1f("u_NormalScale", 1.0f);
                    m_PBRShader->SetUniform1f("u_AO", 1.0f);
                    m_PBRShader->SetUniform3f("u_Emissive", 0.0f, 0.0f, 0.0f);
                    m_PBRShader->SetUniform1f("u_EmissiveIntensity", 1.0f);
                    
                    // No textures
                    m_PBRShader->SetUniform1i("u_HasBaseColorTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasMetalnessTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasRoughnessTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasNormalTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasAOTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasEmissiveTexture", 0);
                    
                    glDrawElements(GL_TRIANGLES, m_SphereIndexCount, GL_UNSIGNED_INT, nullptr);
                }
                
                // Render metal ball (metallic)
                if (m_MetalBallObject) {
                    const LGE::Math::Matrix4& modelMatrix = m_MetalBallObject->GetTransformMatrix();
                    m_PBRShader->SetUniformMat4("u_Model", modelMatrix.GetData());
                    
                    // Material properties: metallic, smooth
                    m_PBRShader->SetUniform3f("u_BaseColor", 0.95f, 0.93f, 0.88f);  // Slightly warm metal
                    m_PBRShader->SetUniform1f("u_Metalness", 1.0f);
                    m_PBRShader->SetUniform1f("u_Roughness", 0.1f);  // Smooth metal
                    m_PBRShader->SetUniform1f("u_NormalScale", 1.0f);
                    m_PBRShader->SetUniform1f("u_AO", 1.0f);
                    m_PBRShader->SetUniform3f("u_Emissive", 0.0f, 0.0f, 0.0f);
                    m_PBRShader->SetUniform1f("u_EmissiveIntensity", 1.0f);
                    
                    // No textures
                    m_PBRShader->SetUniform1i("u_HasBaseColorTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasMetalnessTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasRoughnessTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasNormalTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasAOTexture", 0);
                    m_PBRShader->SetUniform1i("u_HasEmissiveTexture", 0);
                    
                    glDrawElements(GL_TRIANGLES, m_SphereIndexCount, GL_UNSIGNED_INT, nullptr);
                }
                
                m_SphereIndexBuffer->Unbind();
                m_SphereVertexArray->Unbind();
                m_PBRShader->Unbind();
            }
            
            // ImGuizmo is rendered in OnUIRender, not here
            
            // End rendering to framebuffer (restores viewport)
            m_SceneViewport->EndRender();
        }
    }

    void Shutdown() override {
        m_ContentBrowser.reset();
        m_Inspector.reset();
        m_Hierarchy.reset();
        m_Profiler.reset();
        m_Preferences.reset();
        m_ProjectSettings.reset();
        m_Toolbar.reset();
        m_MainMenuBar.reset();
        m_SceneViewport.reset();
        m_GridRenderer.reset();
        m_LitMaterial.reset();
        m_Shader.reset();
        m_VertexBuffer.reset();
        m_VertexArray.reset();
        m_Skybox.reset();
        m_CameraController.reset();
        m_Camera.reset();
        LGE::Application::Shutdown();
    }

private:
    std::shared_ptr<LGE::Shader> m_Shader; // Kept for backward compatibility
    std::shared_ptr<LGE::Material> m_LitMaterial;
    std::unique_ptr<LGE::GridRenderer> m_GridRenderer;
    std::unique_ptr<LGE::VertexBuffer> m_VertexBuffer;
    std::unique_ptr<LGE::VertexArray> m_VertexArray;
    std::shared_ptr<LGE::GameObject> m_CubeObject; // GameObject for the cube
    std::vector<std::shared_ptr<LGE::GameObject>> m_GameObjects; // All GameObjects in the scene
    std::unique_ptr<LGE::Camera> m_Camera;
    std::unique_ptr<LGE::CameraController> m_CameraController;
    std::unique_ptr<LGE::Skybox> m_Skybox;
    std::map<LGE::GameObject*, std::unique_ptr<LGE::DirectionalLight>> m_DirectionalLights; // DirectionalLights attached to GameObjects
    std::unique_ptr<LGE::SceneViewport> m_SceneViewport;
    std::unique_ptr<LGE::Details> m_Details;
    std::unique_ptr<LGE::Hierarchy> m_Hierarchy;
    std::unique_ptr<LGE::Inspector> m_Inspector;
    std::unique_ptr<LGE::ContentBrowser> m_ContentBrowser;
    std::unique_ptr<LGE::Profiler> m_Profiler;
    std::unique_ptr<LGE::Preferences> m_Preferences;
    std::unique_ptr<LGE::ProjectSettings> m_ProjectSettings;
    std::unique_ptr<LGE::Toolbar> m_Toolbar;
    std::unique_ptr<LGE::MainMenuBar> m_MainMenuBar;
    
    // Animation time
    float m_Time = 0.0f;
    
    // PBR test balls
    std::unique_ptr<LGE::VertexBuffer> m_SphereVertexBuffer;
    std::unique_ptr<LGE::VertexArray> m_SphereVertexArray;
    std::unique_ptr<LGE::IndexBuffer> m_SphereIndexBuffer;
    uint32_t m_SphereIndexCount = 0;
    std::shared_ptr<LGE::Shader> m_PBRShader;
    std::shared_ptr<LGE::GameObject> m_GrayBallObject;
    std::shared_ptr<LGE::GameObject> m_MetalBallObject;
    
    // Helper functions for sphere generation
    std::vector<float> GenerateSphereVertices(float radius, int segments);
    std::vector<uint32_t> GenerateSphereIndices(int segments);
};

// Sphere generation helper functions
std::vector<float> ExampleApp::GenerateSphereVertices(float radius, int segments) {
    std::vector<float> vertices;
    vertices.reserve((segments + 1) * (segments + 1) * 14); // position(3) + normal(3) + texCoord(2) + tangent(3) + bitangent(3)
    
    for (int y = 0; y <= segments; ++y) {
        for (int x = 0; x <= segments; ++x) {
            float xSegment = static_cast<float>(x) / static_cast<float>(segments);
            float ySegment = static_cast<float>(y) / static_cast<float>(segments);
            float xPos = std::cos(xSegment * 2.0f * 3.14159f) * std::sin(ySegment * 3.14159f);
            float yPos = std::cos(ySegment * 3.14159f);
            float zPos = std::sin(xSegment * 2.0f * 3.14159f) * std::sin(ySegment * 3.14159f);
            
            // Position
            vertices.push_back(xPos * radius);
            vertices.push_back(yPos * radius);
            vertices.push_back(zPos * radius);
            
            // Normal (same as position for unit sphere)
            vertices.push_back(xPos);
            vertices.push_back(yPos);
            vertices.push_back(zPos);
            
            // Texture coordinates
            vertices.push_back(xSegment);
            vertices.push_back(ySegment);
            
            // Tangent (perpendicular to normal in the x-z plane)
            float tangentX = -std::sin(xSegment * 2.0f * 3.14159f);
            float tangentY = 0.0f;
            float tangentZ = std::cos(xSegment * 2.0f * 3.14159f);
            vertices.push_back(tangentX);
            vertices.push_back(tangentY);
            vertices.push_back(tangentZ);
            
            // Bitangent (cross product of normal and tangent)
            float bitangentX = std::cos(xSegment * 2.0f * 3.14159f) * std::cos(ySegment * 3.14159f);
            float bitangentY = -std::sin(ySegment * 3.14159f);
            float bitangentZ = std::sin(xSegment * 2.0f * 3.14159f) * std::cos(ySegment * 3.14159f);
            vertices.push_back(bitangentX);
            vertices.push_back(bitangentY);
            vertices.push_back(bitangentZ);
        }
    }
    
    return vertices;
}

std::vector<uint32_t> ExampleApp::GenerateSphereIndices(int segments) {
    std::vector<uint32_t> indices;
    indices.reserve(segments * segments * 6);
    
    for (int y = 0; y < segments; ++y) {
        for (int x = 0; x < segments; ++x) {
            int first = y * (segments + 1) + x;
            int second = first + segments + 1;
            
            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);
            
            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
    
    return indices;
}

int main() {
    ExampleApp app;

    if (!app.Initialize()) {
        LGE::Log::Error("Failed to initialize application!");
        return -1;
    }

    app.Run();
    app.Shutdown();

    return 0;
}

