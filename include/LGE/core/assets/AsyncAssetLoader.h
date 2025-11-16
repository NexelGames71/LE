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

#include "LGE/core/GUID.h"
#include "LGE/core/assets/AssetMetadata.h"
#include <memory>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <atomic>

namespace LGE {

// Forward declaration
class AssetLoader;

class AsyncAssetLoader {
private:
    struct LoadRequest {
        GUID guid;
        int priority;
        std::promise<std::shared_ptr<void>> promise;
        AssetType type;
        
        LoadRequest() : priority(0), type(AssetType::Unknown) {}
    };
    
    // Comparator for priority queue (higher priority first)
    struct LoadRequestComparator {
        bool operator()(const LoadRequest& a, const LoadRequest& b) const {
            return a.priority < b.priority; // Lower priority value = higher priority
        }
    };
    
    AssetLoader* m_SyncLoader;
    
    std::priority_queue<LoadRequest, std::vector<LoadRequest>, LoadRequestComparator> m_LoadQueue;
    mutable std::mutex m_QueueMutex;
    std::condition_variable m_QueueCV;
    
    std::vector<std::thread> m_WorkerThreads;
    std::atomic<bool> m_IsRunning{false};
    
    void WorkerThreadFunc();

public:
    AsyncAssetLoader(AssetLoader* loader, int numThreads = 4);
    ~AsyncAssetLoader();
    
    // Load asset asynchronously
    template<typename T>
    std::future<std::shared_ptr<T>> LoadAsync(const GUID& guid, int priority = 0) {
        if (!m_SyncLoader || !m_SyncLoader->GetRegistry()) {
            std::promise<std::shared_ptr<T>> promise;
            promise.set_value(nullptr);
            return promise.get_future();
        }
        
        AssetMetadata* metadata = m_SyncLoader->GetRegistry()->GetAsset(guid);
        if (!metadata) {
            std::promise<std::shared_ptr<T>> promise;
            promise.set_value(nullptr);
            return promise.get_future();
        }
        
        LoadRequest request;
        request.guid = guid;
        request.priority = priority;
        request.type = metadata->type;
        
        auto future = request.promise.get_future();
        
        {
            std::lock_guard<std::mutex> lock(m_QueueMutex);
            m_LoadQueue.push(std::move(request));
        }
        m_QueueCV.notify_one();
        
        // Return a future that wraps the promise future
        return std::async(std::launch::deferred, [future = std::move(future)]() mutable {
            auto result = future.get();
            return std::static_pointer_cast<T>(result);
        });
    }
    
    // Cancel pending loads
    void CancelPending();
    
    // Get queue size
    size_t GetQueueSize() const;
    
    // Check if running
    bool IsRunning() const { return m_IsRunning; }
    
    // Stop worker threads
    void Stop();
};

} // namespace LGE

