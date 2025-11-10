#pragma once
#include "Graphics/GraphicsTypes.h"
#include "Math/MathTypes.h"
#include "Math/MathUtils.h"
#include <vector>

namespace Graphics
{
	// 전방 선언
	class Mesh;
	class Material;

	/**
	 * @brief 단일 렌더링 아이템 (Draw Call 단위)
	 */
	struct RenderItem
	{
		const Mesh* mesh = nullptr;
		const Material* material = nullptr;
		Math::Matrix4x4 worldMatrix = Math::MatrixIdentity();
		Math::Matrix4x4 mvpMatrix = Math::MatrixIdentity();  // 미리 계산된 MVP (셰이더 전달용)

		// 향후 확장
		// Core::uint32 stencilRef = 0;
		// RenderLayer layer = RenderLayer::Opaque;
	};

	/**
	 * @brief 프레임별 렌더링 데이터
	 *
	 * Scene에서 수집한 렌더링 정보를 Renderer에 전달하는 구조체
	 */
	struct FrameData
	{
		// 카메라 정보
		Math::Matrix4x4 viewMatrix = Math::MatrixIdentity();
		Math::Matrix4x4 projectionMatrix = Math::MatrixIdentity();
		Math::Vector3 cameraPosition = { 0.0f, 0.0f, 0.0f };

		// 렌더 아이템 (렌더링 순서별로 분류)
		std::vector<RenderItem> opaqueItems;      // 불투명 오브젝트
		std::vector<RenderItem> transparentItems;  // 투명 오브젝트 (향후)

		// 향후 확장
		// std::vector<Light> lights;
		// std::vector<ShadowCaster> shadowCasters;
		// EnvironmentMap* environmentMap = nullptr;

		void Clear()
		{
			opaqueItems.clear();
			transparentItems.clear();
		}
	};

	/**
	 * @brief 렌더 레이어 (향후 확장)
	 */
	enum class RenderLayer : Core::uint8
	{
		Opaque = 0,
		Transparent = 1,
		UI = 2,
		Debug = 3
	};

} // namespace Graphics
