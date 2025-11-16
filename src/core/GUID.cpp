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

#include "LGE/core/GUID.h"
#include <random>
#include <sstream>
#include <iomanip>

namespace LGE {

std::random_device GUID::s_RandomDevice;
std::mt19937_64 GUID::s_RandomEngine(s_RandomDevice());

GUID::GUID()
    : m_High(0), m_Low(0)
{
}

GUID::GUID(uint64_t high, uint64_t low)
    : m_High(high), m_Low(low)
{
}

GUID GUID::Generate() {
    std::uniform_int_distribution<uint64_t> dist;
    return GUID(dist(s_RandomEngine), dist(s_RandomEngine));
}

GUID GUID::FromString(const std::string& str) {
    // Format: "01234567-89AB-CDEF-0123-456789ABCDEF"
    // Remove dashes and convert hex to uint64_t
    
    std::string clean = str;
    clean.erase(std::remove(clean.begin(), clean.end(), '-'), clean.end());
    
    if (clean.length() != 32) {
        return GUID::Invalid();
    }
    
    try {
        uint64_t high = std::stoull(clean.substr(0, 16), nullptr, 16);
        uint64_t low = std::stoull(clean.substr(16, 16), nullptr, 16);
        return GUID(high, low);
    } catch (...) {
        return GUID::Invalid();
    }
}

std::string GUID::ToString() const {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0') << std::uppercase;
    oss << std::setw(8) << ((m_High >> 32) & 0xFFFFFFFF) << "-";
    oss << std::setw(4) << ((m_High >> 16) & 0xFFFF) << "-";
    oss << std::setw(4) << (m_High & 0xFFFF) << "-";
    oss << std::setw(4) << ((m_Low >> 48) & 0xFFFF) << "-";
    oss << std::setw(12) << (m_Low & 0xFFFFFFFFFFFF);
    return oss.str();
}

bool GUID::IsValid() const {
    return m_High != 0 || m_Low != 0;
}

bool GUID::operator==(const GUID& other) const {
    return m_High == other.m_High && m_Low == other.m_Low;
}

bool GUID::operator!=(const GUID& other) const {
    return !(*this == other);
}

bool GUID::operator<(const GUID& other) const {
    if (m_High < other.m_High) return true;
    if (m_High > other.m_High) return false;
    return m_Low < other.m_Low;
}

} // namespace LGE

