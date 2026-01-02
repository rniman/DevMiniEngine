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
		Albedo = 0,           // 기본 색상 텍스처 (t0)
		Normal,               // 노말 맵 (t1)
		Specular,             // 스페큘러 맵 (t2)
		Roughness,            // 거칠기 맵 (t3) - PBR용
		Metallic,             // 메탈릭 맵 (t4) - PBR용
		AmbientOcclusion,     // AmbientOcclusion 맵 (t5)
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
		case TextureType::Albedo:
			return "Albedo";
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

	/**
	 * @brief 텍스처 타입이 sRGB 색공간을 사용하는지 여부
	 *
	 * sRGB 텍스처: 아티스트가 모니터에서 색상을 조정한 데이터
	 * Linear 텍스처: 물리적 수치 또는 방향 벡터 데이터
	 *
	 * 조명 계산은 Linear 공간에서 수행해야 물리적으로 정확합니다.
	 * GPU가 sRGB 텍스처 샘플링 시 자동으로 Linear로 변환합니다.
	 *
	 * @param type 텍스처 타입
	 * @return sRGB 색공간 사용 시 true, Linear 사용 시 false
	 */
	inline constexpr bool IsSRGBTexture(TextureType type)
	{
		switch (type)
		{
		case TextureType::Albedo:
		case TextureType::Emissive:
			return true;

		case TextureType::Normal:
		case TextureType::Specular:
		case TextureType::Roughness:
		case TextureType::Metallic:
		case TextureType::AmbientOcclusion:
		default:
			return false;
		}
	}

} // namespace Graphics
