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

#include <vector>
#include <deque>

struct ImVec2;

namespace LGE {

class Profiler {
public:
    Profiler();
    ~Profiler();

    void OnUIRender();
    void Update(float deltaTime);
    
    void SetFPS(float fps) { m_CurrentFPS = fps; }
    void SetFrameTime(float frameTime) { m_CurrentFrameTime = frameTime; }

private:
    float m_CurrentFPS;
    float m_CurrentFrameTime;
    
    // History for graphs
    std::deque<float> m_FPSHistory;
    std::deque<float> m_FrameTimeHistory;
    static constexpr size_t MAX_HISTORY_SIZE = 100;
    
    void DrawPerformanceGraph(const char* label, const std::deque<float>& history, float min, float max, ImVec2 size);
};

} // namespace LGE

