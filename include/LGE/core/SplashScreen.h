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

#include "LGE/core/Layer.h"
#include "LGE/core/Event.h"
#include <memory>
#include <chrono>

namespace LGE {

class Texture;
class Window;
class LayerStack;

/**
 * @brief Splash screen layer that displays at application startup
 * 
 * The splash screen automatically fades in, displays for a duration,
 * then fades out and removes itself from the layer stack.
 */
class SplashScreen : public Layer {
public:
    SplashScreen(const std::string& imagePath, float displayDuration = 3.0f);
    ~SplashScreen() override = default;

    void OnAttach() override;
    void OnDetach() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnEvent(Event& event) override;

    /**
     * @brief Set the layer stack reference (called by Application)
     */
    void SetLayerStack(LayerStack* layerStack) { m_LayerStackPtr = layerStack; }

    /**
     * @brief Set the window reference (called by Application)
     */
    void SetWindow(Window* window) { m_Window = window; }

    /**
     * @brief Set the fade in duration
     * @param duration Duration in seconds
     */
    void SetFadeInDuration(float duration) { m_FadeInDuration = duration; }

    /**
     * @brief Set the fade out duration
     * @param duration Duration in seconds
     */
    void SetFadeOutDuration(float duration) { m_FadeOutDuration = duration; }

    /**
     * @brief Set the display duration (time shown at full opacity)
     * @param duration Duration in seconds
     */
    void SetDisplayDuration(float duration) { m_DisplayDuration = duration; }

    /**
     * @brief Set the window scale factor (0.0 to 1.0, where 1.0 is full size)
     * @param scale Scale factor for window size
     */
    void SetWindowScale(float scale) { m_WindowScale = scale; }

    /**
     * @brief Skip the splash screen (immediately remove it)
     */
    void Skip();

    /**
     * @brief Check if the splash screen is finished
     * @return True if finished (faded out completely)
     */
    bool IsFinished() const { return m_State == State::Finished; }

private:
    enum class State {
        FadingIn,
        Displaying,
        FadingOut,
        Finished
    };

    void UpdateAlpha(float deltaTime);
    float GetCurrentAlpha() const;
    void RenderFullscreenImage();

    std::string m_ImagePath;
    std::shared_ptr<Texture> m_SplashTexture;
    
    State m_State;
    float m_ElapsedTime;
    float m_FadeInDuration;
    float m_FadeOutDuration;
    float m_DisplayDuration;
    float m_CurrentAlpha;
    
    LayerStack* m_LayerStackPtr;
    Window* m_Window;
    
    // Original window size (to restore after splash)
    uint32_t m_OriginalWidth;
    uint32_t m_OriginalHeight;
    bool m_WindowResized;
    float m_WindowScale;
    
    std::chrono::high_resolution_clock::time_point m_StartTime;
};

} // namespace LGE

