#pragma once

#include "Core/Types.h"

namespace Core
{
    namespace Memory
    {
        //=============================================================================
        // Memory Configuration
        //=============================================================================

        // Default alignments for memory allocations
        constexpr size_t DEFAULT_ALIGNMENT = 16;      // SIMD optimization
        constexpr size_t CACHE_LINE_SIZE = 64;        // CPU cache line

        // Standard memory size units
        constexpr size_t KB = 1024;
        constexpr size_t MB = 1024 * KB;
        constexpr size_t GB = 1024 * MB;

        // Pre-determined allocator sizes
        constexpr size_t FRAME_ALLOCATOR_SIZE = 10 * MB;   // Reset every frame
        constexpr size_t TEMP_ALLOCATOR_SIZE = 1 * MB;     // Short-lived allocations

        //=============================================================================
        // Memory Alignment Helpers
        //=============================================================================

        /**
         * @brief Align a pointer to the specified alignment
         * @note Alignment must be power of 2
         */
        inline void* AlignPointer(void* ptr, size_t alignment)
        {
            const size_t addr = reinterpret_cast<size_t>(ptr);
            const size_t alignedAddr = (addr + alignment - 1) & ~(alignment - 1);
            return reinterpret_cast<void*>(alignedAddr);
        }

        /**
         * @brief Calculate aligned size
         * @note Result is always >= original size
         */
        inline size_t AlignSize(size_t size, size_t alignment)
        {
            return (size + alignment - 1) & ~(alignment - 1);
        }

        inline bool IsAligned(const void* ptr, size_t alignment)
        {
            return (reinterpret_cast<size_t>(ptr) & (alignment - 1)) == 0;
        }

    } // namespace Memory
} // namespace Core