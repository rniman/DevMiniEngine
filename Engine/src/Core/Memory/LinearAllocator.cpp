#include "pch.h"
#include "Core/Memory/LinearAllocator.h"
#include "Core/Assert.h"
#include "Core/Logging/LogMacros.h" 

using namespace std;

namespace Core
{
    namespace Memory
    {
        LinearAllocator::LinearAllocator(size_t size)
            : mMemory(nullptr)
            , mSize(size)
            , mOffset(0)
            , mAllocationCount(0)
        {
            CORE_VERIFY(size > 0, "LinearAllocator size must be greater than 0");

            // Allocate aligned memory
            mMemory = malloc(size);
            
            CORE_VERIFY(mMemory, "Failed to allocate memory for LinearAllocator");

            LOG_TRACE("LinearAllocator created: %zu bytes", size);
        }

        LinearAllocator::~LinearAllocator()
        {
            if (mMemory)
            {
                LOG_TRACE("LinearAllocator destroyed: %zu bytes allocated, %zu allocations", mOffset, mAllocationCount);
                free(mMemory);
                mMemory = nullptr;
            }
        }

        void* LinearAllocator::Allocate(size_t size, size_t alignment)
        {
            CORE_ASSERT(size > 0, "Allocation size must be greater than 0");
            CORE_ASSERT((alignment & (alignment - 1)) == 0, "Alignment must be power of 2");

            void* const currentPtr = static_cast<char*>(mMemory) + mOffset;
            void* const alignedPtr = AlignPointer(currentPtr, alignment);
            const size_t padding = static_cast<char*>(alignedPtr) - static_cast<char*>(currentPtr);
            
            if (mOffset + padding + size > mSize)
            {
                LOG_ERROR("LinearAllocator out of memory: requested %zu bytes, available %zu bytes", size, mSize - mOffset);
                CORE_ASSERT(false, "LinearAllocator out of memory");
                return nullptr;
            }
            
            mOffset += padding + size;
            mAllocationCount++;

            return alignedPtr;
        }

        void LinearAllocator::Deallocate(void* ptr)
        {
            // LinearAllocator는 개별 해제 불가 - Reset()으로 전체 해제만 가능
            (void)ptr;
        }

        void LinearAllocator::Reset()
        {
            LOG_TRACE("LinearAllocator reset: freed %zu bytes, %zu allocations", mOffset, mAllocationCount);
            mOffset = 0;
            mAllocationCount = 0;
        }

    } // namespace Memory
} // namespace Core