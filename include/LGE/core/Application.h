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

#include <memory>
#include <string>

namespace LGE {

class Window;
class Renderer;

class Application {
public:
    Application(const std::string& name = "LGE Application");
    virtual ~Application();

    // Initialize the application
    virtual bool Initialize();
    
    // Run the main loop
    void Run();
    
    // Cleanup resources
    virtual void Shutdown();

    // Override these in derived classes
    virtual void OnUpdate(float deltaTime) {}
    virtual void OnRender() {}
    virtual void OnEvent() {}

protected:
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Renderer> m_Renderer;
    bool m_Running;
    std::string m_Name;
};

} // namespace LGE

