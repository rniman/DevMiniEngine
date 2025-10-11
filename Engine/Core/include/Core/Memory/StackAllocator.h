#pragma once

#include "Core/Memory/Allocator.h"

namespace Core
{
    namespace Memory
    {
        /**
         * @brief Stack allocator for scoped allocations
         *
         * LIFO (Last In First Out) allocator for nested scope patterns.
         * Supports markers for partial deallocation.
         *
         * Usage:
         *   StackAllocator stack(1 * MB);
         *   auto marker = stack.GetMarker();
         *   void* temp = stack.Allocate(1024);
         *   // ... use memory
         *   stack.FreeToMarker(marker);  // Free all allocations after marker
         */
        class StackAllocator : public Allocator
        {
        public:
            // Marker for stack position
            using Marker = size_t;

            explicit StackAllocator(size_t size);
            ~StackAllocator() override;

            // Non-copyable
            StackAllocator(const StackAllocator&) = delete;
            StackAllocator& operator=(const StackAllocator&) = delete;

            void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;

            /**
             * @brief No-op for stack allocator
             * @note Use FreeToMarker() or Reset() instead
             */
            void Deallocate(void* ptr) override;

            void Reset() override;

            /**
             * @brief Get current stack position
             * @return Marker for current position
             */
            Marker GetMarker() const { return mOffset; }

            /**
             * @brief Free all allocations after marker
             * @note Must free in reverse order (LIFO)
             */
            void FreeToMarker(Marker marker);

            size_t GetAllocatedSize() const override { return mOffset; }
            size_t GetAllocationCount() const override { return mAllocationCount; }
            size_t GetCapacity() const { return mSize; }
            size_t GetFreeSpace() const { return mSize - mOffset; }

        private:
            void* mMemory;              // Start of memory block
            size_t mSize;               // Total size
            size_t mOffset;             // Current offset
            size_t mAllocationCount;    // Number of allocations
        };

    } // namespace Memory
} // namespace Core