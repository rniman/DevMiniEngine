#include "pch.h"
#include "Graphics/Mesh.h"
#include "Graphics/DX12/DX12CommandQueue.h"
#include "Graphics/DX12/DX12CommandContext.h"
#include "Core/Logging/LogMacros.h"

namespace Graphics
{
	Mesh::~Mesh()
	{
		Shutdown();
	}

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
		// 유효성 검증
		if (!device || !commandQueue || !commandContext || !vertices || vertexCount == 0)
		{
			LOG_ERROR("Mesh::Initialize - Invalid parameters");
			return false;
		}

		// 이미 초기화된 경우 정리
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

		LOG_GRAPHICS_INFO("Mesh - Vertex buffer initialized (%u vertices)", vertexCount);

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

			LOG_GRAPHICS_INFO("Mesh - Index buffer initialized (%u indices)", indexCount);
		}

		mInputLayout = BasicVertex::GetInputLayout();

		mInitialized = true;
		LOG_GRAPHICS_INFO("Mesh initialized successfully (V:%u, I:%u)", vertexCount, indexCount);
		return true;
	}

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
		// 유효성 검증
		if (!device || !commandQueue || !commandContext || !vertices || vertexCount == 0)
		{
			LOG_ERROR("Mesh::InitializeTextured - Invalid parameters");
			return false;
		}

		// 이미 초기화된 경우 정리
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
		LOG_GRAPHICS_INFO("Mesh - Vertex buffer initialized (%u textured vertices)", vertexCount);

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
			LOG_GRAPHICS_INFO("Mesh - Index buffer initialized (%u indices)", indexCount);
		}

		mInputLayout = TexturedVertex::GetInputLayout();  // 변경됨

		mInitialized = true;
		LOG_GRAPHICS_INFO("Mesh initialized successfully (V:%u, I:%u)", vertexCount, indexCount);
		return true;
	}

	void Mesh::Shutdown()
	{
		if (!mInitialized)
		{
			return;
		}

		mVertexBuffer.Shutdown();
		mIndexBuffer.Shutdown();
		mInitialized = false;

		LOG_GRAPHICS_INFO("Mesh shutdown complete");
	}

	void Mesh::Draw(ID3D12GraphicsCommandList* commandList)
	{
		if (!mInitialized)
		{
			LOG_ERROR("Mesh::Draw - Mesh not initialized");
			return;
		}

		// Vertex Buffer 바인딩
		D3D12_VERTEX_BUFFER_VIEW vbv = mVertexBuffer.GetVertexBufferView();
		commandList->IASetVertexBuffers(0, 1, &vbv);

		// Index Buffer 바인딩 (있는 경우)
		if (mIndexBuffer.IsInitialized())
		{
			D3D12_INDEX_BUFFER_VIEW ibv = mIndexBuffer.GetIndexBufferView();
			commandList->IASetIndexBuffer(&ibv);
		}

		// Index Buffer 사용 여부에 따라 다른 Draw 호출
		if (mIndexBuffer.IsInitialized())
		{
			// DrawIndexedInstanced(IndexCount, InstanceCount, StartIndex, BaseVertex, StartInstance)
			commandList->DrawIndexedInstanced(
				static_cast<UINT>(mIndexBuffer.GetIndexCount()),
				1,  // 인스턴스 1개
				0,  // 첫 번째 인덱스부터
				0,  // 베이스 버텍스 오프셋 없음
				0   // 첫 번째 인스턴스
			);
		}
		else
		{
			// DrawInstanced(VertexCount, InstanceCount, StartVertex, StartInstance)
			commandList->DrawInstanced(
				static_cast<UINT>(mVertexBuffer.GetVertexCount()),
				1,  // 인스턴스 1개
				0,  // 첫 번째 버텍스부터
				0   // 첫 번째 인스턴스
			);
		}
	}

} // namespace Graphics