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

#include "LGE/core/Input.h"
#include <GLFW/glfw3.h>

namespace LGE {

GLFWwindow* Input::s_Window = nullptr;
bool Input::s_MouseVisible = true;
double Input::s_ScrollX = 0.0;
double Input::s_ScrollY = 0.0;

bool Input::IsKeyPressed(int keycode) {
    if (!s_Window) return false;
    int state = glfwGetKey(s_Window, keycode);
    return state == GLFW_PRESS || state == GLFW_REPEAT;
}

bool Input::IsMouseButtonPressed(int button) {
    if (!s_Window) return false;
    return glfwGetMouseButton(s_Window, button) == GLFW_PRESS;
}

void Input::GetMousePosition(double& x, double& y) {
    if (s_Window) {
        glfwGetCursorPos(s_Window, &x, &y);
    } else {
        x = y = 0.0;
    }
}

void Input::SetMousePosition(double x, double y) {
    if (s_Window) {
        glfwSetCursorPos(s_Window, x, y);
    }
}

void Input::SetMouseVisible(bool visible) {
    if (!s_Window) return;
    s_MouseVisible = visible;
    glfwSetInputMode(s_Window, GLFW_CURSOR, 
        visible ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
}

void Input::GetScrollOffset(double& xOffset, double& yOffset) {
    // This will be set by the scroll callback
    xOffset = s_ScrollX;
    yOffset = s_ScrollY;
    // Reset after reading
    s_ScrollX = 0.0;
    s_ScrollY = 0.0;
}

void Input::SetScrollCallback(GLFWscrollfun callback) {
    if (s_Window) {
        glfwSetScrollCallback(s_Window, callback);
    }
}

} // namespace LGE

