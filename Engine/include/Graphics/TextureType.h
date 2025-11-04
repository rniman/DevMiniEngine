#pragma once

#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	/**
	 * @brief 텍스처의 용도를 구분하는 열거형
	 *
	 * 각 텍스처 타입은 셰이더의 레지스터 슬롯에 매핑됩니다.
	 * 예: Diffuse는 t0, Normal은 t1, Specular는 t2...
	 */
	enum class TextureType : uint8
	{
		Diffuse = 0,          // 기본 색상 텍스처 (t0)
		Normal,               // 노말 맵 (t1)
		Specular,             // 스페큘러 맵 (t2)
		Roughness,            // 거칠기 맵 (t3) - PBR용
		Metallic,             // 메탈릭 맵 (t4) - PBR용
		AmbientOcclusion,     // AO 맵 (t5)
		Emissive,             // 발광 맵 (t6)

		Count                 // 텍스처 타입 개수 (7개)
	};

	/**
	 * @brief TextureType을 셰이더 레지스터 인덱스로 변환
	 *
	 * @param type 텍스처 타입
	 * @return 레지스터 슬롯 번호 (0~6)
	 */
	inline constexpr uint32 GetTextureRegisterSlot(TextureType type)
	{
		return static_cast<uint32>(type);
	}

	/**
	 * @brief TextureType을 문자열로 변환 (디버깅용)
	 */
	inline constexpr const char* TextureTypeToString(TextureType type)
	{
		switch (type)
		{
		case TextureType::Diffuse:
			return "Diffuse";
		case TextureType::Normal:
			return "Normal";
		case TextureType::Specular:
			return "Specular";
		case TextureType::Roughness:
			return "Roughness";
		case TextureType::Metallic:
			return "Metallic";
		case TextureType::AmbientOcclusion:
			return "AmbientOcclusion";
		case TextureType::Emissive:
			return "Emissive";
		default:
			return "Unknown";
		}
	}

} // namespace Graphics
