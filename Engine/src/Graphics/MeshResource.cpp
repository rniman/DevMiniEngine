/**
 * @file MeshResource.cpp
 * @brief MeshResource 클래스 구현
 */
#include "pch.h"
#include "Graphics/MeshResource.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12CommandQueue.h"

namespace Graphics
{
	//=========================================================================
	// 정적 더미 서브메시 (범위 초과 시 반환)
	//=========================================================================

	static const SubmeshInfo sEmptySubmesh = {};

	MeshResource::~MeshResource()
	{
		Shutdown();
	}

	//=========================================================================
	// BasicVertex 초기화
	//=========================================================================

	bool MeshResource::Initialize(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const BasicVertex* vertices,
		size_t vertexCount,
		const Core::uint16* indices,
		size_t indexCount
	)
	{
		if (!device || !commandQueue || !commandContext || !vertices || vertexCount == 0)
		{
			LOG_ERROR("[MeshResource] Initialize - Invalid parameters");
			return false;
		}

		if (mInitialized)
		{
			LOG_WARN("[MeshResource] Initialize - Already initialized, shutting down first");
			Shutdown();
		}

		// Vertex Buffer 초기화
		if (!mVertexBuffer.Initialize(
			device,
			commandQueue,
			commandContext,
			vertices,
			vertexCount,
			sizeof(BasicVertex)
		))
		{
			LOG_ERROR("[MeshResource] Initialize - Failed to initialize vertex buffer");
			return false;
		}

		LOG_GFX_INFO("[MeshResource] - Vertex buffer initialized (%u vertices)", vertexCount);

		// Index Buffer 초기화 (선택적)
		if (indices && indexCount > 0)
		{
			if (!mIndexBuffer.Initialize(
				device,
				commandQueue,
				commandContext,
				indices,
				indexCount,
				DXGI_FORMAT_R16_UINT
			))
			{
				LOG_ERROR("[MeshResource] Initialize - Failed to initialize index buffer");
				mVertexBuffer.Shutdown();
				return false;
			}

			LOG_GFX_INFO("[MeshResource] - Index buffer initialized (%u indices)", indexCount);
		}

		mInputLayout = BasicVertex::GetInputLayout();

		mInitialized = true;
		LOG_GFX_INFO("[MeshResource] initialized successfully (V:%u, I:%u)", vertexCount, indexCount);
		return true;
	}

	//=========================================================================
	// TexturedVertex 초기화
	//=========================================================================

	bool MeshResource::InitializeTextured(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const TexturedVertex* vertices,
		size_t vertexCount,
		const Core::uint16* indices,
		size_t indexCount
	)
	{
		if (!device || !commandQueue || !commandContext || !vertices || vertexCount == 0)
		{
			LOG_ERROR("[MeshResource] InitializeTextured - Invalid parameters");
			return false;
		}

		if (mInitialized)
		{
			LOG_WARN("[MeshResource] InitializeTextured - Already initialized, shutting down first");
			Shutdown();
		}

		// Vertex Buffer 초기화
		if (!mVertexBuffer.Initialize(
			device,
			commandQueue,
			commandContext,
			vertices,
			vertexCount,
			sizeof(TexturedVertex)
		))
		{
			LOG_ERROR("[MeshResource] InitializeTextured - Failed to initialize vertex buffer");
			return false;
		}

		LOG_GFX_INFO("[MeshResource] - Vertex buffer initialized (%u textured vertices)", vertexCount);

		// Index Buffer 초기화 (선택적)
		if (indices && indexCount > 0)
		{
			if (!mIndexBuffer.Initialize(
				device,
				commandQueue,
				commandContext,
				indices,
				indexCount,
				DXGI_FORMAT_R16_UINT
			))
			{
				LOG_ERROR("[MeshResource] InitializeTextured - Failed to initialize index buffer");
				mVertexBuffer.Shutdown();
				return false;
			}

			LOG_GFX_INFO("[MeshResource] - Index buffer initialized (%u indices)", indexCount);
		}

		mInputLayout = TexturedVertex::GetInputLayout();

		mInitialized = true;
		LOG_GFX_INFO("[MeshResource] initialized successfully (V:%u, I:%u)", vertexCount, indexCount);
		return true;
	}

	//=========================================================================
	// StandardVertex 초기화 (16비트 인덱스)
	//=========================================================================

