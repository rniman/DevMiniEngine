#pragma once
#include "Core/Types.h"

namespace Core
{
    namespace Memory
    {
        //=============================================================================
        // 메모리 설정
        //=============================================================================

        // 메모리 할당 기본 정렬
        constexpr size_t DEFAULT_ALIGNMENT = 16;      // SIMD 최적화
        constexpr size_t CACHE_LINE_SIZE = 64;        // CPU 캐시 라인

        // 표준 메모리 크기 단위
        constexpr size_t KB = 1024;
        constexpr size_t MB = 1024 * KB;
        constexpr size_t GB = 1024 * MB;

        // 할당자 사전 정의 크기
        constexpr size_t FRAME_ALLOCATOR_SIZE = 10 * MB;   // 프레임마다 리셋
        constexpr size_t TEMP_ALLOCATOR_SIZE = 1 * MB;     // 단기 할당용

        //=============================================================================
        // 메모리 정렬 헬퍼 함수
        //=============================================================================

        /**
         * @brief 포인터를 지정된 정렬로 정렬
         *
         * @param ptr 정렬할 포인터
         * @param alignment 정렬 크기 (2의 거듭제곱)
         * @return 정렬된 포인터
         *
         * @note alignment는 반드시 2의 거듭제곱이어야 함
         */
        inline void* AlignPointer(void* ptr, size_t alignment)
        {
            const size_t addr = reinterpret_cast<size_t>(ptr);
            const size_t alignedAddr = (addr + alignment - 1) & ~(alignment - 1);
            return reinterpret_cast<void*>(alignedAddr);
        }

        /**
         * @brief 정렬된 크기 계산
         *
         * @param size 원본 크기
         * @param alignment 정렬 크기 (2의 거듭제곱)
         * @return 정렬된 크기
         *
         * @note 결과는 항상 원본 크기 이상
         */
        inline size_t AlignSize(size_t size, size_t alignment)
        {
            return (size + alignment - 1) & ~(alignment - 1);
        }

        /**
         * @brief 포인터 정렬 여부 확인
         *
         * @param ptr 확인할 포인터
         * @param alignment 정렬 크기 (2의 거듭제곱)
         * @return 정렬되어 있으면 true
         */
        inline bool IsAligned(const void* ptr, size_t alignment)
        {
            return (reinterpret_cast<size_t>(ptr) & (alignment - 1)) == 0;
        }

    } // namespace Memory
} // namespace Core
