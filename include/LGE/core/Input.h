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

#include <GLFW/glfw3.h>

namespace LGE {

class Input {
public:
    static bool IsKeyPressed(int keycode);
    static bool IsMouseButtonPressed(int button);
    static void GetMousePosition(double& x, double& y);
    static void SetMousePosition(double x, double y);
    static void SetMouseVisible(bool visible);
    static bool IsMouseVisible() { return s_MouseVisible; }
    static void SetWindow(GLFWwindow* window) { s_Window = window; }
    static void GetScrollOffset(double& xOffset, double& yOffset);
    static void SetScrollCallback(GLFWscrollfun callback);

private:
    static GLFWwindow* s_Window;
    static bool s_MouseVisible;
    static double s_ScrollX;
    static double s_ScrollY;
    
    friend class Window;
};

} // namespace LGE

