#pragma once
#include "Graphics/GraphicsTypes.h"
#include "Math/MathTypes.h"
#include "Math/MathUtils.h"
#include "ECS/Entity.h" 
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
		Math::Matrix4x4 worldMatrix = Math::Matrix4x4::Identity();
		Math::Matrix4x4 mvpMatrix = Math::Matrix4x4::Identity();
	};

	/**
	 * @brief GPU 전달용 Directional Light 데이터
	 *
	 * Shader Constant Buffer 레이아웃과 일치 (16바이트 정렬)
	 */
	struct DirectionalLightData
	{
		Math::Vector4 direction;
		Math::Vector3 color;
		Core::float32 intensity;
	};

	/**
	 * @brief GPU 전달용 Point Light 데이터
	 *
	 * Shader Constant Buffer 레이아웃과 일치 (16바이트 정렬)
	 */
	struct PointLightData
	{
		Math::Vector4 position;
		Math::Vector4 rangeAndColor;
		Math::Vector4 intensityAndAttenuation;
	};

	/**
	 * @brief Debug 시각화용 Entity 정보
	 *
	 * Phase 3.6: Light Gizmo 렌더링을 위한 Entity 목록
	 * 인덱스가 directionalLights, pointLights 배열과 1:1 매칭됨
	 */
	struct DebugInfo
	{
		std::vector<ECS::Entity> directionalLightEntities;
		std::vector<ECS::Entity> pointLightEntities;

		void Clear()
		{
			directionalLightEntities.clear();
			pointLightEntities.clear();
		}
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
		std::vector<RenderItem> opaqueItems;
		std::vector<RenderItem> transparentItems;

		// 조명 데이터
		std::vector<DirectionalLightData> directionalLights;
		std::vector<PointLightData> pointLights;

		// Debug 시각화 데이터
		DebugInfo debug;

		void Clear()
		{
			opaqueItems.clear();
			transparentItems.clear();
			directionalLights.clear();
			pointLights.clear();
			debug.Clear();
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
