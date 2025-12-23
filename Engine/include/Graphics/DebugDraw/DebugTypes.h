#pragma once
#include "Graphics/GraphicsTypes.h"
#include "Math/MathTypes.h"

namespace Graphics
{
	/**
	 * @brief Debug Gizmo 표시 모드
	 */
	enum class DebugDisplayMode : Core::uint8
	{
		All,           // 모든 Light Gizmo 표시
		SelectedOnly,  // 선택된 Entity의 Light만 표시
		None           // 모두 숨김
	};

	/**
	 * @brief Debug Constant Buffer (b0)
	 *
	 * Debug 전용 Root Signature에서 사용됩니다.
	 * 256바이트 정렬 필요
	 */
	struct DebugConstants
	{
		Math::Matrix4x4 mvpMatrix;  // 64 bytes
		Math::Vector3 color;        // 12 bytes
		Core::float32 padding;      // 4 bytes
		// Total: 80 bytes → 256 정렬
	};

	/**
	 * @brief Debug 렌더러 설정
	 */
	struct DebugRendererSettings
	{
		//=====================================================================
		// 표시 모드
		//=====================================================================

		DebugDisplayMode displayMode = DebugDisplayMode::All;

		//=====================================================================
		// 개별 토글 (displayMode가 All일 때만 유효)
		//=====================================================================

		bool showDirectionalLights = true;
		bool showPointLights = true;

		//=====================================================================
		// 렌더링 옵션
		//=====================================================================

		bool depthTestEnabled = true;

		//=====================================================================
		// 색상
		//=====================================================================

		Math::Vector3 directionalLightColor = { 1.0f, 0.65f, 0.0f };  // 주황
		Math::Vector3 pointLightColor = { 1.0f, 0.84f, 0.0f };        // 황금
		Math::Vector3 selectedColor = { 0.0f, 1.0f, 0.5f };           // 민트 (선택 강조)
	};

} // namespace Graphics
