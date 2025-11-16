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

#ifdef _WIN32

#include "LGE/core/filesystem/FileSystemWatcher_Windows.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

WindowsFileSystemWatcher::WindowsFileSystemWatcher()
    : m_DirectoryHandle(INVALID_HANDLE_VALUE)
    , m_StopEvent(INVALID_HANDLE_VALUE)
{
}

WindowsFileSystemWatcher::~WindowsFileSystemWatcher() {
    StopWatching();
}

bool WindowsFileSystemWatcher::StartWatching(const std::filesystem::path& directory) {
    if (m_IsWatching.load()) {
        StopWatching();
    }
    
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        Log::Error("Directory does not exist or is not a directory: " + directory.string());
        return false;
    }
    
    m_WatchedDirectory = directory;
    
    // Create stop event
    m_StopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (m_StopEvent == NULL) {
        Log::Error("Failed to create stop event");
        return false;
    }
    
    // Open directory handle
    std::wstring dirPath = directory.wstring();
    m_DirectoryHandle = CreateFileW(
        dirPath.c_str(),
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );
    
    if (m_DirectoryHandle == INVALID_HANDLE_VALUE) {
        Log::Error("Failed to open directory for watching: " + directory.string());
        CloseHandle(m_StopEvent);
        m_StopEvent = INVALID_HANDLE_VALUE;
        return false;
    }
    
    m_IsWatching = true;
    m_WatchThread = std::thread(&WindowsFileSystemWatcher::WatchThreadFunc, this);
    
    Log::Info("Started watching directory: " + directory.string());
    return true;
}

void WindowsFileSystemWatcher::StopWatching() {
    if (!m_IsWatching.load()) {
        return;
    }
    
    m_IsWatching = false;
    
    // Signal stop event
    if (m_StopEvent != INVALID_HANDLE_VALUE) {
        SetEvent(m_StopEvent);
    }
    
    // Wait for thread to finish
    if (m_WatchThread.joinable()) {
        m_WatchThread.join();
    }
    
    // Close handles
    if (m_DirectoryHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(m_DirectoryHandle);
        m_DirectoryHandle = INVALID_HANDLE_VALUE;
    }
    
    if (m_StopEvent != INVALID_HANDLE_VALUE) {
        CloseHandle(m_StopEvent);
        m_StopEvent = INVALID_HANDLE_VALUE;
    }
    
    Log::Info("Stopped watching directory");
}

void WindowsFileSystemWatcher::WatchThreadFunc() {
    const DWORD bufferSize = 4096;
    char buffer[bufferSize];
    DWORD bytesReturned;
    
    OVERLAPPED overlapped = {0};
    overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    
    while (m_IsWatching.load()) {
        // Reset event
        ResetEvent(overlapped.hEvent);
        
        // Start async read
        BOOL success = ReadDirectoryChangesW(
            m_DirectoryHandle,
            buffer,
            bufferSize,
            TRUE, // Watch subdirectories
            FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | 
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE,
            &bytesReturned,
            &overlapped,
            NULL
        );
        
        if (!success) {
            DWORD error = GetLastError();
            if (error != ERROR_IO_PENDING) {
                Log::Error("ReadDirectoryChangesW failed with error: " + std::to_string(error));
                break;
            }
        }
        
        // Wait for either change notification or stop event
        HANDLE handles[] = {overlapped.hEvent, m_StopEvent};
        DWORD waitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);
        
        if (waitResult == WAIT_OBJECT_0) {
            // Get overlapped result
            if (GetOverlappedResult(m_DirectoryHandle, &overlapped, &bytesReturned, FALSE)) {
                // Process notifications
                FILE_NOTIFY_INFORMATION* notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(buffer);
                
                do {
                    std::wstring fileName(notifyInfo->FileName, notifyInfo->FileNameLength / sizeof(WCHAR));
                    std::filesystem::path fullPath = m_WatchedDirectory / fileName;
                    
                    FileChangeEvent event;
                    event.path = fullPath;
                    event.timestamp = std::time(nullptr);
                    
                    switch (notifyInfo->Action) {
                        case FILE_ACTION_ADDED:
                            event.type = FileChangeType::Added;
                            break;
                        case FILE_ACTION_REMOVED:
                            event.type = FileChangeType::Removed;
                            break;
                        case FILE_ACTION_MODIFIED:
                            event.type = FileChangeType::Modified;
                            break;
                        case FILE_ACTION_RENAMED_OLD_NAME:
                            event.type = FileChangeType::Renamed;
                            event.oldPath = fullPath;
                            // Wait for new name
                            break;
                        case FILE_ACTION_RENAMED_NEW_NAME:
                            event.type = FileChangeType::Renamed;
                            // This should have been paired with OLD_NAME
                            break;
                    }
                    
                    if (m_Callback) {
                        m_Callback(event);
                    }
                    
                    // Move to next notification
                    if (notifyInfo->NextEntryOffset == 0) {
                        break;
                    }
                    notifyInfo = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(
                        reinterpret_cast<BYTE*>(notifyInfo) + notifyInfo->NextEntryOffset);
                } while (true);
            }
        } else if (waitResult == WAIT_OBJECT_0 + 1) {
            // Stop event signaled
            break;
        }
    }
    
    CloseHandle(overlapped.hEvent);
}

} // namespace LGE

#endif // _WIN32



