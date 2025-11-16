cmake_minimum_required(VERSION 3.20)
project(LGE VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
set(LGE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(THIRD_PARTY_DIR ${CMAKE_SOURCE_DIR}/third_party)

# GLAD library (use pre-built)
add_library(glad STATIC IMPORTED)
set_target_properties(glad PROPERTIES
    IMPORTED_LOCATION ${THIRD_PARTY_DIR}/glad/lib/glad.lib
    INTERFACE_INCLUDE_DIRECTORIES ${THIRD_PARTY_DIR}/glad/include
)
set(GLAD_TARGET glad)

# GLFW library (link directly from third_party)
add_library(glfw STATIC IMPORTED)
set_target_properties(glfw PROPERTIES
    IMPORTED_LOCATION ${THIRD_PARTY_DIR}/glfw/lib/glfw3dll.lib
    INTERFACE_INCLUDE_DIRECTORIES ${THIRD_PARTY_DIR}/glfw/include
)

# Source files
set(CORE_SOURCES
    src/core/Application.cpp
    src/core/Window.cpp
    src/core/Log.cpp
    src/core/Input.cpp
    src/core/GUID.cpp
    src/core/GUIDRegistry.cpp
    
    # Assets
    src/core/assets/AssetMetadata.cpp
    src/core/assets/AssetRegistry.cpp
    src/core/assets/AssetScanner.cpp
    src/core/assets/AssetHotReloader.cpp
    src/core/assets/ImporterFactory.cpp
    src/core/assets/MetaFile.cpp
    src/core/assets/ThumbnailGenerator.cpp
    src/core/assets/DependencyGraph.cpp
    src/core/assets/DependencyScanner.cpp
    src/core/assets/AssetDeletionValidator.cpp
    src/core/assets/AssetDeleter.cpp
    src/core/assets/AssetReferenceFinder.cpp
    src/core/assets/AssetSearchIndex.cpp
    src/core/assets/AssetFilter.cpp
    src/core/assets/SavedSearch.cpp
    src/core/assets/AssetCollection.cpp
    src/core/assets/AssetPackager.cpp
    src/core/assets/AssetLoader.cpp
    src/core/assets/ResourceManager.cpp
    src/core/assets/AsyncAssetLoader.cpp
    src/core/assets/AssetRegistryCache.cpp
    src/core/assets/AssetMetadataPool.cpp
    src/core/assets/AssetValidator.cpp
    src/core/assets/DependencyGraph.cpp
    src/core/assets/DependencyScanner.cpp
    src/core/assets/SavedSearch.cpp
    src/core/importers/TextureImporter.cpp
    
    # FileSystem
    src/core/filesystem/FileSystem.cpp
    src/core/filesystem/VirtualFileSystem.cpp
    src/core/filesystem/FileSystemWatcher.cpp
    src/core/filesystem/FileChangeDebouncer.cpp
    src/core/filesystem/FileSystemManager.cpp
    
    # Project
    src/core/project/Project.cpp
    src/core/project/ProjectDescriptor.cpp
    src/core/project/ProjectManager.cpp
    
    # UI (related to core systems)
    src/ui/ContentBrowserModel.cpp
    src/ui/ReferenceViewerWindow.cpp
    src/ui/ValidationReportWindow.cpp
)

# Platform-specific file watcher implementations
if(WIN32)
    list(APPEND CORE_SOURCES
        src/core/filesystem/FileSystemWatcher_Windows.cpp
    )
elseif(UNIX AND NOT APPLE)
    list(APPEND CORE_SOURCES
        src/core/filesystem/FileSystemWatcher_Linux.cpp
    )
elseif(APPLE)
    list(APPEND CORE_SOURCES
        src/core/filesystem/FileSystemWatcher_macOS.cpp
    )
endif()

set(CORE_SOURCES ${CORE_SOURCES}
    # Core systems
    src/core/Layer.cpp
    src/core/LayerStack.cpp
    src/core/SplashScreen.cpp
    # Scene/ECS
    src/core/scene/Component.cpp
    src/core/scene/ComponentFactory.cpp
    src/core/scene/GameObject.cpp
    src/core/scene/World.cpp
    src/core/scene/SceneManager.cpp
    src/core/scene/components/Transform.cpp
    src/core/scene/components/LightPropertiesComponent.cpp
    src/core/scene/components/LightComponent.cpp
    src/core/scene/components/SkyLightComponent.cpp
    src/core/scene/components/FogComponent.cpp
    src/core/scene/components/MeshRenderer.cpp
    src/core/scene/components/CameraComponent.cpp
    src/core/scene/components/Rigidbody.cpp
    src/core/scene/components/Collider.cpp
    src/core/scene/components/BoxCollider.cpp
    src/core/scene/components/SphereCollider.cpp
    src/core/scene/components/CapsuleCollider.cpp
    src/core/scene/components/ScriptComponent.cpp
    src/core/scene/components/PlayerController.cpp
)

set(RENDERING_SOURCES
    src/rendering/Renderer.cpp
    src/rendering/opengl/OpenGLRenderer.cpp
    src/rendering/Shader.cpp
    src/rendering/VertexBuffer.cpp
    src/rendering/VertexArray.cpp
    src/rendering/IndexBuffer.cpp
    src/rendering/Camera.cpp
    src/rendering/CameraController.cpp
    src/rendering/Skybox.cpp
    src/rendering/Texture.cpp
    src/rendering/TextureManager.cpp
    src/rendering/DirectionalLight.cpp
    src/rendering/Framebuffer.cpp
    src/rendering/Material.cpp
    src/rendering/GridRenderer.cpp
    src/rendering/Mesh.cpp
    src/rendering/PostProcessor.cpp
    src/rendering/ExposureSystem.cpp
    src/rendering/LightSystem.cpp
    src/rendering/LightingSettings.cpp
)

set(UI_SOURCES
    src/ui/UI.cpp
    src/ui/SceneViewport.cpp
    src/ui/Hierarchy.cpp
    src/ui/Inspector.cpp
    src/ui/HierarchyWindow.cpp
    src/ui/InspectorWindow.cpp
    src/ui/ContentBrowser.cpp
    src/ui/TextureImporter.cpp
    src/ui/MaterialEditor.cpp
    src/ui/Toolbar.cpp
    src/ui/MainMenuBar.cpp
    src/ui/Details.cpp
    src/ui/Profiler.cpp
    src/ui/Console.cpp
    src/ui/Preferences.cpp
    src/ui/ProjectSettings.cpp
    src/ui/ProjectBrowser.cpp
)

# ImGui sources
set(IMGUI_SOURCES
    third_party/imgui/imgui.cpp
    third_party/imgui/imgui_draw.cpp
    third_party/imgui/imgui_tables.cpp
    third_party/imgui/imgui_widgets.cpp
    third_party/imgui/backends/imgui_impl_glfw.cpp
    third_party/imgui/backends/imgui_impl_opengl3.cpp
)

# ImGuizmo sources
set(IMGUIZMO_SOURCES
    third_party/ImGuizmo/ImGuizmo.cpp
)

set(MATH_SOURCES
    src/math/Matrix.cpp
)

# Create LGE library
add_library(LGE STATIC
    ${CORE_SOURCES}
    ${RENDERING_SOURCES}
    ${MATH_SOURCES}
    ${UI_SOURCES}
    ${IMGUI_SOURCES}
    ${IMGUIZMO_SOURCES}
)

target_include_directories(LGE PUBLIC
    ${LGE_INCLUDE_DIR}
    ${THIRD_PARTY_DIR}/glfw/include
    ${THIRD_PARTY_DIR}/glad/include
    ${THIRD_PARTY_DIR}/imgui
    ${THIRD_PARTY_DIR}/imgui/backends
    ${THIRD_PARTY_DIR}/ImGuizmo
)

target_link_libraries(LGE PUBLIC
    ${GLAD_TARGET}
    glfw
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(LGE PUBLIC
        opengl32
    )
endif()

# Example application
add_executable(LGE_Example
    src/main.cpp
)

target_link_libraries(LGE_Example PRIVATE LGE)

# Copy DLLs and assets to output directory (Windows)
if(WIN32)
    add_custom_command(TARGET LGE_Example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${THIRD_PARTY_DIR}/glfw/lib/glfw3.dll
        $<TARGET_FILE_DIR:LGE_Example>
    )
    
    # Copy assets folder
    add_custom_command(TARGET LGE_Example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:LGE_Example>/assets
    )
    
    # Copy Project Browser icons
    add_custom_command(TARGET LGE_Example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
        $<TARGET_FILE_DIR:LGE_Example>/assets/icons
    )
    
    # Copy blank project thumbnail and preview
    add_custom_command(TARGET LGE_Example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/temp_icons/imgs/GameProjectDialog/blank_project_thumbnail.png
        $<TARGET_FILE_DIR:LGE_Example>/assets/icons/blank_project_thumbnail.png
    )
    
    add_custom_command(TARGET LGE_Example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/temp_icons/imgs/GameProjectDialog/blank_project_preview.png
        $<TARGET_FILE_DIR:LGE_Example>/assets/icons/blank_project_preview.png
    )
    
    # Copy folder icon if it exists
    add_custom_command(TARGET LGE_Example POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/temp_icons/imgs/Icons/Folders/Folder_Base_256x.png
        $<TARGET_FILE_DIR:LGE_Example>/assets/icons/Folder_Base_256x.png
        COMMENT "Copying folder icon"
    )
endif()

