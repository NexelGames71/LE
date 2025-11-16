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

#include "LGE/core/LayerStack.h"
#include "LGE/core/Layer.h"
#include "LGE/core/Event.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

LayerStack::~LayerStack() {
    Clear();
}

void LayerStack::PushLayer(std::shared_ptr<Layer> layer) {
    if (!layer) {
        Log::Warn("Attempted to push null layer to LayerStack");
        return;
    }

    m_Layers.emplace_back(layer);
    layer->OnAttach();
    Log::Trace("Layer '" + layer->GetName() + "' pushed to LayerStack");
}

void LayerStack::PushOverlay(std::shared_ptr<Layer> overlay) {
    if (!overlay) {
        Log::Warn("Attempted to push null overlay to LayerStack");
        return;
    }

    m_Overlays.emplace_back(overlay);
    overlay->OnAttach();
    Log::Trace("Overlay '" + overlay->GetName() + "' pushed to LayerStack");
}

void LayerStack::PopLayer(std::shared_ptr<Layer> layer) {
    if (!layer) return;

    auto it = std::find(m_Layers.begin(), m_Layers.end(), layer);
    if (it != m_Layers.end()) {
        (*it)->OnDetach();
        m_Layers.erase(it);
        Log::Trace("Layer '" + layer->GetName() + "' popped from LayerStack");
    }
}

void LayerStack::PopOverlay(std::shared_ptr<Layer> overlay) {
    if (!overlay) return;

    auto it = std::find(m_Overlays.begin(), m_Overlays.end(), overlay);
    if (it != m_Overlays.end()) {
        (*it)->OnDetach();
        m_Overlays.erase(it);
        Log::Trace("Overlay '" + overlay->GetName() + "' popped from LayerStack");
    }
}

bool LayerStack::RemoveLayer(const std::string& name) {
    auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
        [&name](const std::shared_ptr<Layer>& layer) {
            return layer && layer->GetName() == name;
        });

    if (it != m_Layers.end()) {
        (*it)->OnDetach();
        m_Layers.erase(it);
        Log::Trace("Layer '" + name + "' removed from LayerStack");
        return true;
    }

    return false;
}

bool LayerStack::RemoveOverlay(const std::string& name) {
    auto it = std::find_if(m_Overlays.begin(), m_Overlays.end(),
        [&name](const std::shared_ptr<Layer>& overlay) {
            return overlay && overlay->GetName() == name;
        });

    if (it != m_Overlays.end()) {
        (*it)->OnDetach();
        m_Overlays.erase(it);
        Log::Trace("Overlay '" + name + "' removed from LayerStack");
        return true;
    }

    return false;
}

std::shared_ptr<Layer> LayerStack::FindLayer(const std::string& name) const {
    auto it = std::find_if(m_Layers.begin(), m_Layers.end(),
        [&name](const std::shared_ptr<Layer>& layer) {
            return layer && layer->GetName() == name;
        });

    return (it != m_Layers.end()) ? *it : nullptr;
}

std::shared_ptr<Layer> LayerStack::FindOverlay(const std::string& name) const {
    auto it = std::find_if(m_Overlays.begin(), m_Overlays.end(),
        [&name](const std::shared_ptr<Layer>& overlay) {
            return overlay && overlay->GetName() == name;
        });

    return (it != m_Overlays.end()) ? *it : nullptr;
}

void LayerStack::Clear() {
    // Detach all overlays first (in reverse order)
    for (auto it = m_Overlays.rbegin(); it != m_Overlays.rend(); ++it) {
        if (*it) {
            (*it)->OnDetach();
        }
    }

    // Detach all layers (in reverse order)
    for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
        if (*it) {
            (*it)->OnDetach();
        }
    }

    m_Overlays.clear();
    m_Layers.clear();
    Log::Trace("LayerStack cleared");
}

void LayerStack::OnUpdate(float deltaTime) {
    // Update all enabled layers
    for (auto& layer : m_Layers) {
        if (layer && layer->IsEnabled()) {
            layer->OnUpdate(deltaTime);
        }
    }

    // Update all enabled overlays
    for (auto& overlay : m_Overlays) {
        if (overlay && overlay->IsEnabled()) {
            overlay->OnUpdate(deltaTime);
        }
    }
}

void LayerStack::OnRender() {
    // Render all enabled layers
    for (auto& layer : m_Layers) {
        if (layer && layer->IsEnabled()) {
            layer->OnRender();
        }
    }

    // Render all enabled overlays (on top)
    for (auto& overlay : m_Overlays) {
        if (overlay && overlay->IsEnabled()) {
            overlay->OnRender();
        }
    }
}

void LayerStack::OnEvent(Event& event) {
    // Send event to overlays first (in reverse order - top to bottom)
    for (auto it = m_Overlays.rbegin(); it != m_Overlays.rend(); ++it) {
        if (*it && (*it)->IsEnabled()) {
            (*it)->OnEvent(event);
            if (event.IsHandled()) {
                return; // Event was handled, stop propagation
            }
        }
    }

    // Send event to layers (in reverse order - top to bottom)
    for (auto it = m_Layers.rbegin(); it != m_Layers.rend(); ++it) {
        if (*it && (*it)->IsEnabled()) {
            (*it)->OnEvent(event);
            if (event.IsHandled()) {
                return; // Event was handled, stop propagation
            }
        }
    }
}

} // namespace LGE

