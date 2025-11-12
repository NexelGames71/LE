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

#include <cstdint>

namespace LGE {

class Window;

class UI {
public:
    static void Initialize(Window* window);
    static void Shutdown();
    static void BeginFrame();
    static void EndFrame();
    static void Render();

    static bool WantCaptureMouse();
    static bool WantCaptureKeyboard();
};

} // namespace LGE





