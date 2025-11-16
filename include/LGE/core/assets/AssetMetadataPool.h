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

#include "LGE/core/assets/AssetMetadata.h"
#include <array>
#include <bitset>
#include <vector>
#include <memory>
#include <mutex>

namespace LGE {

class AssetMetadataPool {
private:
    static constexpr size_t POOL_SIZE = 1024;
    static constexpr size_t CHUNK_SIZE = 64;
    
    struct Chunk {
        std::array<AssetMetadata, CHUNK_SIZE> items;
        std::bitset<CHUNK_SIZE> used;
        size_t freeCount = CHUNK_SIZE;
        
        Chunk() : freeCount(CHUNK_SIZE) {
            used.reset();
        }
    };
    
    std::vector<std::unique_ptr<Chunk>> m_Chunks;
    mutable std::mutex m_PoolMutex;

public:
    AssetMetadataPool();
    ~AssetMetadataPool();
    
    AssetMetadata* Allocate();
    void Deallocate(AssetMetadata* ptr);
    
    size_t GetTotalAllocated() const;
    size_t GetTotalCapacity() const;
    
    void Clear();
};

} // namespace LGE

