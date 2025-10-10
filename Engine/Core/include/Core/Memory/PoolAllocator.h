#pragma once

#include "Core/Memory/Allocator.h"
#include <vector>

namespace Core
{
    namespace Memory
    {
        /**
         * @brief Pool allocator for fixed-size objects
         *
         * O(1) allocation and deallocation using free-list.
         * All chunks are same size, ideal for frequent alloc/free patterns.
         *
         * Usage:
         *   PoolAllocator pool(sizeof(MyObject), 100);
         *   void* obj = pool.Allocate();
         *   // ... use object
         *   pool.Deallocate(obj);
         */
        class PoolAllocator : public Allocator
        {
        public:
            PoolAllocator(size_t chunkSize, size_t chunkCount);
            ~PoolAllocator() override;

            // Non-copyable
            PoolAllocator(const PoolAllocator&) = delete;
            PoolAllocator& operator=(const PoolAllocator&) = delete;

            void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;
            void Deallocate(void* ptr) override;

            void Reset() override;

            size_t GetAllocatedSize() const override { return mChunkSize * mAllocatedChunks; }
            size_t GetAllocationCount() const override { return mAllocatedChunks; }
            size_t GetChunkSize() const { return mChunkSize; }
            size_t GetChunkCount() const { return mChunkCount; }
            size_t GetFreeChunkCount() const { return mChunkCount - mAllocatedChunks; }

        private:
            void InitializeFreeList();

            void* mMemory;              // Start of memory block
            size_t mChunkSize;          // Size of each chunk
            size_t mChunkCount;         // Total number of chunks
            size_t mAllocatedChunks;    // Currently allocated chunks
            void* mFreeList;            // Head of free-list
        };

    } // namespace Memory
} // namespace Core