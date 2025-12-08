#pragma once
#include "Graphics/GraphicsTypes.h"
#include "Graphics/RenderTypes.h"
#include <array>
#include <memory>
#include <vector>

namespace Framework
{
	class ResourceManager;
}

namespace Graphics
{
	// Object Constants (b0)
	struct ObjectConstants
	{
		Math::Matrix4x4 worldMatrix;
		Math::Matrix4x4 mvpMatrix;
	};

	// Material Constant Buffer (b1) - Phase 3.3
	struct MaterialConstants
	{
		Math::Vector4 baseColor;
		Core::float32 metallic;
		Core::float32 roughness;
		Math::Vector2 padding;
	};

	struct LightingConstants
	{
		DirectionalLightData dirLights[4];
		Core::uint32 numDirLights;
		Core::uint32 padding0[3];  // 16바이트 정렬

		PointLightData pointLights[8];
		Core::uint32 numPointLights;
		Core::uint32 padding1[3];  // 16바이트 정렬

		Math::Vector3 viewPos;
		Core::float32 padding2;
	};


	// TODO: 향후 렌더러 설정을 커스터마이징하고 싶다면
	//struct RendererDesc
	//{
	//	bool enableShadows = true;
	//	bool enablePostProcessing = false;
	//	uint32 maxLights = 128;
	//	uint32 shadowMapSize = 2048;
	//};

	//bool DX12Renderer::Initialize(
	//	DX12Device* device,
	//	uint32 width,
	//	uint32 height,
	//	const RendererDesc& desc = {})
	//{
	//	// desc를 사용한 커스터마이징
	//}


	// 전방 선언
	class DX12Device;
	class DX12SwapChain;
	class DX12CommandQueue;
	class DX12CommandContext;
	class DX12RootSignature;
	class DX12PipelineStateCache;
	class DX12ShaderCompiler;
	class DX12ConstantBuffer;
	class DX12DepthStencilBuffer;
	class DX12DescriptorHeap;

	/**
	 * @brief DirectX 12 기반 렌더러 클래스
	 *
	 * 실제 렌더링을 담당하는 핵심 클래스
	 * Scene에서 수집한 FrameData를 받아 GPU에 제출
	 *
	 * Phase 3.3: Lighting System 통합
	 * - LightingConstantBuffer 추가 (b2)
	 * - MaterialConstantBuffer 추가 (b1)
	 */
	class DX12Renderer
	{
	public:
		DX12Renderer();
		~DX12Renderer();

		DX12Renderer(const DX12Renderer&) = delete;
		DX12Renderer& operator=(const DX12Renderer&) = delete;

		bool Initialize(
			DX12Device* device,
			Core::uint32 width,
			Core::uint32 height
		);

		bool CreateDefaultRootSignature();

		void Shutdown();

		/**
		 * @brief 윈도우 크기 변경 처리
		 */
		void OnResize(Core::uint32 width, Core::uint32 height);

		/**
		 * @brief 프레임 렌더링 (메인 진입점)
		 *
		 * @param frameData Scene에서 수집한 렌더링 데이터
		 * @param resourceMgr ResourceManager (텍스처 조회용)
		 */
		void RenderFrame(const FrameData& frameData);

		void MoveFrameIndex() { mCurrentFrameIndex = (mCurrentFrameIndex + 1) % FRAME_BUFFER_COUNT; }

		void SetCurrentFrameFenceValue(Core::uint64 value) { mFrameFenceValues[mCurrentFrameIndex] = value; }

		// Getters
		Core::uint32 GetCurrentFrameIndex() const { return mCurrentFrameIndex; }
		Core::uint64 GetCurrentFrameFenceValue() const { return mFrameFenceValues[mCurrentFrameIndex]; }

		DX12DescriptorHeap* GetSrvDescriptorHeap() { return mSrvDescriptorHeap.get(); }
		DX12ShaderCompiler* GetShaderCompiler() { return mShaderCompiler.get(); }
	private:
		// 렌더링 파이프라인 단계
		bool BeginFrame();
		void Clear(const float* clearColor);
		void SetupPipeline();

		/**
		 * @brief 렌더 아이템 그리기
		 *
		 * @param items 렌더링할 아이템 목록
		 * @param resourceMgr ResourceManager (텍스처 조회용)
		 */
		void DrawRenderItems(const std::vector<RenderItem>& items);

		/**
		 * @brief Phase 3.3: Lighting Constant Buffer 업데이트
		 *
		 * @param frameData FrameData (조명 데이터 포함)
		 */
		void UpdateLightingBuffer(const FrameData& frameData);

		void EndFrame();
		void Present(bool vsync);

		// 헬퍼 함수
		ID3D12GraphicsCommandList* GetCurrentCommandList();
		DX12CommandContext* GetCurrentCommandContext();
		void UpdateViewportAndScissor();

		// 디바이스 참조 (소유하지 않음)
		DX12Device* mDevice = nullptr;

		std::unique_ptr<DX12RootSignature> mRootSignature;
		std::unique_ptr<DX12PipelineStateCache> mPipelineStateCache;
		std::unique_ptr<DX12ShaderCompiler> mShaderCompiler;

		// Constant Buffers
		std::unique_ptr<DX12ConstantBuffer> mObjectConstantBuffer;		// b0: MVP
		std::unique_ptr<DX12ConstantBuffer> mMaterialConstantBuffer;    // b1: Material (Phase 3.3)
		std::unique_ptr<DX12ConstantBuffer> mLightingConstantBuffer;    // b2: Lighting (Phase 3.3)

		Core::uint32 mCurrentObjectCBIndex = 0;
		static constexpr Core::uint32 MAX_OBJECTS_PER_FRAME = 100;

		std::unique_ptr<DX12DepthStencilBuffer> mDepthStencilBuffer;
		std::unique_ptr<DX12DescriptorHeap> mSrvDescriptorHeap;

		// 향후 추가될 리소스들
		// std::unique_ptr<DX12ConstantBuffer> mLightConstantBuffer;
		// std::unique_ptr<DX12DescriptorHeap> mSamplerHeap;

		// 렌더 타겟 설정
		Core::uint32 mWidth = 0;
		Core::uint32 mHeight = 0;
		D3D12_VIEWPORT mViewport = {};
		D3D12_RECT mScissorRect = {};
		float mClearColor[4] = { 0.392f, 0.584f, 0.929f, 1.0f };  // Cornflower Blue

		// 프레임 동기화
		std::array<Core::uint64, FRAME_BUFFER_COUNT> mFrameFenceValues = { 0 };
		Core::uint32 mCurrentFrameIndex = 0;

		// 상태
		bool mIsInitialized = false;
	};

} // namespace Graphics
