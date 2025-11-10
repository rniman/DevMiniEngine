#pragma once
#include "Core/Memory/Allocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Types.h"

namespace Core
{
    namespace Memory
    {
        /**
         * @brief 스코프 할당을 위한 스택 할당자
         *
         * LIFO (Last In First Out) 할당 패턴을 지원하는 할당자입니다.
         * 중첩된 스코프 패턴에 최적화되어 있으며, 마커를 통한 부분 해제를 지원합니다.
         *
         * 사용 예:
         *   StackAllocator stack(1 * MB);
         *   auto marker = stack.GetMarker();
         *   void* temp = stack.Allocate(1024);
         *   // ... 메모리 사용
         *   stack.FreeToMarker(marker);  // 마커 이후 할당 모두 해제
         *
         * @note 스레드 안전하지 않음
         * @note 마커는 반드시 LIFO 순서로 해제해야 함
         * @warning FreeToMarker는 LIFO 순서로만 호출해야 함 (최신 마커부터 해제)
         * @warning 순서를 지키지 않으면 메모리 누수 또는 잘못된 해제 발생 가능
         */
        class StackAllocator : public Allocator
        {
        public:
            /**
             * @brief 스택 위치를 나타내는 마커 타입
             *
             * @note 내부적으로 오프셋 값(size_t)을 사용
             * @note GetMarker()로 획득하고 FreeToMarker()로 되돌림
             */
            using Marker = size_t;

            /**
             * @brief 스택 할당자 생성
             * @param size 할당자 전체 용량 (바이트)
             */
            explicit StackAllocator(size_t size);

            ~StackAllocator() override;

            StackAllocator(const StackAllocator&) = delete;
            StackAllocator& operator=(const StackAllocator&) = delete;

            /**
             * @brief 스택에서 메모리 할당
             *
             * @param size 할당할 바이트 크기
             * @param alignment 정렬 요구사항 (기본: DEFAULT_ALIGNMENT)
             * @return 할당된 메모리 포인터, 공간 부족 시 nullptr
             *
             * @note O(1) 복잡도, 패딩으로 인해 실제 사용량은 size보다 클 수 있음
             * @warning 할당된 메모리는 FreeToMarker() 또는 Reset() 호출 전까지 유효
             */
            void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;

            /**
             * @brief 현재 스택 위치 반환
             *
             * @return 현재 오프셋을 나타내는 마커
             *
             * @note 나중에 이 마커로 되돌릴 수 있음 (LIFO)
             */
            Marker GetMarker() const { return mOffset; }

            /**
             * @brief 특정 마커 위치로 스택을 되돌림
             *
             * @param marker 되돌릴 스택 위치 (GetMarker()로 사전 획득한 값)
             *
             * 사용 예:
             *   auto marker1 = stack.GetMarker();
             *   void* data1 = stack.Allocate(100);
             *
             *   auto marker2 = stack.GetMarker();
             *   void* data2 = stack.Allocate(200);
             *
             *   stack.FreeToMarker(marker2);  // data2 해제
             *   stack.FreeToMarker(marker1);  // data1 해제
             *
             * @note LIFO 순서 필수: marker2 - marker1 순서로 해제해야 함
             * @note 할당 카운트는 업데이트되지 않음 (근사값으로 유지)
             * @warning marker > mOffset인 경우 assertion 실패
             * @warning 잘못된 순서 (marker1 - marker2)로 호출하면 안 됨
             */
            void FreeToMarker(Marker marker);

            /**
             * @brief 할당자 초기화
             * @note 모든 할당을 한 번에 해제하고 오프셋을 0으로 재설정
             */
            void Reset() override;

            /**
             * @brief No-op (개별 해제 불가)
             *
             * @param ptr 무시됨
             *
             * @note StackAllocator는 개별 해제를 지원하지 않음
             * @note 메모리를 해제하려면 FreeToMarker() 또는 Reset()을 사용하세요
             */
            void Deallocate(void* ptr) override;

            /**
             * @brief 현재 할당된 메모리 크기
             * @return 할당된 바이트 수 (패딩 포함)
             */
            size_t GetAllocatedSize() const override { return mOffset; }

            /**
             * @brief 누적 할당 횟수 (근사값)
             *
             * @return 대략적인 할당 횟수
             *
             * @note FreeToMarker()는 할당 카운트를 업데이트하지 않으므로 부정확할 수 있음
             * @note 정확한 카운트는 Reset() 호출 시에만 보장됨
             * @warning 이 값은 디버깅 참고용으로만 사용하세요
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
            size_t GetAvailableSize() const { return mSize - mOffset; }

        private:
            void* mMemory;
            size_t mSize;
            size_t mOffset;             // 다음 할당 위치 (마커로도 사용됨)
            size_t mAllocationCount;    // 근사값 (FreeToMarker 시 부정확)
        };

    } // namespace Memory
} // namespace Core
