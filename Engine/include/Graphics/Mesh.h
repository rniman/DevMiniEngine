#pragma once

#include "Math/MathTypes.h"
#include "Graphics/GraphicsTypes.h"
#include "Graphics/DX12/DX12VertexBuffer.h"
#include "Graphics/DX12/DX12IndexBuffer.h"

namespace Graphics
{
	class DX12CommandQueue;
	class DX12CommandContext;

	/**
	 * @brief 기본 버텍스 구조체 (위치 + 색상)
	 *
	 * 첫 삼각형 렌더링을 위한 단순한 버텍스 포맷
	 */
	struct BasicVertex
	{
		Math::Vector3 position;  // 로컬 좌표계 위치
		Math::Vector4 color;     // RGBA 색상 (0.0 ~ 1.0)

		/**
		 * @brief 이 Vertex 타입에 대한 Input Layout 반환
		 */
		static D3D12_INPUT_LAYOUT_DESC GetInputLayout()
		{
			static D3D12_INPUT_ELEMENT_DESC elements[] =
			{
				{
					"POSITION",
					0,
					DXGI_FORMAT_R32G32B32_FLOAT,
					0,
					0,
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					0
				},
				{
					"COLOR",
					0,
					DXGI_FORMAT_R32G32B32A32_FLOAT,
					0,
					12,  // offsetof(BasicVertex, color)
					D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
					0
				}
			};

			return D3D12_INPUT_LAYOUT_DESC{ elements, 2 };
		}
	};

	/**
	 * @brief 렌더링 가능한 메시를 관리하는 클래스
	 *
	 * VertexBuffer와 IndexBuffer를 하나의 렌더링 단위로 조합하여 관리합니다.
	 * GPU 메모리에 지오메트리 데이터를 업로드하고 렌더링 시 바인딩/드로우를 수행합니다.
	 */
	class Mesh
	{
	public:
		Mesh() = default;
		~Mesh();

		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		/**
		 * @brief 정점 및 인덱스 데이터로 메시를 초기화합니다
		 *
		 * GPU 메모리에 버텍스와 인덱스 버퍼를 생성하고 데이터를 업로드합니다.
		 * Upload Heap을 통해 데이터를 복사하며, 완료 후 GPU 작업 대기를 수행합니다.
		 *
		 * @param device DirectX 12 디바이스
		 * @param commandQueue 커맨드 큐 (GPU 작업 제출 및 동기화)
		 * @param commandContext 커맨드 컨텍스트 (커맨드 리스트 및 할당자)
		 * @param vertices 업로드할 버텍스 데이터 배열
		 * @param vertexCount 버텍스 개수
		 * @param indices 업로드할 인덱스 데이터 배열 (nullptr이면 인덱스 버퍼 미사용)
		 * @param indexCount 인덱스 개수 (0이면 인덱스 버퍼 미사용)
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note 이 함수는 내부적으로 GPU 작업 완료를 대기합니다 (WaitForIdle)
		 */
		bool Initialize(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const BasicVertex* vertices,
			size_t vertexCount,
			const uint16* indices = nullptr,
			size_t indexCount = 0
		);

		void Shutdown();

		/**
		 * @brief Vertex/Index Buffer를 바인딩하고 Draw 커맨드를 실행합니다.
		 *
		 * 내부적으로 IASetVertexBuffers, IASetIndexBuffer를 호출하여 버퍼를 바인딩한 후,
		 * 인덱스 버퍼 유무에 따라 DrawIndexedInstanced 또는 DrawInstanced를 호출합니다.
		 * 렌더링에 필요한 모든 상태 (PSO, Root Signature)는 외부에서 미리 설정되어 있어야 합니다.
		 *
		 * @param commandList 바인딩 및 드로우 커맨드를 기록할 커맨드 리스트
		 */
		void Draw(ID3D12GraphicsCommandList* commandList);

		// Getters
		size_t GetVertexCount() const { return mVertexBuffer.GetVertexCount(); }
		size_t GetIndexCount() const { return mIndexBuffer.GetIndexCount(); }
		bool IsInitialized() const { return mInitialized; }
		bool HasIndexBuffer() const { return mIndexBuffer.IsInitialized(); }
		D3D12_INPUT_LAYOUT_DESC GetInputLayout() const { return mInputLayout; }

	private:
		DX12VertexBuffer mVertexBuffer;  // 버텍스 버퍼
		DX12IndexBuffer mIndexBuffer;    // 인덱스 버퍼 (선택적)
		D3D12_INPUT_LAYOUT_DESC mInputLayout = {};
		bool mInitialized = false;       // 초기화 여부
	};

} // namespace Graphics