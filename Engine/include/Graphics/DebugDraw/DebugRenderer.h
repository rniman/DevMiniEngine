// Engine/include/Graphics/DebugDraw/DebugRenderer.h
#pragma once
#include "Graphics/DebugDraw/DebugTypes.h"
#include "Graphics/GraphicsTypes.h"
#include "ECS/Entity.h"
#include "Math/MathTypes.h"
#include <memory>
#include <unordered_set>

namespace Graphics
{
	class DX12Device;
	class DX12ShaderCompiler;
	class DX12ConstantBuffer;
	class DX12VertexBuffer;
	struct FrameData;

	/**
	 * @brief Debug 시각화 렌더러
	 *
	 * Light Gizmo 등 디버그 렌더링을 담당합니다.
	 * 메인 렌더링과 독립된 Root Signature, PSO를 사용합니다.
	 *
	 * @note 표시 모드
	 * - All: 모든 Gizmo 표시
	 * - SelectedOnly: 선택된 Entity만 표시
	 * - None: 모두 숨김
	 */
	class DebugRenderer
	{
	public:
		DebugRenderer() = default;
		~DebugRenderer();

		DebugRenderer(const DebugRenderer&) = delete;
		DebugRenderer& operator=(const DebugRenderer&) = delete;

		//=====================================================================
		// 생명주기
		//=====================================================================

		bool Initialize(DX12Device* device, DX12ShaderCompiler* shaderCompiler);
		void Shutdown();

		void Render(
			ID3D12GraphicsCommandList* cmdList,
			const FrameData& frameData,
			const Math::Matrix4x4& viewMatrix,
			const Math::Matrix4x4& projMatrix
		);

		//=====================================================================
		// 설정
		//=====================================================================

		DebugRendererSettings& GetSettings() { return mSettings; }
		const DebugRendererSettings& GetSettings() const { return mSettings; }

		//=====================================================================
		// Entity 선택 (ECSInspector 연동)
		//=====================================================================

		void SetSelectedEntity(ECS::Entity entity) { mSelectedEntity = entity; }
		ECS::Entity GetSelectedEntity() const { return mSelectedEntity; }
		void ClearSelectedEntity() { mSelectedEntity = ECS::Entity{}; }

		//=====================================================================
		// Entity 가시성
		//=====================================================================

		void SetEntityVisible(ECS::Entity entity, bool visible);
		bool IsEntityVisible(ECS::Entity entity) const;

		bool IsInitialized() const { return mIsInitialized; }

	private:
		//=====================================================================
		// 초기화
		//=====================================================================

		bool CreateRootSignature();
		bool CreatePipelineStates();
		bool CreateConstantBuffer();
		bool CreateUnitShapes();

		//=====================================================================
		// 렌더링
		//=====================================================================

		void RenderDirectionalLights(
			ID3D12GraphicsCommandList* cmdList,
			const FrameData& frameData,
			const Math::Matrix4x4& viewProj
		);

		void RenderPointLights(
			ID3D12GraphicsCommandList* cmdList,
			const FrameData& frameData,
			const Math::Matrix4x4& viewProj
		);

		void DrawArrow(
			ID3D12GraphicsCommandList* cmdList,
			const Math::Matrix4x4& worldMatrix,
			const Math::Matrix4x4& viewProj,
			const Math::Vector3& color
		);

		void DrawSphere(
			ID3D12GraphicsCommandList* cmdList,
			const Math::Matrix4x4& worldMatrix,
			const Math::Matrix4x4& viewProj,
			const Math::Vector3& color
		);

	private:
		//=====================================================================
		// 상태
		//=====================================================================

		DebugRendererSettings mSettings;
		ECS::Entity mSelectedEntity;
		std::unordered_set<ECS::Entity> mHiddenEntities;

		//=====================================================================
		// DX12 리소스
		//=====================================================================

		DX12Device* mDevice = nullptr;
		DX12ShaderCompiler* mShaderCompiler = nullptr;

		ComPtr<ID3D12RootSignature> mRootSignature;
		ComPtr<ID3D12PipelineState> mPSODepthOn;
		ComPtr<ID3D12PipelineState> mPSODepthOff;

		std::unique_ptr<DX12ConstantBuffer> mConstantBuffer;
		std::unique_ptr<DX12VertexBuffer> mArrowBuffer;
		std::unique_ptr<DX12VertexBuffer> mSphereBuffer;

		//=====================================================================
		// 프레임 상태
		//=====================================================================

		Core::uint32 mCurrentFrameIndex = 0;
		Core::uint32 mCurrentDrawIndex = 0;

		static constexpr Core::uint32 MAX_DEBUG_DRAWS_PER_FRAME = 4096;
		static constexpr size_t ALIGNED_CB_SIZE = 256;

		bool mIsInitialized = false;
	};

} // namespace Graphics
