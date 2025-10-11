#include "Core/Memory/PoolAllocator.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"
#include <cstdlib>

namespace Core
{
    namespace Memory
    {
        PoolAllocator::PoolAllocator(size_t chunkSize, size_t chunkCount)
            : mMemory(nullptr)
            , mChunkSize(chunkSize)
            , mChunkCount(chunkCount)
            , mAllocatedChunks(0)
            , mFreeList(nullptr)
        {
            CORE_VERIFY(chunkSize >= sizeof(void*), "Chunk size must be at least pointer size");
            CORE_VERIFY(chunkCount > 0, "Chunk count must be greater than 0");

            // Align chunk size to pointer size for free-list
            mChunkSize = AlignSize(chunkSize, sizeof(void*));

            // Allocate memory block
            const size_t totalSize = mChunkSize * mChunkCount;
            mMemory = std::malloc(totalSize);

            CORE_VERIFY(mMemory, "Failed to allocate memory for PoolAllocator");

            InitializeFreeList();

            LOG_TRACE("PoolAllocator created: %zu chunks x %zu bytes = %zu total", mChunkCount, mChunkSize, totalSize);
        }

        PoolAllocator::~PoolAllocator()
        {
            if (mMemory)
            {
                if (mAllocatedChunks > 0)
                {
                    LOG_WARN("PoolAllocator destroyed with %zu chunks still allocated (potential leak)", mAllocatedChunks);
                }
                else
                {
                    LOG_TRACE("PoolAllocator destroyed cleanly");
                }

                std::free(mMemory);
                mMemory = nullptr;
            }
        }

        void* PoolAllocator::Allocate(size_t size, size_t alignment)
        {
            CORE_ASSERT(size <= mChunkSize, "Requested size exceeds chunk size");
            CORE_ASSERT(alignment <= sizeof(void*), "PoolAllocator supports alignment up to pointer size");

            if (mFreeList == nullptr)
            {
                LOG_ERROR("PoolAllocator out of memory: all %zu chunks allocated", mChunkCount);
                CORE_ASSERT(false, "PoolAllocator out of memory");
                return nullptr;
            }

            // Pop from free-list
            void* chunk = mFreeList;
            mFreeList = *reinterpret_cast<void**>(mFreeList);
            mAllocatedChunks++;

            return chunk;
        }

        void PoolAllocator::Deallocate(void* ptr)
        {
            if (ptr == nullptr)
            {
                return;
            }

            CORE_ASSERT(ptr >= mMemory && ptr < static_cast<char*>(mMemory) + (mChunkSize * mChunkCount),
                "Pointer does not belong to this pool");

            // Push to free-list
            *reinterpret_cast<void**>(ptr) = mFreeList;
            mFreeList = ptr;
            mAllocatedChunks--;
        }

        void PoolAllocator::Reset()
        {
            mAllocatedChunks = 0;
            InitializeFreeList();
        }

        void PoolAllocator::InitializeFreeList()
        {
            mFreeList = mMemory;

            // Link all chunks in free-list
            char* current = static_cast<char*>(mMemory);
            for (size_t i = 0; i < mChunkCount - 1; ++i)
            {
                void** currentAsPtr = reinterpret_cast<void**>(current);
                *currentAsPtr = current + mChunkSize;
                current += mChunkSize;
            }

            // Last chunk points to nullptr
            void** lastChunk = reinterpret_cast<void**>(current);
            *lastChunk = nullptr;
        }

    } // namespace Memory
} // namespace Core