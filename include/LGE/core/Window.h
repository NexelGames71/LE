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
#include <functional>

struct GLFWwindow;

namespace LGE {

struct WindowProperties {
    std::string Title;
    uint32_t Width;
    uint32_t Height;
    bool VSync;
    bool Fullscreen;

    WindowProperties(
        const std::string& title = "LGE Window",
        uint32_t width = 1280,
        uint32_t height = 720,
        bool vsync = true,
        bool fullscreen = false
    ) : Title(title), Width(width), Height(height), VSync(vsync), Fullscreen(fullscreen) {}
};

class Window {
public:
    using EventCallbackFn = std::function<void()>;

    Window(const WindowProperties& props);
    ~Window();

    void OnUpdate();
    void SwapBuffers();

    uint32_t GetWidth() const { return m_Data.Width; }
    uint32_t GetHeight() const { return m_Data.Height; }
    
    void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
    void SetVSync(bool enabled);
    bool IsVSync() const { return m_Data.VSync; }

    GLFWwindow* GetNativeWindow() const { return m_Window; }

    bool ShouldClose() const;

private:
    void Init(const WindowProperties& props);
    void Shutdown();

    GLFWwindow* m_Window;
    
    struct WindowData {
        std::string Title;
        uint32_t Width, Height;
        bool VSync;
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};

} // namespace LGE

