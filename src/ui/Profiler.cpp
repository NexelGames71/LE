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

#include "LGE/ui/Profiler.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>

namespace LGE {

Profiler::Profiler()
    : m_CurrentFPS(0.0f)
    , m_CurrentFrameTime(0.0f)
{
    m_FPSHistory.resize(MAX_HISTORY_SIZE, 0.0f);
    m_FrameTimeHistory.resize(MAX_HISTORY_SIZE, 0.0f);
}

Profiler::~Profiler() {
}

void Profiler::Update(float deltaTime) {
    // Update FPS and frame time
    if (deltaTime > 0.0f) {
        m_CurrentFPS = 1.0f / deltaTime;
        m_CurrentFrameTime = deltaTime * 1000.0f; // Convert to milliseconds
    }
    
    // Update history
    m_FPSHistory.push_back(m_CurrentFPS);
    m_FrameTimeHistory.push_back(m_CurrentFrameTime);
    
    if (m_FPSHistory.size() > MAX_HISTORY_SIZE) {
        m_FPSHistory.pop_front();
    }
    if (m_FrameTimeHistory.size() > MAX_HISTORY_SIZE) {
        m_FrameTimeHistory.pop_front();
    }
}

void Profiler::DrawPerformanceGraph(const char* label, const std::deque<float>& history, float min, float max, ImVec2 size) {
    if (history.empty()) return;
    
    ImGui::PushStyleColor(ImGuiCol_PlotLines, ImVec4(0.40f, 0.70f, 1.00f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_PlotLinesHovered, ImVec4(0.50f, 0.80f, 1.00f, 1.00f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.08f, 0.08f, 0.09f, 1.00f));
    
    // Convert deque to array for ImGui
    std::vector<float> values(history.begin(), history.end());
    
    ImGui::PlotLines(label, values.data(), static_cast<int>(values.size()), 0, nullptr, min, max, size);
    
    ImGui::PopStyleColor(3);
}

void Profiler::OnUIRender() {
    ImGui::Begin("Profiler", nullptr);
    
    // Performance stats
    ImGui::Text("Performance");
    ImGui::Separator();
    
    // FPS
    ImGui::Text("FPS: %.1f", m_CurrentFPS);
    
    // Frame time
    ImGui::Text("Frame Time: %.2f ms", m_CurrentFrameTime);
    
    ImGui::Spacing();
    
    // FPS Graph
    ImGui::Text("FPS Graph");
    float fpsMin = 0.0f;
    float fpsMax = 120.0f;
    if (!m_FPSHistory.empty()) {
        auto minmax = std::minmax_element(m_FPSHistory.begin(), m_FPSHistory.end());
        fpsMin = std::max(0.0f, *minmax.first - 10.0f);
        fpsMax = *minmax.second + 10.0f;
    }
    DrawPerformanceGraph("##FPSGraph", m_FPSHistory, fpsMin, fpsMax, ImVec2(-1, 80.0f));
    
    ImGui::Spacing();
    
    // Frame Time Graph
    ImGui::Text("Frame Time Graph (ms)");
    float frameTimeMin = 0.0f;
    float frameTimeMax = 50.0f;
    if (!m_FrameTimeHistory.empty()) {
        auto minmax = std::minmax_element(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end());
        frameTimeMin = std::max(0.0f, *minmax.first - 5.0f);
        frameTimeMax = *minmax.second + 5.0f;
    }
    DrawPerformanceGraph("##FrameTimeGraph", m_FrameTimeHistory, frameTimeMin, frameTimeMax, ImVec2(-1, 80.0f));
    
    ImGui::Spacing();
    ImGui::Separator();
    
    // Additional stats section
    ImGui::Text("Statistics");
    ImGui::Separator();
    
    // Calculate averages
    float avgFPS = 0.0f;
    float avgFrameTime = 0.0f;
    if (!m_FPSHistory.empty()) {
        for (float fps : m_FPSHistory) {
            avgFPS += fps;
        }
        avgFPS /= m_FPSHistory.size();
    }
    if (!m_FrameTimeHistory.empty()) {
        for (float ft : m_FrameTimeHistory) {
            avgFrameTime += ft;
        }
        avgFrameTime /= m_FrameTimeHistory.size();
    }
    
    ImGui::Text("Average FPS: %.1f", avgFPS);
    ImGui::Text("Average Frame Time: %.2f ms", avgFrameTime);
    
    // Min/Max
    if (!m_FPSHistory.empty()) {
        auto minmax = std::minmax_element(m_FPSHistory.begin(), m_FPSHistory.end());
        ImGui::Text("Min FPS: %.1f", *minmax.first);
        ImGui::Text("Max FPS: %.1f", *minmax.second);
    }
    
    if (!m_FrameTimeHistory.empty()) {
        auto minmax = std::minmax_element(m_FrameTimeHistory.begin(), m_FrameTimeHistory.end());
        ImGui::Text("Min Frame Time: %.2f ms", *minmax.first);
        ImGui::Text("Max Frame Time: %.2f ms", *minmax.second);
    }
    
    ImGui::End();
}

} // namespace LGE

