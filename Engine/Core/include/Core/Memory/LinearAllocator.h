#pragma once

#include "Core/Memory/Allocator.h"

namespace Core
{
    namespace Memory
    {
        /**
         * @brief Linear (Arena) Allocator
         *
         * Fast bump-pointer allocator for per-frame temporary data.
         * Cannot free individual allocations - must reset entire allocator.
         *
         * Usage:
         *   LinearAllocator frameAlloc(10 * MB);
         *   void* memory = frameAlloc.Allocate(1024);
         *   // ... use memory
         *   frameAlloc.Reset();  // Free all at once
         */
        class LinearAllocator : public Allocator
        {
        public:
            explicit LinearAllocator(size_t size);
            ~LinearAllocator() override;

            // Non-copyable
            LinearAllocator(const LinearAllocator&) = delete;
            LinearAllocator& operator=(const LinearAllocator&) = delete;

            /**
             * @brief Allocate memory from the linear buffer
             * @return Pointer to allocated memory, or nullptr if out of space
             */
            void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;

            /**
             * @brief No-op for linear allocator
             * @note Cannot free individual allocations
             */
            void Deallocate(void* ptr) override;

            /**
             * @brief Reset the allocator
             * @note Frees all allocations at once
             */
            void Reset() override;

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