	bool MeshResource::InitializeStandard(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const StandardVertex* vertices,
		size_t vertexCount,
		const Core::uint16* indices,
		size_t indexCount
	)
	{
		if (!device || !commandQueue || !commandContext || !vertices || vertexCount == 0)
		{
			LOG_ERROR("[MeshResource] InitializeStandard - Invalid parameters");
			return false;
		}

		if (mInitialized)
		{
			LOG_WARN("[MeshResource] InitializeStandard - Already initialized, shutting down first");
			Shutdown();
		}

		// Vertex Buffer 초기화
		if (!mVertexBuffer.Initialize(
			device,
			commandQueue,
			commandContext,
			vertices,
			vertexCount,
			sizeof(StandardVertex)
		))
		{
			LOG_ERROR("[MeshResource] InitializeStandard - Failed to initialize vertex buffer");
			return false;
		}

		LOG_GFX_INFO("[MeshResource] - Vertex buffer initialized (%u standard vertices)", vertexCount);

		// Index Buffer 초기화 (선택적)
		if (indices && indexCount > 0)
		{
			if (!mIndexBuffer.Initialize(
				device,
				commandQueue,
				commandContext,
				indices,
				indexCount,
				DXGI_FORMAT_R16_UINT
			))
			{
				LOG_ERROR("[MeshResource] InitializeStandard - Failed to initialize index buffer");
				mVertexBuffer.Shutdown();
				return false;
			}

			LOG_GFX_INFO("[MeshResource] - Index buffer initialized (%u indices, 16-bit)", indexCount);
		}

		mInputLayout = StandardVertex::GetInputLayout();

		mInitialized = true;
		LOG_GFX_INFO("[MeshResource] initialized successfully (V:%u, I:%u, 16-bit indices)", vertexCount, indexCount);
		return true;
	}

	//=========================================================================
	// StandardVertex 초기화 (32비트 인덱스)
	//=========================================================================

	bool MeshResource::InitializeStandard32(
		ID3D12Device* device,
		DX12CommandQueue* commandQueue,
		DX12CommandContext* commandContext,
		const StandardVertex* vertices,
		size_t vertexCount,
		const Core::uint32* indices,
		size_t indexCount
	)
	{
		if (!device || !commandQueue || !commandContext || !vertices || vertexCount == 0)
		{
			LOG_GFX_ERROR("[MeshResource] InitializeStandard32 - Invalid parameters");
			return false;
		}

		if (mInitialized)
		{
			LOG_GFX_WARN("[MeshResource] InitializeStandard32 - Already initialized, shutting down first");
			Shutdown();
		}

		// Vertex Buffer 초기화
		if (!mVertexBuffer.Initialize(
			device,
			commandQueue,
			commandContext,
			vertices,
			vertexCount,
			sizeof(StandardVertex)
		))
		{
			LOG_GFX_ERROR("[MeshResource] InitializeStandard32 - Failed to initialize vertex buffer");
			return false;
		}

		LOG_GFX_INFO("[MeshResource] - Vertex buffer initialized (%u standard vertices)", vertexCount);

		// Index Buffer 초기화 (32비트)
		if (indices && indexCount > 0)
		{
			if (!mIndexBuffer.Initialize(
				device,
				commandQueue,
				commandContext,
				indices,
				indexCount,
				DXGI_FORMAT_R32_UINT  // 32비트 인덱스
			))
			{
				LOG_GFX_ERROR("[MeshResource] InitializeStandard32 - Failed to initialize index buffer");
				mVertexBuffer.Shutdown();
				return false;
			}

			LOG_GFX_INFO("[MeshResource] - Index buffer initialized (%u indices, 32-bit)", indexCount);
		}

		mInputLayout = StandardVertex::GetInputLayout();

		mInitialized = true;
		LOG_GFX_INFO("[MeshResource] initialized successfully (V:%u, I:%u, 32-bit indices)", vertexCount, indexCount);
		return true;
	}

	//=========================================================================
	// 공통
	//=========================================================================

	void MeshResource::Shutdown()
	{
		if (!mInitialized)
		{
			return;
		}

		LOG_GFX_INFO("[MeshResource] shutting down...");

		mVertexBuffer.Shutdown();
		mIndexBuffer.Shutdown();
		mInputLayout = {};
		mInitialized = false;
	}

