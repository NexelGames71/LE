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
#include <vector>
#include <map>
#include <filesystem>

namespace LGE {

// Project descriptor class for managing project metadata
class ProjectDescriptor {
public:
    ProjectDescriptor();
    ~ProjectDescriptor();

    // Project information
    std::string projectName;
    std::string engineVersion;
    std::string defaultScene;
    
    // Plugins list
    std::vector<std::string> plugins;
    
    // Custom settings (key-value pairs)
    std::map<std::string, std::string> settings;
    
    // Save descriptor to file
    bool Save(const std::filesystem::path& path);
    
    // Load descriptor from file
    bool Load(const std::filesystem::path& path);
    
    // Helper methods
    void SetSetting(const std::string& key, const std::string& value);
    std::string GetSetting(const std::string& key, const std::string& defaultValue = "") const;
    bool HasSetting(const std::string& key) const;
    
    void AddPlugin(const std::string& pluginName);
    void RemovePlugin(const std::string& pluginName);
    bool HasPlugin(const std::string& pluginName) const;
    
    // Clear all data
    void Clear();

private:
    // Helper methods for JSON serialization
    std::string SerializeToJSON() const;
    bool DeserializeFromJSON(const std::string& json);
};

} // namespace LGE

