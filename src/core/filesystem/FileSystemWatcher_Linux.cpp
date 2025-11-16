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

#ifdef __linux__

#include "LGE/core/filesystem/FileSystemWatcher_Linux.h"
#include "LGE/core/Log.h"
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <algorithm>

namespace LGE {

LinuxFileSystemWatcher::LinuxFileSystemWatcher()
    : m_InotifyFd(-1)
    , m_ShouldStop(false)
{
}

LinuxFileSystemWatcher::~LinuxFileSystemWatcher() {
    StopWatching();
}

bool LinuxFileSystemWatcher::StartWatching(const std::filesystem::path& directory) {
    if (m_IsWatching.load()) {
        StopWatching();
    }
    
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        Log::Error("Directory does not exist or is not a directory: " + directory.string());
        return false;
    }
    
    m_WatchedDirectory = directory;
    
    // Initialize inotify
    m_InotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (m_InotifyFd < 0) {
        Log::Error("Failed to initialize inotify");
        return false;
    }
    
    // Add recursive watches
    AddWatchRecursive(directory, -1);
    
    m_ShouldStop = false;
    m_IsWatching = true;
    m_WatchThread = std::thread(&LinuxFileSystemWatcher::WatchThreadFunc, this);
    
    Log::Info("Started watching directory: " + directory.string());
    return true;
}

void LinuxFileSystemWatcher::StopWatching() {
    if (!m_IsWatching.load()) {
        return;
    }
    
    m_ShouldStop = true;
    m_IsWatching = false;
    
    if (m_WatchThread.joinable()) {
        m_WatchThread.join();
    }
    
    // Remove all watches
    for (const auto& [wd, path] : m_WatchDescriptors) {
        inotify_rm_watch(m_InotifyFd, wd);
    }
    m_WatchDescriptors.clear();
    
    if (m_InotifyFd >= 0) {
        close(m_InotifyFd);
        m_InotifyFd = -1;
    }
    
    Log::Info("Stopped watching directory");
}

void LinuxFileSystemWatcher::AddWatchRecursive(const std::filesystem::path& path, int parentWd) {
    int wd = inotify_add_watch(
        m_InotifyFd,
        path.c_str(),
        IN_MODIFY | IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_ATTRIB
    );
    
    if (wd >= 0) {
        m_WatchDescriptors[wd] = path;
    }
    
    // Recursively add watches for subdirectories
    try {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            if (entry.is_directory()) {
                AddWatchRecursive(entry.path(), wd);
            }
        }
    } catch (const std::exception& e) {
        Log::Warn("Error adding recursive watch: " + std::string(e.what()));
    }
}

void LinuxFileSystemWatcher::WatchThreadFunc() {
    const size_t bufferSize = (sizeof(inotify_event) + NAME_MAX + 1) * 1024;
    char buffer[bufferSize];
    
    while (!m_ShouldStop.load()) {
        fd_set readFds;
        FD_ZERO(&readFds);
        FD_SET(m_InotifyFd, &readFds);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int result = select(m_InotifyFd + 1, &readFds, NULL, NULL, &timeout);
        
        if (result > 0 && FD_ISSET(m_InotifyFd, &readFds)) {
            ssize_t length = read(m_InotifyFd, buffer, bufferSize);
            
            if (length > 0) {
                size_t i = 0;
                while (i < length) {
                    inotify_event* event = reinterpret_cast<inotify_event*>(&buffer[i]);
                    
                    auto it = m_WatchDescriptors.find(event->wd);
                    if (it != m_WatchDescriptors.end()) {
                        std::filesystem::path fullPath = it->second;
                        if (event->len > 0) {
                            fullPath /= std::string(event->name);
                        }
                        
                        FileChangeEvent changeEvent;
                        changeEvent.path = fullPath;
                        changeEvent.timestamp = std::time(nullptr);
                        
                        if (event->mask & IN_CREATE) {
                            changeEvent.type = FileChangeType::Added;
                            if (m_Callback) {
                                m_Callback(changeEvent);
                            }
                            
                            // If it's a directory, add watch for it
                            if (event->mask & IN_ISDIR) {
                                AddWatchRecursive(fullPath, event->wd);
                            }
                        } else if (event->mask & IN_DELETE) {
                            changeEvent.type = FileChangeType::Removed;
                            if (m_Callback) {
                                m_Callback(changeEvent);
                            }
                        } else if (event->mask & IN_MODIFY || event->mask & IN_ATTRIB) {
                            changeEvent.type = FileChangeType::Modified;
                            if (m_Callback) {
                                m_Callback(changeEvent);
                            }
                        } else if (event->mask & IN_MOVED_FROM) {
                            changeEvent.type = FileChangeType::Renamed;
                            changeEvent.oldPath = fullPath;
                            // Store for pairing with MOVED_TO
                        } else if (event->mask & IN_MOVED_TO) {
                            changeEvent.type = FileChangeType::Renamed;
                            if (m_Callback) {
                                m_Callback(changeEvent);
                            }
                        }
                    }
                    
                    i += sizeof(inotify_event) + event->len;
                }
            }
        }
    }
}

} // namespace LGE

#endif // __linux__



