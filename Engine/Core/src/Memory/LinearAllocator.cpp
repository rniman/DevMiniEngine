#include "Core/Memory/LinearAllocator.h"
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
            assert(size > 0 && "LinearAllocator size must be greater than 0");

            // Allocate aligned memory
            mMemory = std::malloc(size);
            assert(mMemory && "Failed to allocate memory for LinearAllocator");
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
            assert(size > 0 && "Allocation size must be greater than 0");
            assert((alignment & (alignment - 1)) == 0 && "Alignment must be power of 2");

            // Calculate aligned offset
            size_t currentAddr = reinterpret_cast<size_t>(mMemory) + mOffset;
            size_t alignedAddr = (currentAddr + alignment - 1) & ~(alignment - 1);
            size_t padding = alignedAddr - currentAddr;

            // Check if we have enough space
            if (mOffset + padding + size > mSize)
            {
                // Out of memory
                assert(false && "LinearAllocator out of memory");
                return nullptr;
            }

            // Update offset and return pointer
            mOffset += padding + size;
            mAllocationCount++;

            return reinterpret_cast<void*>(alignedAddr);
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