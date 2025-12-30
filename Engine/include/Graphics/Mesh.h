/**
 * @file Mesh.h
 * @brief 렌더링 가능한 메시를 관리하는 클래스
 *
 * VertexBuffer와 IndexBuffer를 하나의 렌더링 단위로 조합하여 관리합니다.
 * GPU 메모리에 지오메트리 데이터를 업로드하고 렌더링 시 바인딩/드로우를 수행합니다.
 *
 * @note Phase 4.2: 32비트 인덱스 지원 추가
 */
#pragma once
#include "Graphics/GraphicsTypes.h"
#include "Graphics/DX12/DX12IndexBuffer.h"
#include "Graphics/DX12/DX12VertexBuffer.h"
#include "Graphics/VertexTypes.h"
#include "Math/MathTypes.h"

namespace Graphics
{
	class DX12CommandQueue;
	class DX12CommandContext;

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

		//=====================================================================
		// BasicVertex 초기화
		//=====================================================================

		/**
		 * @brief BasicVertex + 16비트 인덱스로 메시 초기화
		 */
		bool Initialize(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const BasicVertex* vertices,
			size_t vertexCount,
			const Core::uint16* indices = nullptr,
			size_t indexCount = 0
		);

		//=====================================================================
		// TexturedVertex 초기화
		//=====================================================================

		/**
		 * @brief TexturedVertex + 16비트 인덱스로 메시 초기화
		 */
		bool InitializeTextured(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const TexturedVertex* vertices,
			size_t vertexCount,
			const Core::uint16* indices,
			size_t indexCount
		);

		//=====================================================================
		// StandardVertex 초기화
		//=====================================================================

		/**
		 * @brief StandardVertex + 16비트 인덱스로 메시 초기화
		 *
		 * 정점 수가 65535 이하인 경우 사용합니다.
		 */
		bool InitializeStandard(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const StandardVertex* vertices,
			size_t vertexCount,
			const Core::uint16* indices,
			size_t indexCount
		);

		/**
		 * @brief StandardVertex + 32비트 인덱스로 메시 초기화
		 *
		 * 정점 수가 65535를 초과하는 경우 사용합니다.
		 *
		 * @note 32비트 인덱스는 메모리를 2배 사용하므로,
		 *       가능하면 16비트 버전을 사용하세요.
		 */
		bool InitializeStandard32(
			ID3D12Device* device,
			DX12CommandQueue* commandQueue,
			DX12CommandContext* commandContext,
			const StandardVertex* vertices,
			size_t vertexCount,
			const Core::uint32* indices,
			size_t indexCount
		);

		//=====================================================================
		// 공통
		//=====================================================================

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
		void Draw(ID3D12GraphicsCommandList* commandList) const;

		// Getters
		size_t GetVertexCount() const { return mVertexBuffer.GetVertexCount(); }
		size_t GetIndexCount() const { return mIndexBuffer.GetIndexCount(); }
		bool IsInitialized() const { return mInitialized; }
		bool HasIndexBuffer() const { return mIndexBuffer.IsInitialized(); }
		D3D12_INPUT_LAYOUT_DESC GetInputLayout() const { return mInputLayout; }

	private:
		DX12VertexBuffer mVertexBuffer;
		DX12IndexBuffer mIndexBuffer;
		D3D12_INPUT_LAYOUT_DESC mInputLayout = {};
		bool mInitialized = false;
	};

} // namespace Graphics
