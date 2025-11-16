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
#include <string>
#include <random>
#include <sstream>
#include <iomanip>

namespace LGE {

// Globally Unique Identifier
class GUID {
private:
    uint64_t m_High;
    uint64_t m_Low;
    
    static std::random_device s_RandomDevice;
    static std::mt19937_64 s_RandomEngine;

public:
    GUID();
    GUID(uint64_t high, uint64_t low);
    
    // Generate new GUID
    static GUID Generate();
    
    // Create GUID from string (format: "01234567-89AB-CDEF-0123-456789ABCDEF")
    static GUID FromString(const std::string& str);
    
    // Convert GUID to string
    std::string ToString() const;
    
    // Check if GUID is valid (not all zeros)
    bool IsValid() const;
    
    // Comparison operators
    bool operator==(const GUID& other) const;
    bool operator!=(const GUID& other) const;
    bool operator<(const GUID& other) const;
    
    // Get high and low parts
    uint64_t GetHigh() const { return m_High; }
    uint64_t GetLow() const { return m_Low; }
    
    // Create invalid GUID (all zeros)
    static GUID Invalid() { return GUID(0, 0); }
};

} // namespace LGE

// Hash function for unordered_map
namespace std {
    template<>
    struct hash<LGE::GUID> {
        size_t operator()(const LGE::GUID& guid) const {
            return hash<uint64_t>()(guid.GetHigh()) ^ (hash<uint64_t>()(guid.GetLow()) << 1);
        }
    };
}

