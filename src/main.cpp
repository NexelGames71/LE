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
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Window.h"
#include "LGE/rendering/Shader.h"
#include "LGE/rendering/VertexBuffer.h"
#include "LGE/rendering/VertexArray.h"
#include "LGE/rendering/IndexBuffer.h"
#include "LGE/rendering/Camera.h"
#include "LGE/rendering/CameraController.h"
#include "LGE/rendering/Skybox.h"
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
#include "LGE/ui/Console.h"
#include "LGE/ui/Profiler.h"
#include "LGE/ui/Preferences.h"
#include "LGE/ui/ProjectSettings.h"
#include "LGE/ui/TextureImporter.h"
#include "LGE/ui/ProjectBrowser.h"
#include "LGE/core/SplashScreen.h"
#include "LGE/core/LayerStack.h"
#include "LGE/rendering/TextureManager.h"
#include "LGE/core/Input.h"
#include "LGE/core/scene/GameObject.h"
#include "LGE/core/scene/World.h"
#include "LGE/core/scene/SceneManager.h"
#include "LGE/core/scene/components/Transform.h"
#include "LGE/core/scene/components/MeshRenderer.h"
#include "LGE/core/scene/components/CameraComponent.h"
#include "LGE/core/scene/components/LightComponent.h"
#include "LGE/core/scene/components/SkyLightComponent.h"
#include "LGE/rendering/Mesh.h"
#include "LGE/rendering/LightSystem.h"
#include "LGE/core/project/Project.h"
#include "LGE/core/filesystem/FileSystemManager.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <memory>
#include <cstdint>
#include <cmath>
#include <vector>
#include <iostream>

class ExampleApp : public LGE::Application {
public:
    ExampleApp() : LGE::Application("Luma Engine") {}

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
        
        // Initialize LightSystem
        m_LightSystem = std::make_unique<LGE::LightSystem>();
        if (!m_LightSystem->Initialize()) {
            LGE::Log::Error("Failed to initialize LightSystem!");
            return false;
        }
        
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


        // Setup UI panels
        // Initialize FileSystemManager first (will be initialized when project opens)
        m_FileSystemManager = std::make_unique<LGE::FileSystemManager>();
        
        // Initialize Project Browser
        m_ProjectBrowser = std::make_unique<LGE::ProjectBrowser>();
        m_ProjectBrowser->SetOnProjectOpened([this](std::shared_ptr<LGE::Project> project) {
            m_Project = project;
            m_ShowProjectBrowser = false;
            
            LGE::Log::Info("Project opened: " + (project ? project->GetInfo().name : "null"));
            LGE::Log::Info("Project loaded: " + std::string(project && project->IsLoaded() ? "yes" : "no"));
            
            // Initialize FileSystemManager with the project
            if (m_FileSystemManager && m_Project && m_Project->IsLoaded()) {
                if (m_FileSystemManager->Initialize(m_Project)) {
                    // Perform initial asset scan
                    m_FileSystemManager->ScanAssets();
                    
                    // Enable hot reloading
                    m_FileSystemManager->EnableHotReloading(true);
                    
                    LGE::Log::Info("File system initialized for project: " + m_Project->GetInfo().name);
                } else {
                    LGE::Log::Error("Failed to initialize FileSystemManager");
                }
            } else {
                LGE::Log::Error("Cannot initialize FileSystemManager - project not loaded or manager not available");
            }
            
            // Initialize ContentBrowser with project
            if (m_ContentBrowser && m_Project) {
                m_ContentBrowser->SetProjectRoot(m_Project->GetAssetsPath());
            }
            
            // Initialize ProjectSettings with project and systems
            if (m_ProjectSettings && m_Project) {
                m_ProjectSettings->SetProject(m_Project);
                m_ProjectSettings->SetLightSystem(m_LightSystem.get());
                m_ProjectSettings->SetSceneManager(m_SceneManager.get());
                m_ProjectSettings->LoadLightingSettings();
            }
            
            // Load the default scene using SceneManager
            if (m_Project && m_Project->IsLoaded() && m_SceneManager) {
                std::string defaultScene = m_Project->GetDefaultScene();
                if (!defaultScene.empty()) {
                    std::string scenesPath = m_Project->GetScenesPath();
                    std::string scenePath = LGE::FileSystem::JoinPath(scenesPath, defaultScene);
                    
                    if (LGE::FileSystem::Exists(scenePath)) {
                        if (m_SceneManager->LoadScene(scenePath)) {
                            LGE::Log::Info("Loaded default scene: " + scenePath);
                        } else {
                            LGE::Log::Error("Failed to load default scene: " + scenePath);
                        }
                    } else {
                        LGE::Log::Warn("Default scene not found: " + scenePath);
                    }
                }
            }
        });
        
        // Show project browser initially
        m_ShowProjectBrowser = true;
        
        m_MainMenuBar = std::make_unique<LGE::MainMenuBar>();
        m_Toolbar = std::make_unique<LGE::Toolbar>();
        m_Hierarchy = std::make_unique<LGE::Hierarchy>();
        m_Inspector = std::make_unique<LGE::Inspector>();
        
        // Create SceneManager
        m_SceneManager = std::make_unique<LGE::SceneManager>();
        
        // Register scene event callbacks to update UI
        m_SceneManager->RegisterSceneEventCallback([this](LGE::SceneEvent event, std::shared_ptr<LGE::World> world) {
            this->OnSceneEvent(event, world);
        });
        
        // Create empty editor world
        m_SceneManager->CreateScene("EmptyScene");
        
        m_ContentBrowser = std::make_unique<LGE::ContentBrowser>();
        m_ContentBrowser->SetOnSceneOpened([this](const std::string& scenePath) {
            // Load the scene using SceneManager
            if (m_SceneManager) {
                if (m_SceneManager->LoadScene(scenePath)) {
                    LGE::Log::Info("Scene loaded successfully: " + scenePath);
                } else {
                    LGE::Log::Error("Failed to load scene: " + scenePath);
                }
            }
        });
        
        // Handle scene rename
        m_ContentBrowser->SetOnSceneRenamed([this](const std::string& oldPath, const std::string& newPath) {
            // Extract new scene name from file path
            std::string newFileName = LGE::FileSystem::GetFileName(newPath);
            std::string newSceneName = LGE::FileSystem::GetFileNameWithoutExtension(newFileName);
            
            // Update the scene file's internal name
            auto world = LGE::World::LoadFromFile(newPath);
            if (world) {
                world->SetName(newSceneName);
                world->SaveToFile(newPath);
            }
            
            // If the renamed scene is currently loaded, reload it to update the name
            if (m_SceneManager && m_SceneManager->GetCurrentScenePath() == oldPath) {
                // Reload the scene to get the updated name
                if (m_SceneManager->LoadScene(newPath)) {
                    LGE::Log::Info("Updated scene name to: " + newSceneName);
                }
            }
        });
        m_Profiler = std::make_unique<LGE::Profiler>();
        m_SceneViewport = std::make_unique<LGE::SceneViewport>();
        m_Details = std::make_unique<LGE::Details>();
        m_Console = std::make_unique<LGE::Console>();
        
        // Register Console callback to receive log messages
        LGE::Log::RegisterCallback([this](LGE::LogLevel level, const std::string& message) {
            if (m_Console) {
                m_Console->AddLog(level, message);
            }
        });
        m_Preferences = std::make_unique<LGE::Preferences>();
        m_ProjectSettings = std::make_unique<LGE::ProjectSettings>();
        m_TextureImporter = std::make_unique<LGE::TextureImporter>();
        m_TextureManager = std::make_unique<LGE::TextureManager>();
        
        // Connect texture manager to importer
        m_TextureImporter->SetTextureManager(m_TextureManager.get());
        
