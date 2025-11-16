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

class Texture;

class Toolbar {
public:
    Toolbar();
    ~Toolbar();

    void OnUIRender();
    
    // Set the current scene name to display
    void SetCurrentSceneName(const std::string& sceneName) { m_CurrentSceneName = sceneName; }
    const std::string& GetCurrentSceneName() const { return m_CurrentSceneName; }

private:
    void LoadIcons();
    
    std::shared_ptr<Texture> m_PlayIcon;
    std::shared_ptr<Texture> m_PauseIcon;
    std::shared_ptr<Texture> m_StopIcon;
    bool m_IconsLoaded;
    std::string m_CurrentSceneName;
};

} // namespace LGE


