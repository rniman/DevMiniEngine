#include "pch.h"
#include "Core/Memory/StackAllocator.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h" 

using namespace std;

namespace Core
{
    namespace Memory
    {
        StackAllocator::StackAllocator(size_t size)
            : mMemory(nullptr)
            , mSize(size)
            , mOffset(0)
            , mAllocationCount(0)
        {
            CORE_VERIFY(size > 0, "StackAllocator size must be greater than 0");

            mMemory = malloc(size);

            CORE_VERIFY(mMemory, "Failed to allocate memory for StackAllocator");
         
            LOG_TRACE("StackAllocator created: %zu bytes", size);
        }

        StackAllocator::~StackAllocator()
        {
            if (mMemory)
            {
                LOG_TRACE("StackAllocator destroyed: %zu bytes peak, %zu allocations", mOffset, mAllocationCount);
                free(mMemory);
                mMemory = nullptr;
            }
        }

        void* StackAllocator::Allocate(size_t size, size_t alignment)
        {
            CORE_ASSERT(size > 0, "Allocation size must be greater than 0");
            CORE_ASSERT((alignment & (alignment - 1)) == 0, "Alignment must be power of 2");

            char* const currentPtr = static_cast<char*>(mMemory) + mOffset;
            void* const alignedPtr = AlignPointer(currentPtr, alignment);

            const size_t padding = static_cast<char*>(alignedPtr) - currentPtr;
            const size_t totalSize = padding + size;

            if (mOffset + totalSize > mSize)
            {
                LOG_ERROR("StackAllocator out of memory: requested %zu bytes, available %zu bytes", size, mSize - mOffset);
                CORE_ASSERT(false, "StackAllocator out of memory");
                return nullptr;
            }

            mOffset += totalSize;
            mAllocationCount++;

            return alignedPtr;
        }

        void StackAllocator::Deallocate(void* ptr)
        {
            // StackAllocator는 LIFO 패턴만 지원 - 개별 해제 불가
            // 특정 지점까지 해제: FreeToMarker(), 전체 해제: Reset()
            (void)ptr;
        }

        void StackAllocator::Reset()
        {
            LOG_TRACE("LinearAllocator reset: freed %zu bytes, %zu allocations", mOffset, mAllocationCount);
            mOffset = 0;
            mAllocationCount = 0;
        }

        void StackAllocator::FreeToMarker(Marker marker)
        {
            CORE_ASSERT(marker <= mOffset, "Invalid marker - cannot free to future position");

            if (marker < mOffset)
            {
                const size_t freedBytes = mOffset - marker;
                LOG_TRACE("StackAllocator freed to marker: %zu bytes freed", freedBytes);
                mOffset = marker;

                // NOTE: 마커 기반 해제에서는 정확한 할당 카운트 추적 불가능
                // Reset()에서만 카운트를 0으로 초기화
            }
        }

    } // namespace Memory
} // namespace Core