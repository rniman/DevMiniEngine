#pragma once

#include "Core/Memory/Allocator.h"

namespace Core
{
    namespace Memory
    {
        /**
         * @brief 선형(아레나) 할당자
         *
         * 프레임 단위 임시 데이터를 위한 고속 범프 포인터 할당자입니다.
         * 개별 할당 해제가 불가능하며, 전체 할당자를 초기화해야 합니다.
         *
         * 사용 예:
         *   LinearAllocator frameAlloc(10 * MB);
         *   void* memory = frameAlloc.Allocate(1024);
         *   // ... 메모리 사용
         *   frameAlloc.Reset();  // 한 번에 모두 해제
         *
         * @note 스레드 안전하지 않음
         */
        class LinearAllocator : public Allocator
        {
        public:
            /**
             * @brief 선형 할당자 생성
             * @param size 할당자 전체 용량 (바이트)
             */
            explicit LinearAllocator(size_t size);

            ~LinearAllocator() override;

            LinearAllocator(const LinearAllocator&) = delete;
            LinearAllocator& operator=(const LinearAllocator&) = delete;

            /**
             * @brief 선형 버퍼에서 메모리 할당
             *
             * @param size 할당할 바이트 크기
             * @param alignment 정렬 요구사항 (기본: DEFAULT_ALIGNMENT)
             * @return 할당된 메모리 포인터, 공간 부족 시 nullptr
             *
             * @note O(1) 복잡도, 패딩으로 인해 실제 사용량은 size보다 클 수 있음
             * @warning 할당된 메모리는 Reset() 호출 전까지 유효
             */
            void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;

            /**
             * @brief 할당자 초기화
             *
             * @note 모든 할당을 한 번에 해제하고 오프셋을 0으로 재설정
             */
            void Reset() override;

            /**
             * @brief No-op (개별 해제 불가)
             *
             * @param ptr 무시됨
             *
             * @note LinearAllocator는 개별 해제를 지원하지 않음
             * @note 모든 메모리를 해제하려면 Reset()을 사용하세요
             */
            void Deallocate(void* ptr) override;

            /**
             * @brief 현재 할당된 메모리 크기
             * @return 할당된 바이트 수 (패딩 포함)
             */
            size_t GetAllocatedSize() const override { return mOffset; }

            /**
             * @brief 현재 할당 횟수
             * @return 활성 할당 개수
             */
            size_t GetAllocationCount() const override { return mAllocationCount; }

            /**
             * @brief 할당자의 총 용량
             * @return 전체 버퍼 크기 (바이트)
             */
            size_t GetCapacity() const { return mSize; }

            /**
             * @brief 남은 여유 공간
             * @return 할당 가능한 바이트 수 (대략적, 패딩 미포함)
             */
            size_t GetFreeSpace() const { return mSize - mOffset; }

        private:
            void* mMemory;              // 메모리 블록 시작 주소
            size_t mSize;               // 전체 크기
            size_t mOffset;             // 현재 오프셋 (할당된 크기)
            size_t mAllocationCount;    // 할당 횟수
        };

    } // namespace Memory
} // namespace Core