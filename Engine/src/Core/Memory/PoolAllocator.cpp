#include "pch.h"
#include "Core/Memory/PoolAllocator.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h"

using namespace std;

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

            // Free-list 구현을 위해 각 청크가 최소한 포인터를 저장할 수 있어야 함
            mChunkSize = AlignSize(chunkSize, sizeof(void*));

            const size_t totalSize = mChunkSize * mChunkCount;
            mMemory = malloc(totalSize);

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

                free(mMemory);
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

            CORE_ASSERT(
                ptr >= mMemory && ptr < static_cast<char*>(mMemory) + (mChunkSize * mChunkCount),
                "Pointer does not belong to this pool"
            );

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
            // Free-list는 사용 가능한 청크들을 연결 리스트로 관리
            // 각 청크의 시작 부분에 다음 청크의 포인터를 저장
            mFreeList = mMemory;

            // 각 청크를 다음 청크에 연결하여 free-list 구성
            char* current = static_cast<char*>(mMemory);
            for (size_t i = 0; i < mChunkCount - 1; ++i)
            {
                void** const currentAsPtr = reinterpret_cast<void**>(current);
                *currentAsPtr = current + mChunkSize;
                current += mChunkSize;
            }

            // 마지막 청크는 nullptr로 리스트 종료
            void** const lastChunk = reinterpret_cast<void**>(current);
            *lastChunk = nullptr;
        }

    } // namespace Memory
} // namespace Core
