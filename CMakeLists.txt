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
    src/core/FileSystem.cpp
    src/core/Input.cpp
    src/core/GameObject.cpp
    src/core/components/LightPropertiesComponent.cpp
    src/core/components/LightComponent.cpp
    src/core/components/SkyLightComponent.cpp
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
    src/rendering/DirectionalLight.cpp
    src/rendering/Framebuffer.cpp
    src/rendering/Material.cpp
    src/rendering/GridRenderer.cpp
    src/rendering/Luminite/LuminiteSubsystem.cpp
    src/rendering/Luminite/SkyLightSubsystem.cpp
    src/rendering/Luminite/ShadowSystem.cpp
    src/rendering/PostProcessor.cpp
    src/rendering/ExposureSystem.cpp
)

set(UI_SOURCES
    src/ui/UI.cpp
    src/ui/SceneViewport.cpp
    src/ui/Hierarchy.cpp
    src/ui/Inspector.cpp
    src/ui/ContentBrowser.cpp
    src/ui/Toolbar.cpp
    src/ui/MainMenuBar.cpp
    src/ui/Details.cpp
    src/ui/Profiler.cpp
    src/ui/Preferences.cpp
    src/ui/ProjectSettings.cpp
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
endif()

