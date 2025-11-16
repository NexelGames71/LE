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
#include <vector>
#include <memory>
#include <algorithm>

namespace LGE {

/**
 * @brief Manages a stack of layers and overlays
 * 
 * The LayerStack maintains two separate lists:
 * - Layers: Regular layers that are updated and rendered in order
 * - Overlays: Layers that are rendered on top of regular layers (e.g., UI overlays)
 * 
 * Layers are updated and rendered in the order they were pushed.
 * Overlays are rendered after all regular layers.
 */
class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack();

    /**
     * @brief Push a layer onto the stack
     * @param layer The layer to push
     * 
     * Layers are inserted before the overlay insertion point,
     * so they are updated and rendered before overlays.
     */
    void PushLayer(std::shared_ptr<Layer> layer);

    /**
     * @brief Push an overlay onto the stack
     * @param overlay The overlay to push
     * 
     * Overlays are always rendered on top of regular layers.
     */
    void PushOverlay(std::shared_ptr<Layer> overlay);

    /**
     * @brief Pop a layer from the stack
     * @param layer The layer to pop
     */
    void PopLayer(std::shared_ptr<Layer> layer);

    /**
     * @brief Pop an overlay from the stack
     * @param overlay The overlay to pop
     */
    void PopOverlay(std::shared_ptr<Layer> overlay);

    /**
     * @brief Remove a layer from the stack (by name)
     * @param name The name of the layer to remove
     * @return True if the layer was found and removed, false otherwise
     */
    bool RemoveLayer(const std::string& name);

    /**
     * @brief Remove an overlay from the stack (by name)
     * @param name The name of the overlay to remove
     * @return True if the overlay was found and removed, false otherwise
     */
    bool RemoveOverlay(const std::string& name);

    /**
     * @brief Find a layer by name
     * @param name The name of the layer to find
     * @return Shared pointer to the layer, or nullptr if not found
     */
    std::shared_ptr<Layer> FindLayer(const std::string& name) const;

    /**
     * @brief Find an overlay by name
     * @param name The name of the overlay to find
     * @return Shared pointer to the overlay, or nullptr if not found
     */
    std::shared_ptr<Layer> FindOverlay(const std::string& name) const;

    /**
     * @brief Get all layers
     * @return Const reference to the layers vector
     */
    const std::vector<std::shared_ptr<Layer>>& GetLayers() const { return m_Layers; }

    /**
     * @brief Get all overlays
     * @return Const reference to the overlays vector
     */
    const std::vector<std::shared_ptr<Layer>>& GetOverlays() const { return m_Overlays; }

    /**
     * @brief Clear all layers and overlays
     */
    void Clear();

    /**
     * @brief Update all enabled layers
     * @param deltaTime Time elapsed since last frame in seconds
     */
    void OnUpdate(float deltaTime);

    /**
     * @brief Render all enabled layers and overlays
     */
    void OnRender();

    /**
     * @brief Dispatch an event to all layers (from top to bottom)
     * @param event The event to dispatch
     * 
     * Events are sent to overlays first (in reverse order), then to layers (in reverse order).
     * This allows top layers to handle events before lower layers.
     */
    void OnEvent(Event& event);

    /**
     * @brief Get the number of layers
     * @return The number of layers
     */
    size_t GetLayerCount() const { return m_Layers.size(); }

    /**
     * @brief Get the number of overlays
     * @return The number of overlays
     */
    size_t GetOverlayCount() const { return m_Overlays.size(); }

private:
    std::vector<std::shared_ptr<Layer>> m_Layers;
    std::vector<std::shared_ptr<Layer>> m_Overlays;
};

} // namespace LGE

