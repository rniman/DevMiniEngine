#pragma once

#include "Core/Types.h"
#include "Core/Memory/MemoryConfig.h"

namespace Core
{
    namespace Memory
    {
        /**
         * @brief 메모리 할당자 기본 인터페이스
         *
         * 모든 커스텀 할당자가 상속받는 추상 클래스입니다.
         * LinearAllocator, PoolAllocator, StackAllocator 등이 이 인터페이스를 구현합니다.
         *
         * @note 스레드 안전성은 구현체에 따라 다릅니다.
         */
        class Allocator
        {
        public:
            virtual ~Allocator() = default;

            /**
             * @brief 메모리 할당
             *
             * @param size 할당할 바이트 크기
             * @param alignment 정렬 요구사항 (기본: DEFAULT_ALIGNMENT)
             * @return 할당된 메모리 포인터, 실패 시 nullptr
             *
             * @note 스레드 안전성은 구현체에 따라 다름
             */
            virtual void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) = 0;

            /**
             * @brief 할당자 초기화 (지원되는 경우)
             *
             * @note 기본 구현은 no-op
             * @note LinearAllocator, StackAllocator는 모든 할당을 한 번에 해제
             */
            virtual void Reset() {}

            /**
             * @brief 메모리 해제
             *
             * @param ptr 해제할 메모리 포인터 (nullptr 허용)
             *
             * @note LinearAllocator 같은 일부 구현체는 개별 해제를 지원하지 않음
             */
            virtual void Deallocate(void* ptr) = 0;

            /**
             * @brief 현재 할당된 총 메모리 크기
             * @return 할당된 바이트 수
             */
            virtual size_t GetAllocatedSize() const = 0;

            /**
             * @brief 현재 할당 횟수
             * @return 활성 할당 개수
             */
            virtual size_t GetAllocationCount() const = 0;

        protected:
            Allocator() = default;

            // Non-copyable
            Allocator(const Allocator&) = delete;
            Allocator& operator=(const Allocator&) = delete;
        };

    } // namespace Memory
} // namespace Core