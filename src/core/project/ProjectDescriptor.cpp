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

#include "LGE/core/project/ProjectDescriptor.h"
#include "LGE/core/filesystem/FileSystem.h"
#include "LGE/core/Log.h"
#include <sstream>
#include <algorithm>

namespace LGE {

ProjectDescriptor::ProjectDescriptor()
    : projectName("")
    , engineVersion("0.1.0")
    , defaultScene("")
{
}

ProjectDescriptor::~ProjectDescriptor() {
}

bool ProjectDescriptor::Save(const std::filesystem::path& path) {
    std::string json = SerializeToJSON();
    
    if (json.empty()) {
        Log::Error("Failed to serialize project descriptor to JSON");
        return false;
    }
    
    if (!FileSystem::WriteFile(path.string(), json)) {
        Log::Error("Failed to write project descriptor to: " + path.string());
        return false;
    }
    
    Log::Info("Saved project descriptor to: " + path.string());
    return true;
}

bool ProjectDescriptor::Load(const std::filesystem::path& path) {
    if (!FileSystem::Exists(path.string())) {
        Log::Error("Project descriptor file not found: " + path.string());
        return false;
    }
    
    std::string json = FileSystem::ReadFile(path.string());
    if (json.empty()) {
        Log::Error("Failed to read project descriptor from: " + path.string());
        return false;
    }
    
    if (!DeserializeFromJSON(json)) {
        Log::Error("Failed to parse project descriptor JSON from: " + path.string());
        return false;
    }
    
    Log::Info("Loaded project descriptor from: " + path.string());
    return true;
}

void ProjectDescriptor::SetSetting(const std::string& key, const std::string& value) {
    settings[key] = value;
}

std::string ProjectDescriptor::GetSetting(const std::string& key, const std::string& defaultValue) const {
    auto it = settings.find(key);
    if (it != settings.end()) {
        return it->second;
    }
    return defaultValue;
}

bool ProjectDescriptor::HasSetting(const std::string& key) const {
    return settings.find(key) != settings.end();
}

void ProjectDescriptor::AddPlugin(const std::string& pluginName) {
    if (!HasPlugin(pluginName)) {
        plugins.push_back(pluginName);
    }
}

void ProjectDescriptor::RemovePlugin(const std::string& pluginName) {
    plugins.erase(
        std::remove_if(plugins.begin(), plugins.end(),
            [&pluginName](const std::string& p) { return p == pluginName; }),
        plugins.end()
    );
}

bool ProjectDescriptor::HasPlugin(const std::string& pluginName) const {
    return std::find(plugins.begin(), plugins.end(), pluginName) != plugins.end();
}

void ProjectDescriptor::Clear() {
    projectName.clear();
    engineVersion.clear();
    defaultScene.clear();
    plugins.clear();
    settings.clear();
}

std::string ProjectDescriptor::SerializeToJSON() const {
    std::ostringstream json;
    json << "{\n";
    
    // Project name
    json << "  \"projectName\": \"" << projectName << "\",\n";
    
    // Engine version
    json << "  \"engineVersion\": \"" << engineVersion << "\",\n";
    
    // Default scene
    json << "  \"defaultScene\": \"" << defaultScene << "\",\n";
    
    // Plugins array
    json << "  \"plugins\": [\n";
    for (size_t i = 0; i < plugins.size(); ++i) {
        json << "    \"" << plugins[i] << "\"";
        if (i < plugins.size() - 1) {
            json << ",";
        }
        json << "\n";
    }
    json << "  ],\n";
    
    // Settings object
    json << "  \"settings\": {\n";
    size_t settingIndex = 0;
    for (const auto& [key, value] : settings) {
        json << "    \"" << key << "\": \"" << value << "\"";
        if (settingIndex < settings.size() - 1) {
            json << ",";
        }
        json << "\n";
        ++settingIndex;
    }
    json << "  }\n";
    
    json << "}\n";
    
    return json.str();
}

bool ProjectDescriptor::DeserializeFromJSON(const std::string& json) {
    Clear();
    
    // Simple JSON parsing (basic implementation)
    // In a production engine, you'd use a proper JSON library like nlohmann/json
    
    // Extract projectName
    size_t namePos = json.find("\"projectName\"");
    if (namePos != std::string::npos) {
        size_t colonPos = json.find(':', namePos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                projectName = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Extract engineVersion
    size_t versionPos = json.find("\"engineVersion\"");
    if (versionPos != std::string::npos) {
        size_t colonPos = json.find(':', versionPos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                engineVersion = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Extract defaultScene
    size_t scenePos = json.find("\"defaultScene\"");
    if (scenePos != std::string::npos) {
        size_t colonPos = json.find(':', scenePos);
        size_t quoteStart = json.find('"', colonPos);
        if (quoteStart != std::string::npos) {
            size_t quoteEnd = json.find('"', quoteStart + 1);
            if (quoteEnd != std::string::npos) {
                defaultScene = json.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
            }
        }
    }
    
    // Extract plugins array (simplified - just find the array and parse entries)
    size_t pluginsPos = json.find("\"plugins\"");
    if (pluginsPos != std::string::npos) {
        size_t arrayStart = json.find('[', pluginsPos);
        size_t arrayEnd = json.find(']', arrayStart);
        if (arrayStart != std::string::npos && arrayEnd != std::string::npos) {
            std::string pluginsArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);
            size_t pos = 0;
            while ((pos = pluginsArray.find('"', pos)) != std::string::npos) {
                size_t quoteStart = pos;
                size_t quoteEnd = pluginsArray.find('"', quoteStart + 1);
                if (quoteEnd != std::string::npos) {
                    std::string plugin = pluginsArray.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
                    if (!plugin.empty()) {
                        plugins.push_back(plugin);
                    }
                    pos = quoteEnd + 1;
                } else {
                    break;
                }
            }
        }
    }
    
    // Extract settings object (simplified - parse key-value pairs)
    size_t settingsPos = json.find("\"settings\"");
    if (settingsPos != std::string::npos) {
        size_t objStart = json.find('{', settingsPos);
        size_t objEnd = json.find('}', objStart);
        if (objStart != std::string::npos && objEnd != std::string::npos) {
            std::string settingsObj = json.substr(objStart + 1, objEnd - objStart - 1);
            size_t pos = 0;
            while (pos < settingsObj.length()) {
                // Find key
                size_t keyStart = settingsObj.find('"', pos);
                if (keyStart == std::string::npos) break;
                size_t keyEnd = settingsObj.find('"', keyStart + 1);
                if (keyEnd == std::string::npos) break;
                std::string key = settingsObj.substr(keyStart + 1, keyEnd - keyStart - 1);
                
                // Find value
                size_t colonPos = settingsObj.find(':', keyEnd);
                if (colonPos == std::string::npos) break;
                size_t valueStart = settingsObj.find('"', colonPos);
                if (valueStart == std::string::npos) break;
                size_t valueEnd = settingsObj.find('"', valueStart + 1);
                if (valueEnd == std::string::npos) break;
                std::string value = settingsObj.substr(valueStart + 1, valueEnd - valueStart - 1);
                
                settings[key] = value;
                pos = valueEnd + 1;
            }
        }
    }
    
    return true;
}

} // namespace LGE

