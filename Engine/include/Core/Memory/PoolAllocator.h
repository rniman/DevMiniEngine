#pragma once
#include "Core/Memory/Allocator.h"
#include "Core/Memory/MemoryConfig.h"
#include "Core/Types.h"

namespace Core
{
    namespace Memory
    {
        /**
         * @brief 고정 크기 객체용 풀 할당자
         *
         * O(1) 할당/해제를 제공하는 free-list 기반 메모리 풀입니다.
         * 모든 청크가 동일한 크기이며, 빈번한 할당/해제 패턴에 최적화되어 있습니다.
         *
         * 사용 예:
         *   PoolAllocator pool(sizeof(MyObject), 100);
         *   void* obj = pool.Allocate(sizeof(MyObject));
         *   // ... 객체 사용
         *   pool.Deallocate(obj);
         *
         * @note 스레드 안전하지 않음
         * @note 정렬은 최대 sizeof(void*)까지 지원 (free-list 제약)
         * @warning size > chunkSize인 할당은 assertion 실패
         */
        class PoolAllocator : public Allocator
        {
        public:
            /**
             * @brief 풀 할당자 생성
             *
             * @param chunkSize 각 청크의 크기 (바이트, 최소 sizeof(void*))
             * @param chunkCount 전체 청크 개수
             *
             * @note chunkSize는 자동으로 sizeof(void*)의 배수로 정렬됨
             */
            explicit PoolAllocator(size_t chunkSize, size_t chunkCount);

            ~PoolAllocator() override;

            PoolAllocator(const PoolAllocator&) = delete;
            PoolAllocator& operator=(const PoolAllocator&) = delete;

            /**
             * @brief 풀에서 메모리 할당
             *
             * @param size 할당할 바이트 크기 (chunkSize 이하여야 함)
             * @param alignment 정렬 요구사항 (기본: DEFAULT_ALIGNMENT, 최대: sizeof(void*))
             * @return 할당된 메모리 포인터, 풀이 가득 찬 경우 nullptr
             *
             * @note O(1) 복잡도, free-list에서 pop
             * @warning size가 chunkSize를 초과하면 assertion 실패
             */
            void* Allocate(size_t size, size_t alignment = DEFAULT_ALIGNMENT) override;

            /**
             * @brief 메모리를 풀에 반환
             *
             * @param ptr 해제할 메모리 포인터 (nullptr 허용)
             *
             * @note O(1) 복잡도, free-list에 push
             * @warning ptr은 이 풀에서 할당된 것이어야 함
             */
            void Deallocate(void* ptr) override;

            /**
             * @brief 풀 초기화
             *
             * @note 모든 청크를 free-list로 복원
             * @warning 아직 할당된 객체가 있어도 초기화됨 (수동 소멸 필요)
             */
            void Reset() override;

            /**
             * @brief 현재 할당된 총 메모리 크기
             * @return 할당된 바이트 수 (chunkSize * 할당된 청크 개수)
             */
            size_t GetAllocatedSize() const override { return mChunkSize * mAllocatedChunks; }

            /**
             * @brief 현재 할당된 청크 개수
             * @return 활성 청크 개수
             */
            size_t GetAllocationCount() const override { return mAllocatedChunks; }

            /**
             * @brief 청크 크기
             * @return 각 청크의 바이트 크기 (정렬 적용됨)
             */
            size_t GetChunkSize() const { return mChunkSize; }

            /**
             * @brief 전체 청크 개수
             * @return 풀의 총 청크 개수
             */
            size_t GetChunkCount() const { return mChunkCount; }

            /**
             * @brief 남은 여유 청크 개수
             * @return 할당 가능한 청크 개수
             */
            size_t GetFreeChunkCount() const { return mChunkCount - mAllocatedChunks; }

        private:
            /**
             * @brief free-list 초기화
             *
             * @note 모든 청크를 연결 리스트로 구성
             */
            void InitializeFreeList();

            void* mMemory;              // 메모리 블록 시작 주소
            size_t mChunkSize;          // 각 청크의 크기
            size_t mChunkCount;         // 전체 청크 개수
            size_t mAllocatedChunks;    // 현재 할당된 청크 개수
            void* mFreeList;            // free-list 헤드 포인터
        };

    } // namespace Memory
} // namespace Core
