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

#include "LGE/core/assets/AssetMetadataPool.h"
#include "LGE/core/Log.h"
#include <algorithm>

namespace LGE {

AssetMetadataPool::AssetMetadataPool() {
    // Allocate initial chunk
    m_Chunks.push_back(std::make_unique<Chunk>());
}

AssetMetadataPool::~AssetMetadataPool() {
    Clear();
}

AssetMetadata* AssetMetadataPool::Allocate() {
    std::lock_guard<std::mutex> lock(m_PoolMutex);
    
    // Find a free slot in existing chunks
    for (auto& chunk : m_Chunks) {
        for (size_t i = 0; i < CHUNK_SIZE; ++i) {
            if (!chunk->used[i]) {
                chunk->used[i] = true;
                chunk->freeCount--;
                return &chunk->items[i];
            }
        }
    }
    
    // All chunks full, allocate new chunk
    auto newChunk = std::make_unique<Chunk>();
    newChunk->used[0] = true;
    newChunk->freeCount--;
    AssetMetadata* ptr = &newChunk->items[0];
    m_Chunks.push_back(std::move(newChunk));
    
    return ptr;
}

void AssetMetadataPool::Deallocate(AssetMetadata* ptr) {
    if (!ptr) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_PoolMutex);
    
    // Find which chunk this pointer belongs to
    for (auto& chunk : m_Chunks) {
        if (ptr >= &chunk->items[0] && ptr < &chunk->items[CHUNK_SIZE]) {
            size_t index = ptr - &chunk->items[0];
            if (chunk->used[index]) {
                chunk->used[index] = false;
                chunk->freeCount++;
                // Reset the metadata
                chunk->items[index] = AssetMetadata();
            }
            return;
        }
    }
    
    Log::Warn("Attempted to deallocate pointer not in pool");
}

size_t AssetMetadataPool::GetTotalAllocated() const {
    std::lock_guard<std::mutex> lock(m_PoolMutex);
    size_t total = 0;
    for (const auto& chunk : m_Chunks) {
        total += (CHUNK_SIZE - chunk->freeCount);
    }
    return total;
}

size_t AssetMetadataPool::GetTotalCapacity() const {
    std::lock_guard<std::mutex> lock(m_PoolMutex);
    return m_Chunks.size() * CHUNK_SIZE;
}

void AssetMetadataPool::Clear() {
    std::lock_guard<std::mutex> lock(m_PoolMutex);
    for (auto& chunk : m_Chunks) {
        chunk->used.reset();
        chunk->freeCount = CHUNK_SIZE;
        // Reset all items
        for (auto& item : chunk->items) {
            item = AssetMetadata();
        }
    }
}

} // namespace LGE