        // Connect preferences and project settings to menu bar
        m_MainMenuBar->SetPreferences(m_Preferences.get());
        m_MainMenuBar->SetProjectSettings(m_ProjectSettings.get());
        m_MainMenuBar->SetTextureImporter(m_TextureImporter.get());
        
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
            LGE::Log::Info("SetOnCreateGameObject callback called with type: " + type);
            
            auto activeWorld = m_SceneManager ? m_SceneManager->GetActiveWorld() : nullptr;
            if (!activeWorld) {
                LGE::Log::Error("Cannot create GameObject: No active world");
                return;
            }
            
            LGE::Log::Info("Active world found: " + activeWorld->GetName());
            
            std::shared_ptr<LGE::GameObject> newObject;
            
            if (type == "Empty") {
                // Create empty GameObject
                newObject = activeWorld->CreateGameObject("GameObject");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
                newObject->SetRotation(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
                newObject->SetScale(LGE::Math::Vector3(1.0f, 1.0f, 1.0f));
            }
            else if (type == "Cube") {
                // Create cube with MeshRenderer
                newObject = activeWorld->CreateGameObject("Cube");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 0.5f, 0.0f)); // Offset slightly up
                
                LGE::Log::Info("Creating Cube GameObject...");
                auto meshRenderer = newObject->AddComponent<LGE::MeshRenderer>();
                if (meshRenderer) {
                    LGE::Log::Info("MeshRenderer component added");
                    auto cubeMesh = LGE::PrimitiveMesh::CreateCube();
                    if (cubeMesh) {
                        LGE::Log::Info("Cube mesh created: " + cubeMesh->GetName() + " (vertices: " + std::to_string(cubeMesh->GetVertexCount()) + ")");
                        meshRenderer->SetMesh(cubeMesh);
                        // Assign default material
                        meshRenderer->SetMaterial(m_LitMaterial);
                        LGE::Log::Info("Created Cube with mesh and material");
                    } else {
                        LGE::Log::Error("Failed to create cube mesh");
                    }
                } else {
                    LGE::Log::Error("Failed to add MeshRenderer to Cube");
                }
            }
            else if (type == "Sphere") {
                // Create sphere with MeshRenderer
                newObject = activeWorld->CreateGameObject("Sphere");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 0.5f, 0.0f)); // Offset slightly up
                
                auto meshRenderer = newObject->AddComponent<LGE::MeshRenderer>();
                if (meshRenderer) {
                    auto sphereMesh = LGE::PrimitiveMesh::CreateSphere();
                    if (sphereMesh) {
                        meshRenderer->SetMesh(sphereMesh);
                        // Assign default material
                        meshRenderer->SetMaterial(m_LitMaterial);
                        LGE::Log::Info("Created Sphere with mesh and material");
                    } else {
                        LGE::Log::Error("Failed to create sphere mesh");
                    }
                } else {
                    LGE::Log::Error("Failed to add MeshRenderer to Sphere");
                }
            }
            else if (type == "Plane") {
                // Create plane with MeshRenderer
                newObject = activeWorld->CreateGameObject("Plane");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
                
                auto meshRenderer = newObject->AddComponent<LGE::MeshRenderer>();
                if (meshRenderer) {
                    auto planeMesh = LGE::PrimitiveMesh::CreatePlane();
                    if (planeMesh) {
                        meshRenderer->SetMesh(planeMesh);
                        // Assign default material
                        meshRenderer->SetMaterial(m_LitMaterial);
                        LGE::Log::Info("Created Plane with mesh and material");
                    } else {
                        LGE::Log::Error("Failed to create plane mesh");
                    }
                } else {
                    LGE::Log::Error("Failed to add MeshRenderer to Plane");
                }
            }
            else if (type == "Cylinder") {
                // Create cylinder with MeshRenderer
                newObject = activeWorld->CreateGameObject("Cylinder");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 0.5f, 0.0f)); // Offset slightly up
                
                auto meshRenderer = newObject->AddComponent<LGE::MeshRenderer>();
                if (meshRenderer) {
                    auto cylinderMesh = LGE::PrimitiveMesh::CreateCylinder();
                    if (cylinderMesh) {
                        meshRenderer->SetMesh(cylinderMesh);
                        // Assign default material
                        meshRenderer->SetMaterial(m_LitMaterial);
                        LGE::Log::Info("Created Cylinder with mesh and material");
                    } else {
                        LGE::Log::Error("Failed to create cylinder mesh");
                    }
                } else {
                    LGE::Log::Error("Failed to add MeshRenderer to Cylinder");
                }
            }
            else if (type == "Capsule") {
                // Create capsule with MeshRenderer
                newObject = activeWorld->CreateGameObject("Capsule");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 0.5f, 0.0f)); // Offset slightly up
                
                auto meshRenderer = newObject->AddComponent<LGE::MeshRenderer>();
                if (meshRenderer) {
                    auto capsuleMesh = LGE::PrimitiveMesh::CreateCapsule();
                    if (capsuleMesh) {
                        meshRenderer->SetMesh(capsuleMesh);
                        // Assign default material
                        meshRenderer->SetMaterial(m_LitMaterial);
                        LGE::Log::Info("Created Capsule with mesh and material");
                    } else {
                        LGE::Log::Error("Failed to create capsule mesh");
                    }
                } else {
                    LGE::Log::Error("Failed to add MeshRenderer to Capsule");
                }
            }
            else if (type == "Camera") {
                // Create camera
                newObject = activeWorld->CreateGameObject("Camera");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 0.0f, 3.0f));
                newObject->AddComponent<LGE::CameraComponent>();
            }
            else if (type == "DirectionalLight") {
                // Create directional light
                newObject = activeWorld->CreateGameObject("Directional Light");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 3.0f, 0.0f));
                newObject->SetRotation(LGE::Math::Vector3(-45.0f, 0.0f, 0.0f)); // Point downward
                auto light = newObject->AddComponent<LGE::LightComponent>();
                if (light) {
                    light->Type = LGE::LightType::Directional;
                    light->Color = LGE::Math::Vector3(1.0f, 1.0f, 1.0f);
                    light->Intensity = 1.0f;
                }
            }
            else if (type == "PointLight") {
                // Create point light
                newObject = activeWorld->CreateGameObject("Point Light");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 2.0f, 0.0f));
                auto light = newObject->AddComponent<LGE::LightComponent>();
                if (light) {
                    light->Type = LGE::LightType::Point;
                    light->Color = LGE::Math::Vector3(1.0f, 1.0f, 1.0f);
                    light->Intensity = 1.0f;
                    light->Range = 10.0f;
                }
            }
            else if (type == "SpotLight") {
                // Create spot light
                newObject = activeWorld->CreateGameObject("Spot Light");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 3.0f, 0.0f));
                newObject->SetRotation(LGE::Math::Vector3(-45.0f, 0.0f, 0.0f)); // Point downward
                auto light = newObject->AddComponent<LGE::LightComponent>();
                if (light) {
                    light->Type = LGE::LightType::Spot;
                    light->Color = LGE::Math::Vector3(1.0f, 1.0f, 1.0f);
                    light->Intensity = 1.0f;
                    light->Range = 10.0f;
                    light->InnerAngle = 0.349066f;  // ~20 degrees
                    light->OuterAngle = 0.523599f;  // ~30 degrees
                }
            }
            else if (type == "SkyLight") {
                // Create sky light (global environment light)
                newObject = activeWorld->CreateGameObject("Sky Light");
                newObject->SetPosition(LGE::Math::Vector3(0.0f, 0.0f, 0.0f));
                auto skyLight = newObject->AddComponent<LGE::SkyLightComponent>();
                if (skyLight) {
                    skyLight->Enabled = true;
                    skyLight->Intensity = 1.0f;
                    skyLight->UseDiffuseIBL = true;
                    skyLight->UseSpecularIBL = true;
                }
            }
            
            if (newObject) {
                // Focus camera on the new object
                if (m_Camera) {
                    LGE::Math::Vector3 objPos = newObject->GetPosition();
                    // Position camera to look at the object from a reasonable distance
                    LGE::Math::Vector3 cameraOffset = LGE::Math::Vector3(0.0f, 2.0f, 5.0f);
                    m_Camera->SetPosition(objPos + cameraOffset);
                    m_Camera->SetTarget(objPos);
                }
                
                // Update UI panels
                UpdateUIFromWorld();
                
                m_Hierarchy->SetSelectedObject(newObject.get());
                m_SceneViewport->SetSelectedObject(newObject.get());
                m_Inspector->SetSelectedObject(newObject.get());
            }
        });
        
        // Update UI from current world
        UpdateUIFromWorld();
        
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
        
        LGE::Log::Info("Scene setup complete!");
        LGE::Log::Info("Camera initialized!");
        LGE::Log::Info("Skybox initialized!");
        LGE::Log::Info("Controls: WASD - Move, Right Click + Drag - Rotate, Mouse Wheel - Zoom, Middle Mouse + Drag - Pan");
        return true;
    }

    void OnUpdate(float deltaTime) override {
        // Update animation time
        m_Time += deltaTime;
        
        // Only update editor systems if project is loaded
        if (!m_ShowProjectBrowser && m_Project && m_Project->IsLoaded()) {
            // Update camera controller
            if (m_CameraController) {
                m_CameraController->OnUpdate(deltaTime);
            }
            
            // Update profiler
            if (m_Profiler) {
                m_Profiler->Update(deltaTime);
            }
            
            // Update active world
            if (m_SceneManager) {
                auto activeWorld = m_SceneManager->GetActiveWorld();
                if (activeWorld) {
                    activeWorld->Update(deltaTime);
                }
            }
        }
    }
    
    // Scene event handler
    void OnSceneEvent(LGE::SceneEvent event, std::shared_ptr<LGE::World> world) {
        switch (event) {
            case LGE::SceneEvent::Loaded:
                UpdateUIFromWorld();
                if (world && m_Toolbar) {
                    m_Toolbar->SetCurrentSceneName(world->GetName());
                }
                break;
            case LGE::SceneEvent::Unloaded:
                // Clear UI references
                if (m_Hierarchy) {
                    m_Hierarchy->SetGameObjects({});
                }
                if (m_SceneViewport) {
                    m_SceneViewport->SetGameObjects({});
                }
                if (m_Inspector) {
                    m_Inspector->SetSelectedObject(nullptr);
                }
                if (m_Toolbar) {
                    m_Toolbar->SetCurrentSceneName("");
                }
                break;
            default:
                break;
        }
    }
    
    // Update UI from current world
    void UpdateUIFromWorld() {
        if (!m_SceneManager) return;
        
        auto activeWorld = m_SceneManager->GetActiveWorld();
        if (!activeWorld) return;
        
        auto worldObjects = activeWorld->GetAllGameObjects();
        
        if (m_SceneViewport) {
            m_SceneViewport->SetGameObjects(worldObjects);
        }
        if (m_Hierarchy) {
            m_Hierarchy->SetGameObjects(worldObjects);
        }
    }

    void OnRender() override {
        // Only show editor UI (menu bar, toolbar, etc.) if project is loaded
        if (!m_ShowProjectBrowser && m_Project && m_Project->IsLoaded()) {
            // Render main menu bar
            if (m_MainMenuBar) {
                m_MainMenuBar->OnUIRender();
            }
            
            // Render toolbar before dock space
            if (m_Toolbar) {
                m_Toolbar->OnUIRender();
            }
        }
        
        // Create dock space over viewport
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        // Account for menu bar and toolbar (only if project is loaded)
        float menuBarHeight = (!m_ShowProjectBrowser && m_Project && m_Project->IsLoaded()) ? ImGui::GetFrameHeight() : 0.0f;
        float toolbarHeight = (!m_ShowProjectBrowser && m_Project && m_Project->IsLoaded()) ? 28.0f : 0.0f;
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
        
        // Check if splash screen is still active
        bool splashScreenActive = false;
        auto& layerStack = GetLayerStack();
        const auto& overlays = layerStack.GetOverlays();
        for (const auto& overlay : overlays) {
            if (overlay && overlay->GetName() == "SplashScreen" && overlay->IsEnabled()) {
                // Check if splash screen is finished
                auto* splash = dynamic_cast<LGE::SplashScreen*>(overlay.get());
                if (splash && !splash->IsFinished()) {
                    splashScreenActive = true;
                    break;
                }
            }
        }
        
        // Show Project Browser if no project is loaded AND splash screen is finished
        if (m_ShowProjectBrowser && m_ProjectBrowser && !splashScreenActive) {
            // Don't create dock space when showing project browser
            ImGui::PopStyleVar(5);
            
            if (m_ProjectBrowser->OnUIRender()) {
                // Project was selected/created (handled by callback)
                // The callback already initializes everything
            }
        } else if (!m_ShowProjectBrowser && m_Project && m_Project->IsLoaded() && !splashScreenActive) {
            // Debug: Log that we're entering the dock space rendering
            static int frameCount = 0;
            frameCount++;
            if (frameCount <= 3) {
                LGE::Log::Info("Rendering dock space - project is loaded (frame " + std::to_string(frameCount) + ")");
                LGE::Log::Info("m_ShowProjectBrowser: " + std::string(m_ShowProjectBrowser ? "true" : "false"));
                LGE::Log::Info("m_Project: " + std::string(m_Project ? "valid" : "null"));
                if (m_Project) {
                    LGE::Log::Info("m_Project->IsLoaded(): " + std::string(m_Project->IsLoaded() ? "true" : "false"));
                }
            }
            // Only create dock space if project is loaded
            char label[32];
            snprintf(label, IM_ARRAYSIZE(label), "DockSpaceViewport_%08X", viewport->ID);
            
            ImGui::Begin(label, NULL, host_window_flags);
            ImGui::PopStyleVar(5);
            
            // Create dock space (below toolbar)
            ImGuiID dockspace_id = ImGui::GetID("DockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);
            
            // Only show dock space and editor if project is loaded
            // Setup initial docking layout (only on first frame after project is loaded)
            static std::shared_ptr<LGE::Project> lastProject = nullptr;
            static bool layoutSetup = false;
            if (lastProject != m_Project) {
                lastProject = m_Project;
                layoutSetup = false;
                LGE::Log::Info("New project opened, will setup dock layout");
            }
            
            // Render all dockable panels first (they need to exist before docking)
            {
            // Render all dockable panels (only if project is loaded)
            
            // Update UI from current world
            UpdateUIFromWorld();
            
            // Render Outliner (Hierarchy) and Details (Inspector) panels
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
            
            if (m_Console) {
                m_Console->OnUIRender();
            }
            
            if (m_Preferences) {
                m_Preferences->OnUIRender();
            }
            
            if (m_ProjectSettings) {
                m_ProjectSettings->OnUIRender();
            }
            
            if (m_TextureImporter) {
                m_TextureImporter->OnUIRender();
            }
            }
            
            // Setup dock layout AFTER windows are created (they need to exist first)
            // Only set up default layout if no saved layout exists (first time or ini file deleted)
            if (!layoutSetup) {
                // Check if dock space already has a layout (from saved ini file)
                ImGuiDockNode* dockNode = ImGui::DockBuilderGetNode(dockspace_id);
                bool hasExistingLayout = dockNode != nullptr && (dockNode->ChildNodes[0] != nullptr || dockNode->ChildNodes[1] != nullptr);
                
                if (!hasExistingLayout) {
                    // No saved layout exists, set up default layout
                    layoutSetup = true;
                    
                    // Build initial layout using DockBuilder API (internal API)
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
                    
                    // Dock windows (they should exist now since we rendered them above)
                    ImGui::DockBuilderDockWindow("Outliner", dockIdLeftTop);  // Left top
                    ImGui::DockBuilderDockWindow("Details", dockIdLeftMiddle);  // Left middle
                    ImGui::DockBuilderDockWindow("Content Browser", dockIdLeftBottom);  // Left bottom
                    ImGui::DockBuilderDockWindow("Scene Viewport", dockMain);  // Center
                    ImGui::DockBuilderDockWindow("Profiler", dockIdLeftBottom);  // Dock with Content Browser (can be split)
                    ImGui::DockBuilderDockWindow("Console", dockIdLeftBottom);  // Dock with Content Browser (can be split)
                    
                    ImGui::DockBuilderFinish(dockspace_id);
                    LGE::Log::Info("Dock layout setup complete");
                } else {
                    // Layout exists from ini file, just mark as setup
                    layoutSetup = true;
                    LGE::Log::Info("Using saved dock layout from ini file");
                }
            }
            
            ImGui::End(); // End dock space window
        } else {
            // Project browser is showing, don't render dock space
            ImGui::PopStyleVar(5);
        }
        
        // Render scene to framebuffer (everything should render here)
        // Only render if project is loaded and viewport is valid (and not showing project browser)
        if (!m_ShowProjectBrowser && m_Project && m_Project->IsLoaded() && m_SceneViewport && m_SceneViewport->GetWidth() > 0 && m_SceneViewport->GetHeight() > 0) {
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
            
            
            // Update lights from active world
            // CRITICAL: This must be called every frame to pick up inspector changes
            if (m_LightSystem && m_SceneManager && m_Camera) {
                auto activeWorld = m_SceneManager->GetActiveWorld();
                if (activeWorld) {
                    // BeginFrame clears and collects lights from the world
                    m_LightSystem->BeginFrame(*activeWorld);
                    // UploadToGPU sends the collected lights to the GPU buffer
                    // This ensures any changes made in the inspector are immediately visible
                    m_LightSystem->UploadToGPU();
                    
                    // Render shadow maps before main rendering (only if viewport is valid)
                    if (m_SceneViewport && m_SceneViewport->GetWidth() > 0 && m_SceneViewport->GetHeight() > 0) {
                        m_LightSystem->RenderShadowMaps(*activeWorld, m_Camera.get());
                    }
                }
            }
            
            // Render GameObjects
            RenderGameObjects();
            
            // Render light gizmos (editor-only visual helpers)
            RenderLightGizmos();
            
            // ImGuizmo is rendered in OnUIRender, not here
            
            // End rendering to framebuffer (restores viewport)
            m_SceneViewport->EndRender();
        }
    }
    
    void RenderGameObjects() {
        if (!m_SceneManager || !m_Camera) {
            return;
        }
        
        auto activeWorld = m_SceneManager->GetActiveWorld();
        if (!activeWorld) {
            return;
        }
        
        // Get view-projection matrix
        const auto& viewProj = m_Camera->GetViewProjectionMatrix();
        
        // Render all GameObjects
        auto allObjects = activeWorld->GetAllGameObjects();
        static int lastObjectCount = 0;
        if (allObjects.size() != lastObjectCount) {
            LGE::Log::Info("RenderGameObjects: Found " + std::to_string(allObjects.size()) + " GameObjects");
            lastObjectCount = static_cast<int>(allObjects.size());
        }
        
        int renderedCount = 0;
        for (const auto& obj : allObjects) {
            if (!obj || !obj->IsActive()) {
                continue;
            }
            
            // Get MeshRenderer component
            auto meshRenderer = obj->GetComponent<LGE::MeshRenderer>();
            if (!meshRenderer) {
                continue;
            }
            
            // Get mesh
            auto mesh = meshRenderer->GetMesh();
            if (!mesh) {
                LGE::Log::Warn("RenderGameObjects: GameObject \"" + obj->GetName() + "\" has MeshRenderer but no mesh");
                continue;
            }
            
            // Get material (use default if none assigned)
            auto material = meshRenderer->GetMaterial(0);
            if (!material) {
                material = m_LitMaterial;
            }
            
            if (!material || !material->GetShader()) {
                LGE::Log::Warn("RenderGameObjects: GameObject \"" + obj->GetName() + "\" has no valid material/shader");
                continue;
            }
            
            // Get transform
            auto* transform = obj->GetTransform();
            if (!transform) {
                continue;
            }
            
            // Calculate model matrix from transform
            LGE::Math::Matrix4 modelMatrix = transform->GetWorldMatrix();
            
            // Bind material (this binds the shader and sets uniforms)
            material->Bind();
            
            // IMPORTANT: Bind lighting buffers BEFORE setting uniforms
            // This ensures the SSBO is available when the shader reads from it
            // The SSBO must be bound to binding point 3 before the shader uses it
            if (m_LightSystem) {
                // Ensure SSBO is bound to the correct binding point
                m_LightSystem->BindLightingBuffers();
                
                // Set light count AFTER binding buffers
                int lightCount = static_cast<int>(m_LightSystem->GetLightBuffer().size());
                material->GetShader()->SetUniform1i("u_LightCount", lightCount);
            }
            
            // Set view-projection and model matrices
            material->GetShader()->SetUniformMat4("u_ViewProjection", viewProj.m);
            material->GetShader()->SetUniformMat4("u_Model", modelMatrix.m);
            
            // Set view position for lighting calculations
            LGE::Math::Vector3 viewPos = m_Camera->GetPosition();
            material->GetShader()->SetUniform3f("u_ViewPos", viewPos.x, viewPos.y, viewPos.z);
            
            // Set shadow map (if available)
            if (m_LightSystem) {
                
                // Set shadow map and light view-projection
                auto* shadow = m_LightSystem->GetDirectionalShadow();
                if (shadow && shadow->IsValid) {
                    material->GetShader()->SetUniform1i("u_HasDirectionalShadow", 1);
                    material->GetShader()->SetUniformMat4("u_LightViewProj", shadow->LightViewProj.m);
                    
                    // Bind shadow map texture
                    glActiveTexture(GL_TEXTURE0 + 4); // Use texture slot 4 for shadow map
                    glBindTexture(GL_TEXTURE_2D, shadow->ShadowMapTextureID);
                    material->GetShader()->SetTexture("u_DirectionalShadowMap", shadow->ShadowMapTextureID, 4);
                } else {
                    material->GetShader()->SetUniform1i("u_HasDirectionalShadow", 0);
                }
            }
            
            // Set use vertex color to 1.0 to use vertex colors from the mesh
            material->GetShader()->SetUniform1f("u_UseVertexColor", 1.0f);
            
            // Bind vertex array
            auto vertexArray = mesh->GetVertexArray();
            if (vertexArray) {
                vertexArray->Bind();
                
                // Draw mesh
                auto indexBuffer = mesh->GetIndexBuffer();
                uint32_t vertexCount = mesh->GetVertexCount();
                uint32_t indexCount = mesh->GetIndexCount();
                
                if (indexBuffer && indexCount > 0) {
                    // Indexed drawing
                    indexBuffer->Bind();
                    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
                    indexBuffer->Unbind();
                    renderedCount++;
                } else if (vertexCount > 0) {
                    // Non-indexed drawing
                    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
                    renderedCount++;
                } else {
                    LGE::Log::Warn("RenderGameObjects: Mesh \"" + mesh->GetName() + "\" has no vertices or indices");
                }
                
                vertexArray->Unbind();
            } else {
                LGE::Log::Warn("RenderGameObjects: Mesh \"" + mesh->GetName() + "\" has no vertex array");
            }
            
            // Unbind material
            material->Unbind();
        }
        
        static int lastRenderedCount = 0;
        if (renderedCount != lastRenderedCount) {
            LGE::Log::Info("RenderGameObjects: Rendered " + std::to_string(renderedCount) + " meshes");
            lastRenderedCount = renderedCount;
        }
    }

    // Helper function to transform a point by a matrix
    LGE::Math::Vector3 TransformPoint(const LGE::Math::Matrix4& matrix, const LGE::Math::Vector3& point) {
        LGE::Math::Vector4 result = matrix * LGE::Math::Vector4(point.x, point.y, point.z, 1.0f);
        return LGE::Math::Vector3(result.x, result.y, result.z);
    }
    
    void RenderLightGizmos() {
        if (!m_SceneManager || !m_Camera) return;
        
        auto activeWorld = m_SceneManager->GetActiveWorld();
        if (!activeWorld) return;
        
        auto allObjects = activeWorld->GetAllGameObjects();
        
        // Load gizmo shader if not already loaded
        static std::shared_ptr<LGE::Shader> gizmoShader = nullptr;
        if (!gizmoShader) {
            std::string vertSource = LGE::FileSystem::ReadFile("assets/shaders/Gizmo.vert");
            std::string fragSource = LGE::FileSystem::ReadFile("assets/shaders/Gizmo.frag");
            
            if (!vertSource.empty() && !fragSource.empty()) {
                gizmoShader = std::make_shared<LGE::Shader>(vertSource, fragSource);
                if (gizmoShader->GetRendererID() == 0) {
                    LGE::Log::Warn("Failed to load gizmo shader, gizmos will not be visible");
                    gizmoShader = nullptr;
                    return;
                }
            } else {
                LGE::Log::Warn("Gizmo shader files not found, gizmos will not be visible");
                return;
            }
        }
        
        // Handle mouse input for gizmo handles
        HandleGizmoInput(allObjects);
        
        // Save OpenGL state
        GLboolean depthTestEnabled;
        glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
        GLboolean cullFaceEnabled;
        glGetBooleanv(GL_CULL_FACE, &cullFaceEnabled);
        GLboolean blendEnabled;
        glGetBooleanv(GL_BLEND, &blendEnabled);
        GLint currentPolygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, currentPolygonMode);
        GLint currentProgram;
        glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
        GLint currentVAO;
        glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &currentVAO);
        
        // Set up OpenGL state for gizmo rendering
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDisable(GL_BLEND);
        glLineWidth(2.0f); // Make gizmo lines more visible
        
        gizmoShader->Bind();
        const LGE::Math::Matrix4& viewProj = m_Camera->GetViewProjectionMatrix();
        gizmoShader->SetUniformMat4("u_ViewProjection", viewProj.m);
        
        // Render gizmos for each light
        for (const auto& obj : allObjects) {
            if (!obj || !obj->IsActive()) continue;
            
            auto light = obj->GetComponent<LGE::LightComponent>();
            if (!light) continue;
            
            auto* transform = obj->GetTransform();
            if (!transform) continue;
            
            LGE::Math::Vector3 position = transform->GetWorldPosition();
            LGE::Math::Vector3 forward = transform->Forward();
            LGE::Math::Vector3 color = light->Color;
            
            // Set color uniform
            gizmoShader->SetUniform3f("u_Color", color.x, color.y, color.z);
            
            if (light->IsDirectional()) {
                // Directional light: Draw a line showing direction
                RenderDirectionalLightGizmo(position, forward, gizmoShader);
            }
            else if (light->IsPoint()) {
                // Point light: Draw 3-ring wireframe sphere with handle
                RenderPointLightGizmo(obj, position, light->Range, color, gizmoShader);
            }
            else if (light->IsSpot()) {
                // Spot light: Draw wireframe cone with inner cone and handles
                RenderSpotLightGizmo(obj, transform, light->Range, light->InnerAngle, light->OuterAngle, color, gizmoShader);
            }
        }
        
        gizmoShader->Unbind();
        
        // Restore OpenGL state
        glLineWidth(1.0f);
        if (!depthTestEnabled) {
            glDisable(GL_DEPTH_TEST);
        }
        if (cullFaceEnabled) {
            glEnable(GL_CULL_FACE);
        }
        if (blendEnabled) {
            glEnable(GL_BLEND);
        }
        
        // Restore VAO and shader program
        glBindVertexArray(currentVAO);
        glUseProgram(currentProgram);
    }
    
    void RenderDirectionalLightGizmo(const LGE::Math::Vector3& position, const LGE::Math::Vector3& forward, std::shared_ptr<LGE::Shader> shader) {
        // Create a simple line from position to position + forward * 5
        struct GizmoVertex {
            LGE::Math::Vector3 position;
        };
        
        std::vector<GizmoVertex> vertices = {
            { position },
            { position + forward * 5.0f }
        };
        
        // Create temporary VBO and VAO
        auto vbo = std::make_unique<LGE::VertexBuffer>(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(GizmoVertex)));
        auto vao = std::make_unique<LGE::VertexArray>();
        
        vao->Bind();
        vbo->Bind();
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void*)0);
        
        LGE::Math::Matrix4 modelMatrix = LGE::Math::Matrix4::Identity();
        shader->SetUniformMat4("u_Model", modelMatrix.m);
        
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
        
        vao->Unbind();
        vbo->Unbind();
    }
    
    
    void RenderPointLightGizmo(std::shared_ptr<LGE::GameObject> obj, const LGE::Math::Vector3& position, float range, const LGE::Math::Vector3& color, std::shared_ptr<LGE::Shader> shader) {
        struct GizmoVertex {
            LGE::Math::Vector3 position;
        };
        
        const int segments = 32;
        const float PI = 3.14159265359f;
        std::vector<GizmoVertex> vertices;
        
        // XY ring (circle in XY plane)
        for (int i = 0; i < segments; i++) {
            float a0 = (i / (float)segments) * 2.0f * PI;
            float a1 = ((i + 1) / (float)segments) * 2.0f * PI;
            
            vertices.push_back({ position + LGE::Math::Vector3(range * std::cos(a0), range * std::sin(a0), 0.0f) });
            vertices.push_back({ position + LGE::Math::Vector3(range * std::cos(a1), range * std::sin(a1), 0.0f) });
        }
        
        // XZ ring (circle in XZ plane)
        for (int i = 0; i < segments; i++) {
            float a0 = (i / (float)segments) * 2.0f * PI;
            float a1 = ((i + 1) / (float)segments) * 2.0f * PI;
            
            vertices.push_back({ position + LGE::Math::Vector3(range * std::cos(a0), 0.0f, range * std::sin(a0)) });
            vertices.push_back({ position + LGE::Math::Vector3(range * std::cos(a1), 0.0f, range * std::sin(a1)) });
        }
        
        // YZ ring (circle in YZ plane)
        for (int i = 0; i < segments; i++) {
            float a0 = (i / (float)segments) * 2.0f * PI;
            float a1 = ((i + 1) / (float)segments) * 2.0f * PI;
            
            vertices.push_back({ position + LGE::Math::Vector3(0.0f, range * std::cos(a0), range * std::sin(a0)) });
            vertices.push_back({ position + LGE::Math::Vector3(0.0f, range * std::cos(a1), range * std::sin(a1)) });
        }
        
        // Create temporary VBO and VAO
        auto vbo = std::make_unique<LGE::VertexBuffer>(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(GizmoVertex)));
        auto vao = std::make_unique<LGE::VertexArray>();
        
        vao->Bind();
        vbo->Bind();
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void*)0);
        
        LGE::Math::Matrix4 modelMatrix = LGE::Math::Matrix4::Identity();
        shader->SetUniformMat4("u_Model", modelMatrix.m);
        
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
        
        vao->Unbind();
        vbo->Unbind();
        
        // Render handle for range adjustment (on one of the rings, e.g., positive X axis)
        LGE::Math::Vector3 handlePos = position + LGE::Math::Vector3(range, 0.0f, 0.0f);
        LGE::Math::Vector3 handleColor = (m_GizmoHandleState.isDragging && m_GizmoHandleState.lightObject == obj && m_GizmoHandleState.handleType == 0) 
            ? LGE::Math::Vector3(1.0f, 1.0f, 0.0f) : color; // Yellow when selected
        RenderGizmoHandle(handlePos, 0.15f, handleColor, shader);
    }
    
    void RenderSpotLightGizmo(std::shared_ptr<LGE::GameObject> obj, LGE::Transform* transform, float range, float innerAngle, float outerAngle, const LGE::Math::Vector3& color, std::shared_ptr<LGE::Shader> shader) {
        struct GizmoVertex {
            LGE::Math::Vector3 position;
        };
        
        const int segments = 32;
        const float PI = 3.14159265359f;
        
        LGE::Math::Vector3 origin = transform->GetWorldPosition();
        LGE::Math::Vector3 forward = transform->Forward();
        LGE::Math::Vector3 right = transform->Right();
        LGE::Math::Vector3 up = transform->Up();
        
        float height = range;
        float outerRadius = range * std::tan(outerAngle);
        float innerRadius = range * std::tan(innerAngle);
        
        LGE::Math::Vector3 coneTip = origin;
        LGE::Math::Vector3 baseCenter = origin + forward * height;
        
        std::vector<GizmoVertex> vertices;
        
        // Draw outer cone outline: lines from tip to base circle
        LGE::Math::Vector3 prevBasePoint;
        bool firstPoint = true;
        
        for (int i = 0; i <= segments; i++) {
            float angle = (i / (float)segments) * 2.0f * PI;
            
            LGE::Math::Vector3 localOffset = right * (outerRadius * std::cos(angle)) + up * (outerRadius * std::sin(angle));
            LGE::Math::Vector3 worldPoint = baseCenter + localOffset;
            
            // Draw line from tip to base point
            vertices.push_back({ coneTip });
            vertices.push_back({ worldPoint });
            
            // Draw ring line (connect to previous point to form base circle)
            if (!firstPoint) {
                vertices.push_back({ prevBasePoint });
                vertices.push_back({ worldPoint });
            }
            
            prevBasePoint = worldPoint;
            firstPoint = false;
        }
        
        // Draw inner cone outline (lighter/different color to distinguish)
        LGE::Math::Vector3 prevInnerPoint;
        firstPoint = true;
        
        for (int i = 0; i <= segments; i++) {
            float angle = (i / (float)segments) * 2.0f * PI;
            
            LGE::Math::Vector3 localOffset = right * (innerRadius * std::cos(angle)) + up * (innerRadius * std::sin(angle));
            LGE::Math::Vector3 worldPoint = baseCenter + localOffset;
            
            // Draw line from tip to inner base point
            vertices.push_back({ coneTip });
            vertices.push_back({ worldPoint });
            
            // Draw inner ring line
            if (!firstPoint) {
                vertices.push_back({ prevInnerPoint });
                vertices.push_back({ worldPoint });
            }
            
            prevInnerPoint = worldPoint;
            firstPoint = false;
        }
        
        // Create temporary VBO and VAO
        auto vbo = std::make_unique<LGE::VertexBuffer>(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(GizmoVertex)));
        auto vao = std::make_unique<LGE::VertexArray>();
        
        vao->Bind();
        vbo->Bind();
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GizmoVertex), (void*)0);
        
        LGE::Math::Matrix4 modelMatrix = LGE::Math::Matrix4::Identity();
        shader->SetUniformMat4("u_Model", modelMatrix.m);
        
        glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(vertices.size()));
        
        vao->Unbind();
        vbo->Unbind();
        
        // Render handles for range and outer angle adjustment
        // Range handle: at the center of the base circle
        LGE::Math::Vector3 rangeHandlePos = baseCenter;
        LGE::Math::Vector3 rangeHandleColor = (m_GizmoHandleState.isDragging && m_GizmoHandleState.lightObject == obj && m_GizmoHandleState.handleType == 1) 
            ? LGE::Math::Vector3(1.0f, 1.0f, 0.0f) : LGE::Math::Vector3(1.0f, 1.0f, 0.0f); // Always yellow
        RenderGizmoHandleSquare(rangeHandlePos, 0.3f, rangeHandleColor, forward, right, up, shader);
        
        // Multiple outer angle handles: evenly distributed around the base circle perimeter
        const int numHandles = 5; // 5 handles as shown in the image
        for (int i = 0; i < numHandles; i++) {
            float angle = (i / (float)numHandles) * 2.0f * PI;
            LGE::Math::Vector3 localOffset = right * (outerRadius * std::cos(angle)) + up * (outerRadius * std::sin(angle));
            LGE::Math::Vector3 handlePos = baseCenter + localOffset;
            
            bool isSelected = (m_GizmoHandleState.isDragging && m_GizmoHandleState.lightObject == obj && 
                              m_GizmoHandleState.handleType == 2 && i == m_GizmoHandleState.handleIndex);
            LGE::Math::Vector3 handleColor = isSelected 
                ? LGE::Math::Vector3(1.0f, 1.0f, 0.0f) : LGE::Math::Vector3(1.0f, 1.0f, 0.0f); // Always yellow
            
            // Calculate handle orientation (face the camera - billboard style)
            // For simplicity, use the light's right and up vectors
            RenderGizmoHandleSquare(handlePos, 0.3f, handleColor, forward, right, up, shader);
        }
    }

    void RenderGizmoHandleSquare(const LGE::Math::Vector3& position, float size, const LGE::Math::Vector3& color, 
                                  const LGE::Math::Vector3& forward, const LGE::Math::Vector3& right, const LGE::Math::Vector3& up, 
                                  std::shared_ptr<LGE::Shader> shader) {
        // Create a square quad for the handle
        struct SquareVertex {
            LGE::Math::Vector3 position;
        };
        
        // Create a square in the plane defined by right and up vectors
        float halfSize = size * 0.5f;
        std::vector<SquareVertex> vertices = {
            { position + right * (-halfSize) + up * (-halfSize) },
            { position + right * (halfSize) + up * (-halfSize) },
            { position + right * (halfSize) + up * (halfSize) },
            { position + right * (-halfSize) + up * (halfSize) }
        };
        
        // Create indices for two triangles
        std::vector<uint32_t> indices = {
            0, 1, 2,
            0, 2, 3
        };
        
        // Create temporary VBO, IBO, and VAO
        auto vbo = std::make_unique<LGE::VertexBuffer>(vertices.data(), static_cast<uint32_t>(vertices.size() * sizeof(SquareVertex)));
        auto ibo = std::make_unique<LGE::IndexBuffer>(indices.data(), static_cast<uint32_t>(indices.size()));
        auto vao = std::make_unique<LGE::VertexArray>();
        
        vao->Bind();
        vbo->Bind();
        ibo->Bind();
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SquareVertex), (void*)0);
        
        LGE::Math::Matrix4 modelMatrix = LGE::Math::Matrix4::Identity();
        shader->SetUniformMat4("u_Model", modelMatrix.m);
        shader->SetUniform3f("u_Color", color.x, color.y, color.z);
        
        // Fill the square (not wireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, nullptr);
        
        vao->Unbind();
        vbo->Unbind();
        ibo->Unbind();
    }
    
    void RenderGizmoHandle(const LGE::Math::Vector3& position, float size, const LGE::Math::Vector3& color, std::shared_ptr<LGE::Shader> shader) {
        // Create a small sphere mesh for the handle
        // Use a simple approach: create a small sphere using PrimitiveMesh
        static std::shared_ptr<LGE::Mesh> handleMesh = nullptr;
        if (!handleMesh) {
            handleMesh = LGE::PrimitiveMesh::CreateSphere();
        }
        
        if (!handleMesh) return;
        
        // Scale the sphere to the desired size
        LGE::Math::Matrix4 modelMatrix = LGE::Math::Matrix4::Identity();
        modelMatrix = modelMatrix * LGE::Math::Matrix4::Scale(LGE::Math::Vector3(size, size, size));
        modelMatrix = modelMatrix * LGE::Math::Matrix4::Translate(LGE::Math::Vector3(position.x, position.y, position.z));
        
        shader->SetUniformMat4("u_Model", modelMatrix.m);
        shader->SetUniform3f("u_Color", color.x, color.y, color.z);
        
        // Render the sphere
        auto vertexArray = handleMesh->GetVertexArray();
        if (vertexArray) {
            vertexArray->Bind();
            auto indexBuffer = handleMesh->GetIndexBuffer();
            if (indexBuffer) {
                indexBuffer->Bind();
                glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(handleMesh->GetIndexCount()), GL_UNSIGNED_INT, nullptr);
                indexBuffer->Unbind();
            }
            vertexArray->Unbind();
        }
    }
    
    bool IsHandleClicked(const LGE::Math::Vector3& handleWorldPos, float handleSize) {
        if (!m_Camera || !m_SceneViewport) return false;
        
        // Get mouse position
        double mouseX, mouseY;
        LGE::Input::GetMousePosition(mouseX, mouseY);
        
        // Convert handle world position to screen space
        LGE::Math::Vector4 clipPos = m_Camera->GetViewProjectionMatrix() * LGE::Math::Vector4(handleWorldPos.x, handleWorldPos.y, handleWorldPos.z, 1.0f);
        
        if (clipPos.w <= 0.0f) return false; // Behind camera
        
        // Normalize device coordinates
        float ndcX = clipPos.x / clipPos.w;
        float ndcY = clipPos.y / clipPos.w;
        
        // Convert to screen coordinates (viewport space)
        float screenX = (ndcX + 1.0f) * 0.5f * m_SceneViewport->GetWidth();
        float screenY = (1.0f - ndcY) * 0.5f * m_SceneViewport->GetHeight();
        
        // Calculate distance from mouse to handle in screen space
        float dx = static_cast<float>(mouseX) - screenX;
        float dy = static_cast<float>(mouseY) - screenY;
        float distance = std::sqrt(dx * dx + dy * dy);
        
        // Check if within handle size (convert world size to screen size)
        // Use a larger threshold for easier clicking (handles are now 0.3f in world space)
        float threshold = 25.0f; // pixels - increased for larger handles
        return distance < threshold;
    }
    
    void HandleGizmoInput(const std::vector<std::shared_ptr<LGE::GameObject>>& allObjects) {
        if (!m_Camera || !m_SceneViewport) return;
        
        bool leftMousePressed = LGE::Input::IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT);
        static bool leftMouseWasPressed = false;
        
        // Check if viewport is focused and hovered
        if (!m_SceneViewport->IsFocused() || !m_SceneViewport->IsHovered()) {
            if (!leftMousePressed) {
                m_GizmoHandleState.isDragging = false;
            }
            leftMouseWasPressed = leftMousePressed;
            return;
        }
        
        if (leftMousePressed && !leftMouseWasPressed) {
            // Mouse just clicked - check for handle clicks
            m_GizmoHandleState.isDragging = false;
            
            for (const auto& obj : allObjects) {
                if (!obj || !obj->IsActive()) continue;
                
                auto light = obj->GetComponent<LGE::LightComponent>();
                if (!light) continue;
                
                auto* transform = obj->GetTransform();
                if (!transform) continue;
                
                LGE::Math::Vector3 position = transform->GetWorldPosition();
                
                if (light->IsPoint()) {
                    // Check if range handle was clicked
                    LGE::Math::Vector3 handlePos = position + LGE::Math::Vector3(light->Range, 0.0f, 0.0f);
                    if (IsHandleClicked(handlePos, 0.3f)) {
                        m_GizmoHandleState.lightObject = obj;
                        m_GizmoHandleState.handleType = 0; // Point light range
                        m_GizmoHandleState.handleWorldPos = handlePos;
                        m_GizmoHandleState.initialValue = light->Range;
                        m_GizmoHandleState.isDragging = true;
                        break;
                    }
                }
                else if (light->IsSpot()) {
                    LGE::Math::Vector3 forward = transform->Forward();
                    LGE::Math::Vector3 right = transform->Right();
                    LGE::Math::Vector3 up = transform->Up();
                    float range = light->Range;
                    float outerRadius = range * std::tan(light->OuterAngle);
                    const float PI = 3.14159265359f;
                    const int numHandles = 5;
                    
                    LGE::Math::Vector3 baseCenter = position + forward * range;
                    
                    // Check if center handle was clicked
                    if (IsHandleClicked(baseCenter, 0.3f)) {
                        m_GizmoHandleState.lightObject = obj;
                        m_GizmoHandleState.handleType = 1; // Spot light range
                        m_GizmoHandleState.handleIndex = -1; // Center handle
                        m_GizmoHandleState.handleWorldPos = baseCenter;
                        m_GizmoHandleState.initialValue = range;
                        m_GizmoHandleState.isDragging = true;
                        break;
                    }
                    
                    // Check if any perimeter handle was clicked
                    for (int i = 0; i < numHandles; i++) {
                        float angle = (i / (float)numHandles) * 2.0f * PI;
                        LGE::Math::Vector3 localOffset = right * (outerRadius * std::cos(angle)) + up * (outerRadius * std::sin(angle));
                        LGE::Math::Vector3 handlePos = baseCenter + localOffset;
                        
                        if (IsHandleClicked(handlePos, 0.3f)) {
                            m_GizmoHandleState.lightObject = obj;
                            m_GizmoHandleState.handleType = 2; // Spot light outer angle
                            m_GizmoHandleState.handleIndex = i; // Which perimeter handle
                            m_GizmoHandleState.handleWorldPos = handlePos;
                            m_GizmoHandleState.initialValue = light->OuterAngle;
                            m_GizmoHandleState.isDragging = true;
                            break;
                        }
                    }
                    
                    if (m_GizmoHandleState.isDragging) {
                        break;
                    }
                }
            }
        }
        else if (leftMousePressed && leftMouseWasPressed && m_GizmoHandleState.isDragging && m_GizmoHandleState.lightObject) {
            // Mouse is being dragged - adjust light property
            auto light = m_GizmoHandleState.lightObject->GetComponent<LGE::LightComponent>();
            auto* transform = m_GizmoHandleState.lightObject->GetTransform();
            
            if (light && transform) {
                // Get mouse movement
                double mouseX, mouseY;
                LGE::Input::GetMousePosition(mouseX, mouseY);
                
                // Get camera position and direction for movement calculation
                LGE::Math::Vector3 cameraPos = m_Camera->GetPosition();
                LGE::Math::Vector3 cameraTarget = m_Camera->GetTarget();
                LGE::Math::Vector3 cameraDir = LGE::Math::Vector3(
                    cameraTarget.x - cameraPos.x,
                    cameraTarget.y - cameraPos.y,
                    cameraTarget.z - cameraPos.z
                );
                
                // Normalize camera direction
                float camDirLen = std::sqrt(cameraDir.x * cameraDir.x + cameraDir.y * cameraDir.y + cameraDir.z * cameraDir.z);
                if (camDirLen > 0.001f) {
                    cameraDir = LGE::Math::Vector3(cameraDir.x / camDirLen, cameraDir.y / camDirLen, cameraDir.z / camDirLen);
                }
                
                // Calculate camera right vector
                LGE::Math::Vector3 cameraUp = m_Camera->GetUp();
                LGE::Math::Vector3 cameraRight = LGE::Math::Vector3(
                    cameraDir.y * cameraUp.z - cameraDir.z * cameraUp.y,
                    cameraDir.z * cameraUp.x - cameraDir.x * cameraUp.z,
                    cameraDir.x * cameraUp.y - cameraDir.y * cameraUp.x
                );
                
                // Normalize right vector
                float rightLen = std::sqrt(cameraRight.x * cameraRight.x + cameraRight.y * cameraRight.y + cameraRight.z * cameraRight.z);
                if (rightLen > 0.001f) {
                    cameraRight = LGE::Math::Vector3(cameraRight.x / rightLen, cameraRight.y / rightLen, cameraRight.z / rightLen);
                }
                
                // Project handle position to screen space
                LGE::Math::Vector4 clipPos = m_Camera->GetViewProjectionMatrix() * LGE::Math::Vector4(
                    m_GizmoHandleState.handleWorldPos.x, 
                    m_GizmoHandleState.handleWorldPos.y, 
                    m_GizmoHandleState.handleWorldPos.z, 
                    1.0f
                );
                
                if (clipPos.w > 0.0f) {
                    float ndcX = clipPos.x / clipPos.w;
                    float ndcY = clipPos.y / clipPos.w;
                    float screenX = (ndcX + 1.0f) * 0.5f * m_SceneViewport->GetWidth();
                    float screenY = (1.0f - ndcY) * 0.5f * m_SceneViewport->GetHeight();
                    
                    // Calculate mouse delta
                    float deltaX = static_cast<float>(mouseX) - screenX;
                    float deltaY = static_cast<float>(mouseY) - screenY;
                    
                    // Calculate movement in world space (approximate)
                    // Use camera's forward/right vectors to determine movement direction
                    LGE::Math::Vector3 lightPos = transform->GetWorldPosition();
                    LGE::Math::Vector3 toHandle = m_GizmoHandleState.handleWorldPos - lightPos;
                    float handleDistance = std::sqrt(toHandle.x * toHandle.x + toHandle.y * toHandle.y + toHandle.z * toHandle.z);
                    
                    if (handleDistance > 0.001f) {
                        LGE::Math::Vector3 handleDir = LGE::Math::Vector3(toHandle.x / handleDistance, toHandle.y / handleDistance, toHandle.z / handleDistance);
                        
                        // Project mouse movement onto handle direction
                        float movement = (deltaX * 0.01f + deltaY * 0.01f); // Scale factor
                        
                        if (m_GizmoHandleState.handleType == 0) {
                            // Point light range
                            float newRange = m_GizmoHandleState.initialValue + movement;
                            if (newRange > 0.1f && newRange < 1000.0f) {
                                light->Range = newRange;
                            }
                        }
                        else if (m_GizmoHandleState.handleType == 1) {
                            // Spot light range
                            float newRange = m_GizmoHandleState.initialValue + movement;
                            if (newRange > 0.1f && newRange < 1000.0f) {
                                light->Range = newRange;
                            }
                        }
                        else if (m_GizmoHandleState.handleType == 2) {
                            // Spot light outer angle
                            float newAngle = m_GizmoHandleState.initialValue + movement * 0.1f;
                            if (newAngle > 0.01f && newAngle < 3.14159f / 2.0f) {
                                light->OuterAngle = newAngle;
                            }
                        }
                    }
                }
            }
        }
        else if (!leftMousePressed && leftMouseWasPressed) {
            // Mouse released - stop dragging
            m_GizmoHandleState.isDragging = false;
        }
        
        leftMouseWasPressed = leftMousePressed;
    }

    void Shutdown() override {
        
        m_SceneManager.reset();
        m_ContentBrowser.reset();
        m_Inspector.reset();
        m_Hierarchy.reset();
        m_Profiler.reset();
        m_Preferences.reset();
        m_ProjectSettings.reset();
        m_TextureImporter.reset();
        m_TextureManager.reset();
        m_Toolbar.reset();
        m_MainMenuBar.reset();
        m_SceneViewport.reset();
        m_LightSystem.reset();
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
    std::unique_ptr<LGE::Camera> m_Camera;
    std::unique_ptr<LGE::CameraController> m_CameraController;
    std::unique_ptr<LGE::Skybox> m_Skybox;
    std::unique_ptr<LGE::SceneViewport> m_SceneViewport;
    std::unique_ptr<LGE::Details> m_Details;
    std::unique_ptr<LGE::Hierarchy> m_Hierarchy;
    std::unique_ptr<LGE::Inspector> m_Inspector;
    std::unique_ptr<LGE::SceneManager> m_SceneManager;
    std::unique_ptr<LGE::ContentBrowser> m_ContentBrowser;
    std::unique_ptr<LGE::Profiler> m_Profiler;
    std::unique_ptr<LGE::Console> m_Console;
    std::unique_ptr<LGE::Preferences> m_Preferences;
    std::unique_ptr<LGE::ProjectSettings> m_ProjectSettings;
    std::unique_ptr<LGE::TextureImporter> m_TextureImporter;
    std::unique_ptr<LGE::TextureManager> m_TextureManager;
    std::unique_ptr<LGE::Toolbar> m_Toolbar;
    std::unique_ptr<LGE::MainMenuBar> m_MainMenuBar;
    std::unique_ptr<LGE::ProjectBrowser> m_ProjectBrowser;
    std::shared_ptr<LGE::Project> m_Project;
    std::unique_ptr<LGE::FileSystemManager> m_FileSystemManager;
    std::unique_ptr<LGE::LightSystem> m_LightSystem;
    bool m_ShowProjectBrowser;
    
    // Animation time
    float m_Time = 0.0f;
    
    // Gizmo handle dragging state
    struct GizmoHandleState {
        std::shared_ptr<LGE::GameObject> lightObject;
        int handleType; // 0 = point light range, 1 = spot light range, 2 = spot light outer angle
        int handleIndex; // For spot light: which perimeter handle (0-4) or -1 for center
        LGE::Math::Vector3 handleWorldPos;
        float initialValue;
        bool isDragging;
    };
    GizmoHandleState m_GizmoHandleState;
    
    // PBR test balls
    std::unique_ptr<LGE::VertexBuffer> m_SphereVertexBuffer;
    std::unique_ptr<LGE::VertexArray> m_SphereVertexArray;
    std::unique_ptr<LGE::IndexBuffer> m_SphereIndexBuffer;
    uint32_t m_SphereIndexCount = 0;
    std::shared_ptr<LGE::Shader> m_PBRShader;
    
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
    try {
        ExampleApp app;

        if (!app.Initialize()) {
            LGE::Log::Error("Failed to initialize application!");
            // Keep console open on Windows to see error messages
            #ifdef _WIN32
            std::cout << "Press Enter to exit..." << std::endl;
            std::cin.get();
            #endif
            return -1;
        }

        app.Run();
        app.Shutdown();
    } catch (const std::exception& e) {
        LGE::Log::Fatal("Exception caught: " + std::string(e.what()));
        #ifdef _WIN32
        std::cout << "Exception: " << e.what() << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        #endif
        return -1;
    } catch (...) {
        LGE::Log::Fatal("Unknown exception caught!");
        #ifdef _WIN32
        std::cout << "Unknown exception caught!" << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();
        #endif
        return -1;
    }

    return 0;
}

