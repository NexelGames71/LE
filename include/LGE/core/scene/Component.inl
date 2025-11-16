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

// Template implementations for Component helper methods
// Note: This file is included inside the LGE namespace in Component.h

template<typename T>
T* Component::GetComponent() const {
    if (m_Owner) {
        return m_Owner->GetComponent<T>();
    }
    return nullptr;
}

template<typename T>
std::vector<T*> Component::GetComponents() const {
    if (m_Owner) {
        return m_Owner->GetComponents<T>();
    }
    return std::vector<T*>();
}

