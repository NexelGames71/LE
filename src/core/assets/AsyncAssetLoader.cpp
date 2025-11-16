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

#include "LGE/core/assets/AsyncAssetLoader.h"
#include "LGE/core/assets/AssetLoader.h"
#include "LGE/core/Log.h"

namespace LGE {

AsyncAssetLoader::AsyncAssetLoader(AssetLoader* loader, int numThreads)
    : m_SyncLoader(loader)
{
    m_IsRunning = true;
    
    // Create worker threads
    for (int i = 0; i < numThreads; ++i) {
        m_WorkerThreads.emplace_back(&AsyncAssetLoader::WorkerThreadFunc, this);
    }
    
    Log::Info("AsyncAssetLoader started with " + std::to_string(numThreads) + " worker threads");
}

AsyncAssetLoader::~AsyncAssetLoader() {
    Stop();
}

void AsyncAssetLoader::WorkerThreadFunc() {
    while (m_IsRunning) {
        LoadRequest request;
        bool hasRequest = false;
        
        {
            std::unique_lock<std::mutex> lock(m_QueueMutex);
            m_QueueCV.wait(lock, [this] { 
                return !m_LoadQueue.empty() || !m_IsRunning; 
            });
            
            if (!m_IsRunning && m_LoadQueue.empty()) {
                break;
            }
            
            if (!m_LoadQueue.empty()) {
                request = std::move(const_cast<LoadRequest&>(m_LoadQueue.top()));
                m_LoadQueue.pop();
                hasRequest = true;
            }
        }
        
        if (hasRequest) {
            // Load the asset using sync loader
            std::shared_ptr<void> asset = m_SyncLoader->LoadGeneric(request.guid, request.type);
            request.promise.set_value(asset);
        }
    }
}

void AsyncAssetLoader::CancelPending() {
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    
    while (!m_LoadQueue.empty()) {
        LoadRequest request = std::move(const_cast<LoadRequest&>(m_LoadQueue.top()));
        m_LoadQueue.pop();
        request.promise.set_value(nullptr);
    }
    
    Log::Info("Cancelled all pending asset loads");
}

size_t AsyncAssetLoader::GetQueueSize() const {
    std::lock_guard<std::mutex> lock(m_QueueMutex);
    return m_LoadQueue.size();
}

void AsyncAssetLoader::Stop() {
    if (!m_IsRunning) {
        return;
    }
    
    m_IsRunning = false;
    m_QueueCV.notify_all();
    
    // Wait for all threads to finish
    for (auto& thread : m_WorkerThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    m_WorkerThreads.clear();
    Log::Info("AsyncAssetLoader stopped");
}

} // namespace LGE

