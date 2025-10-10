#pragma once

#include "Core/Memory/Allocator.h"

namespace Core
{
    namespace Memory
    {
        /**
         * @brief Linear (Arena) Allocator
         *
         * Fast allocator that bumps a pointer forward.
         * Cannot free individual allocations - must reset entire allocator.
         * Ideal for per-frame temporary allocations where all memory
         * can be freed at once.
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
            /**
             * @brief Constructor
             * @param size Total size of the allocator in bytes
             */
            explicit LinearAllocator(size_t size);

            /**
             * @brief Destructor - frees the underlying memory
             */
            ~LinearAllocator() override;

            // Non-copyable
            LinearAllocator(const LinearAllocator&) = delete;
            LinearAllocator& operator=(const LinearAllocator&) = delete;

            /**
             * @brief Allocate memory from the linear buffer
             * @param size Size in bytes
             * @param alignment Memory alignment
             * @return Pointer to allocated memory, or nullptr if out of space
             */
            void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;

            /**
             * @brief No-op for linear allocator
             * @note LinearAllocator cannot free individual allocations
             */
            void Deallocate(void* ptr) override;

            /**
             * @brief Reset the allocator to initial state
             * @note This frees all allocations at once
             */
            void Reset() override;

            /**
             * @brief Get current allocated size
             */
            size_t GetAllocatedSize() const override { return mOffset; }

            /**
             * @brief Get allocation count
             */
            size_t GetAllocationCount() const override { return mAllocationCount; }

            /**
             * @brief Get total capacity
             */
            size_t GetCapacity() const { return mSize; }

            /**
             * @brief Get remaining free space
             */
            size_t GetFreeSpace() const { return mSize - mOffset; }

        private:
            void* mMemory;              // Start of memory block
            size_t mSize;               // Total size
            size_t mOffset;             // Current offset
            size_t mAllocationCount;    // Number of allocations
        };

    } // namespace Memory
} // namespace Core