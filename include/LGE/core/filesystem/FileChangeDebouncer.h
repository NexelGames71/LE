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

#include "LGE/core/filesystem/FileSystemWatcher.h"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include <functional>
#include <atomic>

namespace LGE {

class FileChangeDebouncer {
private:
    std::mutex m_Mutex;
    std::unordered_map<std::string, FileChangeEvent> m_PendingChanges;
    std::chrono::milliseconds m_DebounceDelay;
    std::thread m_ProcessingThread;
    std::atomic<bool> m_IsRunning;
    
    std::function<void(const std::vector<FileChangeEvent>&)> m_BatchCallback;
    
    void ProcessingLoop();

public:
    FileChangeDebouncer(std::chrono::milliseconds delay = std::chrono::milliseconds(500));
    ~FileChangeDebouncer();
    
    void AddEvent(const FileChangeEvent& event);
    void SetBatchCallback(std::function<void(const std::vector<FileChangeEvent>&)> cb);
    void Flush(); // Process immediately
    
    void Stop();
};

} // namespace LGE



