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

#include <string>

namespace LGE {

/**
 * @brief Base class for all events
 * 
 * Events are dispatched through the LayerStack to layers and overlays.
 * Layers can mark events as handled to stop propagation.
 */
class Event {
public:
    Event() : Handled(false) {}
    virtual ~Event() = default;

    /**
     * @brief Get the name of this event type
     * @return The event type name
     */
    virtual std::string GetEventName() const = 0;

    /**
     * @brief Get a string representation of this event
     * @return String representation
     */
    virtual std::string ToString() const { return GetEventName(); }

    /**
     * @brief Check if this event has been handled
     * @return True if handled, false otherwise
     */
    bool IsHandled() const { return Handled; }

    /**
     * @brief Mark this event as handled
     */
    void SetHandled(bool handled = true) { Handled = handled; }

    /**
     * @brief Whether this event has been handled (stops propagation)
     */
    bool Handled = false;
};

} // namespace LGE

