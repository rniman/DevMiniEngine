/**
 * @file Mesh.cpp
 * @brief Mesh 클래스 구현
 */
#include "pch.h"
#include "Graphics/Mesh.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Graphics/DX12/DX12CommandQueue.h"

namespace Graphics
{
	Mesh::~Mesh()
	{
		Shutdown();
	}

	//=========================================================================
	// BasicVertex 초기화
	//=========================================================================

	bool Mesh::Initialize(
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
			LOG_ERROR("Mesh::Initialize - Invalid parameters");
			return false;
		}

		if (mInitialized)
		{
			LOG_WARN("Mesh::Initialize - Already initialized, shutting down first");
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
			LOG_ERROR("Mesh::Initialize - Failed to initialize vertex buffer");
			return false;
		}

		LOG_GFX_INFO("Mesh - Vertex buffer initialized (%u vertices)", vertexCount);

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
				LOG_ERROR("Mesh::Initialize - Failed to initialize index buffer");
				mVertexBuffer.Shutdown();
				return false;
			}

			LOG_GFX_INFO("Mesh - Index buffer initialized (%u indices)", indexCount);
		}

		mInputLayout = BasicVertex::GetInputLayout();

		mInitialized = true;
		LOG_GFX_INFO("Mesh initialized successfully (V:%u, I:%u)", vertexCount, indexCount);
		return true;
	}

	//=========================================================================
	// TexturedVertex 초기화
	//=========================================================================

	bool Mesh::InitializeTextured(
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
			LOG_ERROR("Mesh::InitializeTextured - Invalid parameters");
			return false;
		}

		if (mInitialized)
		{
			LOG_WARN("Mesh::InitializeTextured - Already initialized, shutting down first");
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
			LOG_ERROR("Mesh::InitializeTextured - Failed to initialize vertex buffer");
			return false;
		}

		LOG_GFX_INFO("Mesh - Vertex buffer initialized (%u textured vertices)", vertexCount);

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
				LOG_ERROR("Mesh::InitializeTextured - Failed to initialize index buffer");
				mVertexBuffer.Shutdown();
				return false;
			}

			LOG_GFX_INFO("Mesh - Index buffer initialized (%u indices)", indexCount);
		}

		mInputLayout = TexturedVertex::GetInputLayout();

		mInitialized = true;
		LOG_GFX_INFO("Mesh initialized successfully (V:%u, I:%u)", vertexCount, indexCount);
		return true;
	}

	//=========================================================================
	// StandardVertex 초기화 (16비트 인덱스)
	//=========================================================================

	bool Mesh::InitializeStandard(
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
			LOG_ERROR("Mesh::InitializeStandard - Invalid parameters");
			return false;
		}

		if (mInitialized)
		{
			LOG_WARN("Mesh::InitializeStandard - Already initialized, shutting down first");
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
			LOG_ERROR("Mesh::InitializeStandard - Failed to initialize vertex buffer");
			return false;
		}

		LOG_GFX_INFO("Mesh - Vertex buffer initialized (%u standard vertices)", vertexCount);

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
				LOG_ERROR("Mesh::InitializeStandard - Failed to initialize index buffer");
				mVertexBuffer.Shutdown();
				return false;
			}

			LOG_GFX_INFO("Mesh - Index buffer initialized (%u indices, 16-bit)", indexCount);
		}

		mInputLayout = StandardVertex::GetInputLayout();

		mInitialized = true;
		LOG_GFX_INFO("Mesh initialized successfully (V:%u, I:%u, 16-bit indices)", vertexCount, indexCount);
		return true;
	}

	//=========================================================================
	// StandardVertex 초기화 (32비트 인덱스)
	//=========================================================================

	bool Mesh::InitializeStandard32(
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
			LOG_GFX_ERROR("Mesh::InitializeStandard32 - Invalid parameters");
			return false;
		}

		if (mInitialized)
		{
			LOG_GFX_WARN("Mesh::InitializeStandard32 - Already initialized, shutting down first");
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
			LOG_GFX_ERROR("Mesh::InitializeStandard32 - Failed to initialize vertex buffer");
			return false;
		}

		LOG_GFX_INFO("Mesh - Vertex buffer initialized (%u standard vertices)", vertexCount);

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
				LOG_GFX_ERROR("Mesh::InitializeStandard32 - Failed to initialize index buffer");
				mVertexBuffer.Shutdown();
				return false;
			}

			LOG_GFX_INFO("Mesh - Index buffer initialized (%u indices, 32-bit)", indexCount);
		}

		mInputLayout = StandardVertex::GetInputLayout();

		mInitialized = true;
		LOG_GFX_INFO("Mesh initialized successfully (V:%u, I:%u, 32-bit indices)", vertexCount, indexCount);
		return true;
	}

	//=========================================================================
	// 공통
	//=========================================================================

	void Mesh::Shutdown()
	{
		if (!mInitialized)
		{
			return;
		}

		LOG_GFX_INFO("Mesh shutting down...");

		mVertexBuffer.Shutdown();
		mIndexBuffer.Shutdown();
		mInputLayout = {};
		mInitialized = false;
	}

	void Mesh::Draw(ID3D12GraphicsCommandList* commandList) const
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

} // namespace Graphics
