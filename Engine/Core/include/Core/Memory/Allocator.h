#pragma once

#include "Core/Types.h"
#include "Core/Memory/MemoryConfig.h"

namespace Core
{
    namespace Memory
    {
        /**
         * @brief Base allocator interface
         * All custom allocators inherit from this.
         */
        class Allocator
        {
        public:
            virtual ~Allocator() = default;

            /**
             * @brief Allocate memory
             * @return Pointer to allocated memory, or nullptr on failure
             */
            virtual void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) = 0;

            /**
             * @brief Deallocate memory
             */
            virtual void Deallocate(void* ptr) = 0;

            virtual size_t GetAllocatedSize() const = 0;
            virtual size_t GetAllocationCount() const = 0;

            /**
             * @brief Reset allocator (if supported)
             */
            virtual void Reset() {}

        protected:
            Allocator() = default;

            // Non-copyable
            Allocator(const Allocator&) = delete;
            Allocator& operator=(const Allocator&) = delete;
        };

    } // namespace Memory
} // namespace Core