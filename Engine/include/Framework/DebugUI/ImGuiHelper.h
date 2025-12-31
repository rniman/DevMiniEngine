/**
 * @file ImGuiHelper.h
 * @brief ImGui 유틸리티 함수 및 색상 상수
 *
 * Debug UI 전반에서 사용되는 공통 헬퍼 함수를 제공합니다.
 *
 * @note Phase 4.2: ModelViewerApp 리팩토링에서 추출
 */
#pragma once
#include "Math/MathTypes.h"
#include <imgui.h>

namespace Framework
{
	//=========================================================================
	// 색상 상수
	//=========================================================================

	namespace UIColor
	{
		inline const ImVec4 Error = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);   // 빨강
		inline const ImVec4 Warning = ImVec4(1.0f, 0.8f, 0.3f, 1.0f);   // 노랑
		inline const ImVec4 Success = ImVec4(0.3f, 1.0f, 0.3f, 1.0f);   // 초록
		inline const ImVec4 Info = ImVec4(0.5f, 0.5f, 1.0f, 1.0f);   // 파랑
		inline const ImVec4 Disabled = ImVec4(0.7f, 0.7f, 0.7f, 1.0f);   // 회색
		inline const ImVec4 Highlight = ImVec4(0.3f, 0.8f, 1.0f, 1.0f);   // 하늘색
		inline const ImVec4 White = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	//=========================================================================
	// 텍스트 헬퍼
	//=========================================================================

	/**
	 * @brief 상태에 따른 색상 텍스트 출력
	 *
	 * @param label 라벨 (앞에 출력)
	 * @param isOk 상태 (true: 성공, false: 실패)
	 * @param okText 성공 시 텍스트
	 * @param failText 실패 시 텍스트
	 */
	inline void ImGuiTextStatus(
		const char* label,
		bool isOk,
		const char* okText = "OK",
		const char* failText = "Failed")
	{
		ImGui::Text("%s: ", label);
		ImGui::SameLine();
		if (isOk)
		{
			ImGui::TextColored(UIColor::Success, "%s", okText);
		}
		else
		{
			ImGui::TextColored(UIColor::Error, "%s", failText);
		}
	}

	/**
	 * @brief Vector2 텍스트 출력
	 */
	inline void ImGuiTextVector2(const char* label, const Math::Vector2& v)
	{
		ImGui::Text("%s: (%.2f, %.2f)", label, v.x, v.y);
	}

	/**
	 * @brief Vector3 텍스트 출력
	 */
	inline void ImGuiTextVector3(const char* label, const Math::Vector3& v)
	{
		ImGui::Text("%s: (%.2f, %.2f, %.2f)", label, v.x, v.y, v.z);
	}

	/**
	 * @brief Vector4 텍스트 출력
	 */
	inline void ImGuiTextVector4(const char* label, const Math::Vector4& v)
	{
		ImGui::Text("%s: (%.2f, %.2f, %.2f, %.2f)", label, v.x, v.y, v.z, v.w);
	}

	//=========================================================================
	// 레이아웃 헬퍼
	//=========================================================================

	/**
	 * @brief 라벨 + 값 정렬된 출력
	 *
	 * @param label 라벨
	 * @param format printf 포맷
	 * @param ... 가변 인자
	 */
	inline void ImGuiLabelValue(const char* label, const char* format, ...)
	{
		ImGui::Text("%s:", label);
		ImGui::SameLine(120);  // 고정 위치

		va_list args;
		va_start(args, format);
		ImGui::TextV(format, args);
		va_end(args);
	}

	/**
	 * @brief 구분선 + 제목
	 */
	inline void ImGuiSectionHeader(const char* title)
	{
		ImGui::Separator();
		ImGui::TextColored(UIColor::Info, "%s", title);
		ImGui::Separator();
	}

	//=========================================================================
	// 입력 헬퍼
	//=========================================================================

	/**
	 * @brief 범위 제한 float 슬라이더 (변경 여부 반환)
	 */
	inline bool ImGuiSliderFloatClamped(
		const char* label,
		float* value,
		float minVal,
		float maxVal,
		const char* format = "%.2f")
	{
		bool changed = ImGui::SliderFloat(label, value, minVal, maxVal, format);
		if (*value < minVal) *value = minVal;
		if (*value > maxVal) *value = maxVal;
		return changed;
	}

	/**
	 * @brief 범위 제한 int 슬라이더 (변경 여부 반환)
	 */
	inline bool ImGuiSliderIntClamped(
		const char* label,
		int* value,
		int minVal,
		int maxVal)
	{
		bool changed = ImGui::SliderInt(label, value, minVal, maxVal);
		if (*value < minVal) *value = minVal;
		if (*value > maxVal) *value = maxVal;
		return changed;
	}

	//=========================================================================
	// 윈도우 헬퍼
	//=========================================================================

	/**
	 * @brief 조건부 CollapsingHeader (Early return 패턴용)
	 *
	 * @code
	 * if (ImGuiBeginSection("Materials"))
	 * {
	 *     // 내용
	 * }
	 * @endcode
	 */
	inline bool ImGuiBeginSection(const char* label, bool defaultOpen = true)
	{
		ImGuiTreeNodeFlags flags = defaultOpen ? ImGuiTreeNodeFlags_DefaultOpen : 0;
		return ImGui::CollapsingHeader(label, flags);
	}

} // namespace Framework
