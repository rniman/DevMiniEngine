#include "Core/Memory/LinearAllocator.h"
#include "Core/Assert.h"
#include <cstdlib>
#include <cassert>

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
            mMemory = std::malloc(size);
            
            CORE_VERIFY(mMemory, "Failed to allocate memory for LinearAllocator");
        }

        LinearAllocator::~LinearAllocator()
        {
            if (mMemory)
            {
                std::free(mMemory);
                mMemory = nullptr;
            }
        }

        void* LinearAllocator::Allocate(size_t size, size_t alignment)
        {
            CORE_ASSERT(size > 0, "Allocation size must be greater than 0");
            CORE_ASSERT((alignment & (alignment - 1)) == 0, "Alignment must be power of 2");

            // Calculate aligned offset
            void* currentPtr = static_cast<char*>(mMemory) + mOffset;
            void* alignedPtr = AlignPointer(currentPtr, alignment);
            size_t padding = static_cast<char*>(alignedPtr) - static_cast<char*>(currentPtr);

            // Check if we have enough space
            if (mOffset + padding + size > mSize)
            {
                // Out of memory
                CORE_ASSERT(false, "LinearAllocator out of memory");
                return nullptr;
            }

            // Update offset and return pointer
            mOffset += padding + size;
            mAllocationCount++;

            return alignedPtr;
        }

        void LinearAllocator::Deallocate(void* ptr)
        {
            // Linear allocator cannot free individual allocations
            // Use Reset() to free all memory at once
            (void)ptr;  // Unused
        }

        void LinearAllocator::Reset()
        {
            mOffset = 0;
            mAllocationCount = 0;
        }

    } // namespace Memory
} // namespace Core