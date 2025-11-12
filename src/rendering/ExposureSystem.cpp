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

#include "LGE/rendering/ExposureSystem.h"
#include <glad/glad.h>
#include <algorithm>
#include <cmath>

namespace LGE {

ExposureSystem::ExposureSystem()
    : m_AutoExposure(false)
    , m_CurrentExposure(1.0f)
    , m_ManualEV100(0.0f)
    , m_TargetLuminance(0.18f)  // Middle gray
    , m_AdaptationSpeed(1.0f)   // 1 second adaptation
    , m_MinExposure(0.1f)
    , m_MaxExposure(10.0f)
    , m_LastLuminance(0.18f)
{
}

void ExposureSystem::Update(float deltaTime, uint32_t hdrTextureID, uint32_t width, uint32_t height) {
    if (m_AutoExposure) {
        // Only calculate luminance every few frames to avoid performance issues
        // For now, disable auto exposure calculation as it's too slow with glReadPixels
        // TODO: Implement GPU-based luminance calculation using compute shader or mipmap downsampling
        // For now, just use manual exposure
        m_CurrentExposure = EV100ToExposure(m_ManualEV100);
        
        // Uncomment below when GPU-based luminance is implemented:
        /*
        // Calculate average luminance from HDR texture
        float avgLuminance = CalculateAverageLuminance(hdrTextureID, width, height);
        
        // Clamp luminance to avoid extreme values
        avgLuminance = std::max(0.01f, std::min(avgLuminance, 10.0f));
        
        // Calculate target exposure based on target luminance
        // Exposure = targetLuminance / currentLuminance
        float targetExposure = m_TargetLuminance / avgLuminance;
        
        // Clamp to min/max range
        targetExposure = std::max(m_MinExposure, std::min(m_MaxExposure, targetExposure));
        
        // Smoothly adapt exposure
        float adaptationFactor = 1.0f - std::exp(-deltaTime * m_AdaptationSpeed);
        m_CurrentExposure = m_CurrentExposure + (targetExposure - m_CurrentExposure) * adaptationFactor;
        
        m_LastLuminance = avgLuminance;
        */
    } else {
        // Manual exposure: convert EV100 to exposure
        m_CurrentExposure = EV100ToExposure(m_ManualEV100);
    }
}

float ExposureSystem::CalculateAverageLuminance(uint32_t hdrTextureID, uint32_t width, uint32_t height) {
    // Simple approach: sample a small region of the texture
    // For a full implementation, you'd want to use a compute shader with mipmap downsampling
    
    // For now, use a simple CPU-based sampling (this is slow, but works)
    // In production, use a compute shader or GPU-based histogram
    
    // Sample a 16x16 grid
    const int sampleSize = 16;
    float totalLuminance = 0.0f;
    int sampleCount = 0;
    
    // Create a temporary framebuffer to read from
    GLuint readFBO;
    glGenFramebuffers(1, &readFBO);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, hdrTextureID, 0);
    
    // Sample pixels
    float pixels[sampleSize * sampleSize * 3];
    for (int y = 0; y < sampleSize; ++y) {
        for (int x = 0; x < sampleSize; ++x) {
            int pixelX = (x * width) / sampleSize;
            int pixelY = (y * height) / sampleSize;
            
            glReadPixels(pixelX, pixelY, 1, 1, GL_RGB, GL_FLOAT, &pixels[(y * sampleSize + x) * 3]);
            
            // Calculate luminance (relative luminance formula)
            float r = pixels[(y * sampleSize + x) * 3 + 0];
            float g = pixels[(y * sampleSize + x) * 3 + 1];
            float b = pixels[(y * sampleSize + x) * 3 + 2];
            
            float luminance = 0.2126f * r + 0.7152f * g + 0.0722f * b;
            totalLuminance += luminance;
            sampleCount++;
        }
    }
    
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &readFBO);
    
    return sampleCount > 0 ? (totalLuminance / sampleCount) : m_TargetLuminance;
}

void ExposureSystem::SetManualExposure(float ev100) {
    m_ManualEV100 = ev100;
    m_CurrentExposure = EV100ToExposure(ev100);
}

float ExposureSystem::EV100ToExposure(float ev100) {
    // EV100 to exposure conversion
    // Exposure = 1.0 / (2^EV100)
    return 1.0f / std::pow(2.0f, ev100);
}

} // namespace LGE

