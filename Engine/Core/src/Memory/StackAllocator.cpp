#include "Core/Memory/StackAllocator.h"
#include "Core/Assert.h"
#include <cstdlib>

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

            mMemory = std::malloc(size);

            CORE_VERIFY(mMemory, "Failed to allocate memory for StackAllocator");
        }

        StackAllocator::~StackAllocator()
        {
            if (mMemory)
            {
                std::free(mMemory);
                mMemory = nullptr;
            }
        }

        void* StackAllocator::Allocate(size_t size, size_t alignment)
        {
            CORE_ASSERT(size > 0, "Allocation size must be greater than 0");
            CORE_ASSERT((alignment & (alignment - 1)) == 0, "Alignment must be power of 2");

            // Calculate aligned data position
            void* currentPtr = static_cast<char*>(mMemory) + mOffset;
            void* alignedPtr = AlignPointer(currentPtr, alignment);

            // Calculate padding needed for alignment
            size_t padding = static_cast<char*>(alignedPtr) - static_cast<char*>(currentPtr);
            size_t totalSize = padding + size;

            // Check if we have enough space
            if (mOffset + totalSize > mSize)
            {
                CORE_ASSERT(false, "StackAllocator out of memory");
                return nullptr;
            }

            // Update offset and count
            mOffset += totalSize;
            mAllocationCount++;

            return alignedPtr;
        }

        void StackAllocator::Deallocate(void* ptr)
        {
            // Stack allocator cannot free individual allocations
            // Use FreeToMarker() or Reset()
            (void)ptr;  // Unused
        }

        void StackAllocator::Reset()
        {
            mOffset = 0;
            mAllocationCount = 0;
        }

        void StackAllocator::FreeToMarker(Marker marker)
        {
            CORE_ASSERT(marker <= mOffset, "Invalid marker - cannot free to future position");

            if (marker < mOffset)
            {
                mOffset = marker;

                // NOTE: Cannot accurately track allocation count with markers
                // Reset to 0 as approximation
                if (marker == 0)
                {
                    mAllocationCount = 0;
                }
            }
        }

    } // namespace Memory
} // namespace Core