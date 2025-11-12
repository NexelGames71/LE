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

// Exposure mode
enum class EExposureMode {
    Manual,     // Manual exposure control
    Auto        // Automatic exposure based on scene luminance
};

// Exposure system for auto exposure calculation
class ExposureSystem {
public:
    ExposureSystem();
    ~ExposureSystem() = default;

    // Update exposure (call each frame)
    void Update(float deltaTime, uint32_t hdrTextureID, uint32_t width, uint32_t height);

    // Get current exposure value
    float GetExposure() const { return m_CurrentExposure; }

    // Manual exposure control
    void SetManualExposure(float ev100);
    float GetManualExposure() const { return m_ManualEV100; }

    // Auto exposure settings
    void SetAutoExposure(bool enabled) { m_AutoExposure = enabled; }
    bool GetAutoExposure() const { return m_AutoExposure; }
    
    void SetTargetLuminance(float target) { m_TargetLuminance = target; }
    float GetTargetLuminance() const { return m_TargetLuminance; }
    
    void SetAdaptationSpeed(float speed) { m_AdaptationSpeed = speed; }
    float GetAdaptationSpeed() const { return m_AdaptationSpeed; }
    
    void SetMinExposure(float minExp) { m_MinExposure = minExp; }
    void SetMaxExposure(float maxExp) { m_MaxExposure = maxExp; }
    float GetMinExposure() const { return m_MinExposure; }
    float GetMaxExposure() const { return m_MaxExposure; }

    // Calculate exposure from EV100 (Exposure Value)
    static float EV100ToExposure(float ev100);

private:
    // Calculate average luminance from HDR texture (downsampled)
    float CalculateAverageLuminance(uint32_t hdrTextureID, uint32_t width, uint32_t height);

    bool m_AutoExposure;
    float m_CurrentExposure;
    float m_ManualEV100;
    
    // Auto exposure parameters
    float m_TargetLuminance;    // Target luminance (middle gray, typically 0.18)
    float m_AdaptationSpeed;    // How fast exposure adapts (seconds)
    float m_MinExposure;
    float m_MaxExposure;
    
    // Internal state
    float m_LastLuminance;
};

} // namespace LGE

