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
#include <sstream>
#include <iomanip>

namespace LGE {

// Import settings structure (using JSON string for simplicity)
struct ImportSettings {
    std::string settings; // JSON string
    
    ImportSettings() : settings("{}") {}
    
    // Get setting value (with default)
    template<typename T>
    T Get(const std::string& key, const T& defaultValue) const;
    
    // Set setting value
    template<typename T>
    void Set(const std::string& key, const T& value);
    
    // Get as JSON string
    std::string ToJson() const { return settings; }
    
    // Load from JSON string
    void FromJson(const std::string& json) { settings = json; }
    
    // Check if setting exists
    bool Has(const std::string& key) const;
};

// Specialized Get implementations
template<>
inline bool ImportSettings::Get<bool>(const std::string& key, const bool& defaultValue) const {
    // Simple JSON parsing for boolean
    size_t pos = settings.find("\"" + key + "\"");
    if (pos != std::string::npos) {
        size_t colonPos = settings.find(':', pos);
        if (colonPos != std::string::npos) {
            size_t valueStart = colonPos + 1;
            while (valueStart < settings.length() && (settings[valueStart] == ' ' || settings[valueStart] == '\t')) {
                ++valueStart;
            }
            if (valueStart < settings.length()) {
                if (settings.substr(valueStart, 4) == "true") return true;
                if (settings.substr(valueStart, 5) == "false") return false;
            }
        }
    }
    return defaultValue;
}

template<>
inline int ImportSettings::Get<int>(const std::string& key, const int& defaultValue) const {
    size_t pos = settings.find("\"" + key + "\"");
    if (pos != std::string::npos) {
        size_t colonPos = settings.find(':', pos);
        if (colonPos != std::string::npos) {
            size_t numStart = settings.find_first_of("-0123456789", colonPos);
            if (numStart != std::string::npos) {
                size_t numEnd = settings.find_first_not_of("0123456789", numStart + 1);
                if (numEnd == std::string::npos) numEnd = settings.length();
                try {
                    return std::stoi(settings.substr(numStart, numEnd - numStart));
                } catch (...) {}
            }
        }
    }
    return defaultValue;
}

template<>
inline std::string ImportSettings::Get<std::string>(const std::string& key, const std::string& defaultValue) const {
    size_t pos = settings.find("\"" + key + "\"");
    if (pos != std::string::npos) {
        size_t colonPos = settings.find(':', pos);
        size_t quoteStart = settings.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = settings.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                return settings.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    return defaultValue;
}

template<typename T>
T ImportSettings::Get(const std::string& key, const T& defaultValue) const {
    // Default implementation for unsupported types
    return defaultValue;
}

template<>
inline void ImportSettings::Set<bool>(const std::string& key, const bool& value) {
    // Simple JSON update
    std::string searchKey = "\"" + key + "\"";
    size_t pos = settings.find(searchKey);
    if (pos != std::string::npos) {
        size_t colonPos = settings.find(':', pos);
        if (colonPos != std::string::npos) {
            size_t valueEnd = settings.find_first_of(",}", colonPos);
            if (valueEnd != std::string::npos) {
                std::string newValue = value ? "true" : "false";
                settings.replace(colonPos + 1, valueEnd - colonPos - 1, " " + newValue);
                return;
            }
        }
    }
    // Add new key-value pair
    if (settings.length() > 2) {
        settings.insert(settings.length() - 1, ",\n    \"" + key + "\": " + (value ? "true" : "false"));
    } else {
        settings = "{\n    \"" + key + "\": " + (value ? "true" : "false") + "\n}";
    }
}

template<>
inline void ImportSettings::Set<int>(const std::string& key, const int& value) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = settings.find(searchKey);
    if (pos != std::string::npos) {
        size_t colonPos = settings.find(':', pos);
        if (colonPos != std::string::npos) {
            size_t valueEnd = settings.find_first_of(",}", colonPos);
            if (valueEnd != std::string::npos) {
                settings.replace(colonPos + 1, valueEnd - colonPos - 1, " " + std::to_string(value));
                return;
            }
        }
    }
    if (settings.length() > 2) {
        settings.insert(settings.length() - 1, ",\n    \"" + key + "\": " + std::to_string(value));
    } else {
        settings = "{\n    \"" + key + "\": " + std::to_string(value) + "\n}";
    }
}

template<>
inline void ImportSettings::Set<std::string>(const std::string& key, const std::string& value) {
    std::string searchKey = "\"" + key + "\"";
    size_t pos = settings.find(searchKey);
    if (pos != std::string::npos) {
        size_t colonPos = settings.find(':', pos);
        size_t quoteStart = settings.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = settings.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                settings.replace(quoteStart + 1, quoteEnd - quoteStart - 1, value);
                return;
            }
        }
    }
    if (settings.length() > 2) {
        settings.insert(settings.length() - 1, ",\n    \"" + key + "\": \"" + value + "\"");
    } else {
        settings = "{\n    \"" + key + "\": \"" + value + "\"\n}";
    }
}

template<typename T>
void ImportSettings::Set(const std::string& key, const T& value) {
    // Default implementation for unsupported types
    // Could be extended for other types
}

inline bool ImportSettings::Has(const std::string& key) const {
    return settings.find("\"" + key + "\"") != std::string::npos;
}

} // namespace LGE



