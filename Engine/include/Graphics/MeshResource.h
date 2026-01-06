/**
 * @file MeshResource.h
 * @brief 렌더링 가능한 메시를 관리하는 클래스
 *
 * VertexBuffer와 IndexBuffer를 하나의 렌더링 단위로 조합하여 관리합니다.
 * GPU 메모리에 지오메트리 데이터를 업로드하고 렌더링 시 바인딩/드로우를 수행합니다.
 *
 * @note Phase 4.2: 32비트 인덱스 지원 추가
 * @note Phase 4.4: 서브메시 지원 추가 (DrawSubmesh)
 */
#pragma once
#include "Graphics/GraphicsTypes.h"
#include "Graphics/DX12/DX12IndexBuffer.h"
#include "Graphics/DX12/DX12VertexBuffer.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/SubmeshInfo.h"
#include "Math/MathTypes.h"
#include <vector>

namespace Graphics
{
	class DX12CommandQueue;
	class DX12CommandContext;

	/**
	 * @brief 렌더링 가능한 메시를 관리하는 클래스
	 *
	 * VertexBuffer와 IndexBuffer를 하나의 렌더링 단위로 조합하여 관리합니다.
	 * GPU 메모리에 지오메트리 데이터를 업로드하고 렌더링 시 바인딩/드로우를 수행합니다.
	 *
	 * @note Phase 4.4: 서브메시별 개별 렌더링 지원
	 */
	class MeshResource
	{
	public:
		MeshResource() = default;
		~MeshResource();

		MeshResource(const MeshResource&) = delete;
		MeshResource& operator=(const MeshResource&) = delete;

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
		 * @brief 전체 메시 렌더링 (하위 호환용)
		 *
		 * 내부적으로 IASetVertexBuffers, IASetIndexBuffer를 호출하여 버퍼를 바인딩한 후,
		 * 인덱스 버퍼 유무에 따라 DrawIndexedInstanced 또는 DrawInstanced를 호출합니다.
		 *
		 * @param commandList 바인딩 및 드로우 커맨드를 기록할 커맨드 리스트
		 *
		 * @note 서브메시가 있는 경우 전체 인덱스를 한 번에 그립니다.
		 *       서브메시별 렌더링이 필요하면 DrawSubmesh()를 사용하세요.
		 */
		void Draw(ID3D12GraphicsCommandList* commandList) const;

		/**
		 * @brief 특정 서브메시만 렌더링
		 *
		 * @param commandList 커맨드 리스트
		 * @param submeshIndex 서브메시 인덱스 (0부터 시작)
		 *
		 * @note 버퍼 바인딩 후 해당 서브메시의 인덱스 범위만 그립니다.
		 * @note 유효하지 않은 인덱스는 무시됩니다.
		 */
		void DrawSubmesh(ID3D12GraphicsCommandList* commandList, Core::uint32 submeshIndex) const;

		//=====================================================================
		// 서브메시 관리
		//=====================================================================

		/**
		 * @brief 서브메시 정보 설정
		 *
		 * @param submeshes 서브메시 배열 (복사)
		 *
		 * @note ResourceManager::CreateMeshFromAsset()에서 호출됩니다.
		 * @note 빈 배열을 전달하면 전체 메시를 단일 서브메시로 자동 등록합니다.
		 */
		void SetSubmeshes(const std::vector<SubmeshInfo>& submeshes);

		/** @brief 서브메시 개수 (최소 1) */
		Core::uint32 GetSubmeshCount() const;

		/** @brief 특정 서브메시 정보 반환 */
		const SubmeshInfo& GetSubmesh(Core::uint32 index) const;

		/** @brief 서브메시 배열 반환 */
		const std::vector<SubmeshInfo>& GetSubmeshes() const { return mSubmeshes; }

		//=====================================================================
		// Getters
		//=====================================================================

		size_t GetVertexCount() const { return mVertexBuffer.GetVertexCount(); }
		size_t GetIndexCount() const { return mIndexBuffer.GetIndexCount(); }
		bool IsInitialized() const { return mInitialized; }
		bool HasIndexBuffer() const { return mIndexBuffer.IsInitialized(); }
		D3D12_INPUT_LAYOUT_DESC GetInputLayout() const { return mInputLayout; }

	private:
		/**
		 * @brief 전체 메시를 단일 서브메시로 등록
		 *
		 * 서브메시 정보가 없는 경우 자동으로 호출됩니다.
		 */
		void CreateDefaultSubmesh();

		DX12VertexBuffer mVertexBuffer;
		DX12IndexBuffer mIndexBuffer;
		D3D12_INPUT_LAYOUT_DESC mInputLayout = {};
		bool mInitialized = false;

		// Phase 4.4: 서브메시 정보
		std::vector<SubmeshInfo> mSubmeshes;
	};

} // namespace Graphics
