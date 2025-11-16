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

#include <filesystem>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>

namespace LGE {

enum class FileChangeType {
    Added,
    Modified,
    Removed,
    Renamed
};

struct FileChangeEvent {
    FileChangeType type;
    std::filesystem::path path;
    std::filesystem::path oldPath; // For rename events
    std::time_t timestamp;
    
    FileChangeEvent()
        : type(FileChangeType::Added)
        , timestamp(0)
    {}
    
    FileChangeEvent(FileChangeType t, const std::filesystem::path& p)
        : type(t)
        , path(p)
        , timestamp(std::time(nullptr))
    {}
};

class FileSystemWatcher {
protected:
    std::filesystem::path m_WatchedDirectory;
    std::atomic<bool> m_IsWatching;
    std::function<void(const FileChangeEvent&)> m_Callback;

public:
    FileSystemWatcher() : m_IsWatching(false) {}
    virtual ~FileSystemWatcher() = default;
    
    virtual bool StartWatching(const std::filesystem::path& directory) = 0;
    virtual void StopWatching() = 0;
    virtual bool IsWatching() const { return m_IsWatching.load(); }
    
    void SetCallback(std::function<void(const FileChangeEvent&)> cb) {
        m_Callback = cb;
    }
    
    // Factory method
    static std::unique_ptr<FileSystemWatcher> Create();
    
    std::filesystem::path GetWatchedDirectory() const { return m_WatchedDirectory; }
};

} // namespace LGE



