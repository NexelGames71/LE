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

#include "LGE/core/Event.h"
#include <string>

namespace LGE {

/**
 * @brief Base class for all application layers
 * 
 * Layers are used to organize application functionality into separate modules.
 * They can be pushed onto the layer stack and will receive update, render, and event callbacks.
 * Overlay layers are rendered on top of regular layers.
 */
class Layer {
public:
    Layer(const std::string& name = "Layer");
    virtual ~Layer() = default;

    /**
     * @brief Called when the layer is attached to the layer stack
     */
    virtual void OnAttach() {}

    /**
     * @brief Called when the layer is detached from the layer stack
     */
    virtual void OnDetach() {}

    /**
     * @brief Called every frame with the delta time
     * @param deltaTime Time elapsed since last frame in seconds
     */
    virtual void OnUpdate(float deltaTime) {}

    /**
     * @brief Called every frame for rendering
     */
    virtual void OnRender() {}

    /**
     * @brief Called when an event occurs
     * @param event The event that occurred
     */
    virtual void OnEvent(Event& event) {}

    /**
     * @brief Get the name of this layer
     * @return The layer's name
     */
    const std::string& GetName() const { return m_Name; }

    /**
     * @brief Check if this layer is enabled
     * @return True if enabled, false otherwise
     */
    bool IsEnabled() const { return m_Enabled; }

    /**
     * @brief Enable or disable this layer
     * @param enabled Whether to enable the layer
     */
    void SetEnabled(bool enabled) { m_Enabled = enabled; }

protected:
    std::string m_Name;
    bool m_Enabled;
};

} // namespace LGE