	void MeshResource::Draw(ID3D12GraphicsCommandList* commandList) const
	{
		if (!mInitialized || !commandList)
		{
			return;
		}

		// Vertex Buffer 바인딩
		D3D12_VERTEX_BUFFER_VIEW vbView = mVertexBuffer.GetVertexBufferView();
		commandList->IASetVertexBuffers(0, 1, &vbView);

		// Index Buffer 바인딩 및 Draw
		if (mIndexBuffer.IsInitialized())
		{
			D3D12_INDEX_BUFFER_VIEW ibView = mIndexBuffer.GetIndexBufferView();
			commandList->IASetIndexBuffer(&ibView);
			commandList->DrawIndexedInstanced(
				static_cast<UINT>(mIndexBuffer.GetIndexCount()),
				1, 0, 0, 0
			);
		}
		else
		{
			commandList->DrawInstanced(
				static_cast<UINT>(mVertexBuffer.GetVertexCount()),
				1, 0, 0
			);
		}
	}


	void MeshResource::DrawSubmesh(ID3D12GraphicsCommandList* commandList, Core::uint32 submeshIndex) const
	{
		if (!mInitialized || !commandList)
		{
			return;
		}

		if (submeshIndex >= mSubmeshes.size())
		{
			LOG_WARN(
				"[MeshResource] DrawSubmesh: index %u out of range (count: %zu)",
				submeshIndex, mSubmeshes.size()
			);
			return;
		}

		const SubmeshInfo& submesh = mSubmeshes[submeshIndex];

		// Vertex Buffer 바인딩
		D3D12_VERTEX_BUFFER_VIEW vbView = mVertexBuffer.GetVertexBufferView();
		commandList->IASetVertexBuffers(0, 1, &vbView);

		// Index Buffer 바인딩 및 서브메시 범위만 Draw
		if (mIndexBuffer.IsInitialized())
		{
			D3D12_INDEX_BUFFER_VIEW ibView = mIndexBuffer.GetIndexBufferView();
			commandList->IASetIndexBuffer(&ibView);
			commandList->DrawIndexedInstanced(
				static_cast<UINT>(submesh.indexCount),
				1,
				static_cast<UINT>(submesh.startIndex),
				static_cast<INT>(submesh.baseVertex),
				0
			);
		}
		else
		{
			// 인덱스 버퍼 없는 경우 (드문 케이스)
			commandList->DrawInstanced(
				static_cast<UINT>(submesh.indexCount),  // 정점 수로 해석
				1,
				static_cast<UINT>(submesh.startIndex),
				0
			);
		}
	}

	//=========================================================================
	// 서브메시 관리
	//=========================================================================

	void MeshResource::SetSubmeshes(const std::vector<SubmeshInfo>& submeshes)
	{
		if (submeshes.empty())
		{
			// 서브메시 정보 없으면 전체를 단일 서브메시로
			CreateDefaultSubmesh();
		}
		else
		{
			mSubmeshes = submeshes;
		}

		LOG_DEBUG("[MeshResource] SetSubmeshes: %zu submesh(es)", mSubmeshes.size());
	}

	Core::uint32 MeshResource::GetSubmeshCount() const
	{
		return static_cast<Core::uint32>(mSubmeshes.size());
	}

	const SubmeshInfo& MeshResource::GetSubmesh(Core::uint32 index) const
	{
		if (index < mSubmeshes.size())
		{
			return mSubmeshes[index];
		}

		LOG_WARN(
			"[MeshResource] GetSubmesh: index %u out of range (count: %zu)",
			index,
			mSubmeshes.size()
		);
		return sEmptySubmesh;
	}

	void MeshResource::CreateDefaultSubmesh()
	{
		mSubmeshes.clear();

		SubmeshInfo defaultSubmesh;
		defaultSubmesh.startIndex = 0;
		defaultSubmesh.indexCount = static_cast<Core::uint32>(mIndexBuffer.GetIndexCount());
		defaultSubmesh.baseVertex = 0;
		defaultSubmesh.materialIndex = 0;

		mSubmeshes.push_back(defaultSubmesh);

		LOG_DEBUG("[MeshResource] Created default submesh (indexCount: %u)", defaultSubmesh.indexCount);
	}

} // namespace Graphics
