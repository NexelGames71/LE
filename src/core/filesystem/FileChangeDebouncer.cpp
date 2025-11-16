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

#include "LGE/core/filesystem/FileChangeDebouncer.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

FileChangeDebouncer::FileChangeDebouncer(std::chrono::milliseconds delay)
    : m_DebounceDelay(delay)
    , m_IsRunning(true)
{
    m_ProcessingThread = std::thread(&FileChangeDebouncer::ProcessingLoop, this);
}

FileChangeDebouncer::~FileChangeDebouncer() {
    Stop();
}

void FileChangeDebouncer::AddEvent(const FileChangeEvent& event) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    std::string key = event.path.string();
    
    // Merge events: if file was modified multiple times, keep the latest
    // If file was removed, that takes precedence
    if (event.type == FileChangeType::Removed) {
        m_PendingChanges[key] = event;
    } else if (m_PendingChanges.find(key) == m_PendingChanges.end() ||
               m_PendingChanges[key].type != FileChangeType::Removed) {
        // Update with latest event (prefer Modified over Added if already exists)
        if (m_PendingChanges.find(key) != m_PendingChanges.end() &&
            m_PendingChanges[key].type == FileChangeType::Added &&
            event.type == FileChangeType::Modified) {
            // Change Added to Modified
            m_PendingChanges[key] = event;
        } else if (m_PendingChanges.find(key) == m_PendingChanges.end()) {
            m_PendingChanges[key] = event;
        }
    }
}

void FileChangeDebouncer::SetBatchCallback(std::function<void(const std::vector<FileChangeEvent>&)> cb) {
    m_BatchCallback = cb;
}

void FileChangeDebouncer::Flush() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    
    if (m_PendingChanges.empty() || !m_BatchCallback) {
        return;
    }
    
    std::vector<FileChangeEvent> events;
    events.reserve(m_PendingChanges.size());
    
    for (auto& [path, event] : m_PendingChanges) {
        events.push_back(event);
    }
    
    m_PendingChanges.clear();
    
    // Release lock before calling callback
    lock.~lock_guard();
    
    if (m_BatchCallback) {
        m_BatchCallback(events);
    }
}

void FileChangeDebouncer::Stop() {
    m_IsRunning = false;
    
    if (m_ProcessingThread.joinable()) {
        m_ProcessingThread.join();
    }
}

void FileChangeDebouncer::ProcessingLoop() {
    while (m_IsRunning.load()) {
        std::this_thread::sleep_for(m_DebounceDelay);
        
        std::lock_guard<std::mutex> lock(m_Mutex);
        
        if (m_PendingChanges.empty() || !m_BatchCallback) {
            continue;
        }
        
        std::vector<FileChangeEvent> events;
        events.reserve(m_PendingChanges.size());
        
        for (auto& [path, event] : m_PendingChanges) {
            events.push_back(event);
        }
        
        m_PendingChanges.clear();
        
        // Release lock before calling callback
        lock.~lock_guard();
        
        if (m_BatchCallback) {
            m_BatchCallback(events);
        }
    }
}

} // namespace LGE



