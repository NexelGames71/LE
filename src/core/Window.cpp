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

#include "LGE/core/Window.h"
#include "LGE/core/Log.h"
#include "LGE/core/Input.h"
#include <GLFW/glfw3.h>

namespace LGE {

Window::Window(const WindowProperties& props) {
    Init(props);
}

Window::~Window() {
    Shutdown();
}

void Window::Init(const WindowProperties& props) {
    m_Data.Title = props.Title;
    m_Data.Width = props.Width;
    m_Data.Height = props.Height;
    m_Data.VSync = props.VSync;

    Log::Info("Creating window: " + props.Title + " (" + 
              std::to_string(props.Width) + "x" + std::to_string(props.Height) + ")");

    if (!glfwInit()) {
        Log::Fatal("Failed to initialize GLFW!");
        return;
    }

    // Set OpenGL version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Window = glfwCreateWindow(
        static_cast<int>(props.Width),
        static_cast<int>(props.Height),
        props.Title.c_str(),
        props.Fullscreen ? glfwGetPrimaryMonitor() : nullptr,
        nullptr
    );

    if (!m_Window) {
        Log::Fatal("Failed to create GLFW window!");
        const char* description;
        int code = glfwGetError(&description);
        if (description) {
            Log::Fatal("GLFW Error: " + std::string(description) + " (code: " + std::to_string(code) + ")");
        }
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_Window);
    SetVSync(props.VSync);
    
    // Ensure window is visible
    glfwShowWindow(m_Window);

    // Set user pointer for callbacks
    glfwSetWindowUserPointer(m_Window, &m_Data);
    
    // Set Input window
    Input::SetWindow(m_Window);
    
    // Set scroll callback
    Input::SetScrollCallback([](GLFWwindow* window, double xoffset, double yoffset) {
        Input::s_ScrollX = xoffset;
        Input::s_ScrollY = yoffset;
    });

    // Window resize callback
    glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
        data.Width = width;
        data.Height = height;
        
        if (data.EventCallback) {
            data.EventCallback();
        }
    });

    Log::Info("Window created successfully!");
}

void Window::Shutdown() {
    if (m_Window) {
        glfwDestroyWindow(m_Window);
        m_Window = nullptr;
    }
    glfwTerminate();
}

void Window::OnUpdate() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    glfwSwapBuffers(m_Window);
}

void Window::SetVSync(bool enabled) {
    m_Data.VSync = enabled;
    glfwSwapInterval(enabled ? 1 : 0);
}

bool Window::ShouldClose() const {
    if (!m_Window) return true;
    return glfwWindowShouldClose(m_Window);
}

void Window::SetSize(uint32_t width, uint32_t height) {
    if (!m_Window) return;
    
    m_Data.Width = width;
    m_Data.Height = height;
    glfwSetWindowSize(m_Window, static_cast<int>(width), static_cast<int>(height));
}

void Window::CenterOnScreen() {
    if (!m_Window) return;
    
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor) return;
    
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    if (!mode) return;
    
    int monitorX, monitorY;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);
    
    int windowWidth, windowHeight;
    glfwGetWindowSize(m_Window, &windowWidth, &windowHeight);
    
    int centerX = monitorX + (mode->width - windowWidth) / 2;
    int centerY = monitorY + (mode->height - windowHeight) / 2;
    
    glfwSetWindowPos(m_Window, centerX, centerY);
}

} // namespace LGE

