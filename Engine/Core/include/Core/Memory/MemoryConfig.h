#pragma once

#include "Core/Types.h"

namespace Core
{
    namespace Memory
    {
        //=============================================================================
        // Memory Configuration
        //=============================================================================

        // Memory alignment
        constexpr size_t DEFAULT_ALIGNMENT = 16;      // 16-byte aligned (SIMD)
        constexpr size_t CACHE_LINE_SIZE = 64;        // CPU cache line

        // Memory pool sizes
        constexpr size_t KB = 1024;
        constexpr size_t MB = 1024 * KB;
        constexpr size_t GB = 1024 * MB;

        constexpr size_t FRAME_ALLOCATOR_SIZE = 10 * MB;   // 10MB per frame
        constexpr size_t TEMP_ALLOCATOR_SIZE = 1 * MB;     // Temporary 1MB

        //=============================================================================
        // Memory Alignment Helpers
        //=============================================================================

        /**
         * @brief Align a pointer to the specified alignment
         * @param ptr Pointer to align
         * @param alignment Alignment boundary (must be power of 2)
         * @return Aligned pointer
         */
        inline void* AlignPointer(void* ptr, size_t alignment)
        {
            const size_t addr = reinterpret_cast<size_t>(ptr);
            const size_t alignedAddr = (addr + alignment - 1) & ~(alignment - 1);
            return reinterpret_cast<void*>(alignedAddr);
        }

        /**
         * @brief Calculate aligned size
         * @param size Original size
         * @param alignment Alignment boundary
         * @return Aligned size
         */
        inline size_t AlignSize(size_t size, size_t alignment)
        {
            return (size + alignment - 1) & ~(alignment - 1);
        }

        /**
         * @brief Check if a pointer is aligned
         */
        inline bool IsAligned(const void* ptr, size_t alignment)
        {
            return (reinterpret_cast<size_t>(ptr) & (alignment - 1)) == 0;
        }

    } // namespace Memory
} // namespace Core