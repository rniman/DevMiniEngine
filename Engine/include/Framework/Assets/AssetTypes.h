/**
 * @file AssetTypes.h
 * @brief Asset 시스템의 기본 타입 및 열거형 정의
 *
 * Asset 타입, 상태, 정보 구조체 및 유틸리티 함수를 제공합니다.
 */
#pragma once
#include "Core/Types.h"
#include <string>
#include <unordered_map>

namespace Framework
{
	/**
	 * @brief Asset의 종류를 구분하는 열거형
	 *
	 * 각 Asset 타입은 파일 확장자와 매핑됩니다.
	 * 예: Model(.gltf, .glb), Texture(.dds, .png)
	 */
	enum class AssetType : Core::uint8
	{
		Mesh = 0,		// 단일 메시 데이터
		Texture,		// 텍스처 이미지
		Material,		// 머티리얼 정의
		Model,			// glTF 모델 (메시 + 머티리얼 + 계층구조)
		Animation,		// 애니메이션 데이터 (Phase 6+)

		Count,
		Unknown
	};

	/**
	 * @brief Asset의 로딩 상태를 나타내는 열거형
	 *
	 * 비동기 로딩을 지원하기 위한 상태 머신입니다.
	 */
	enum class AssetState : Core::uint8
	{
		Unloaded = 0,	// 초기 상태 또는 해제됨
		Queued,			// 로드 요청됨, 대기 중
		Loading,		// 로딩 진행 중
		Loaded,			// 완료, 사용 가능
		Failed			// 로드 실패
	};

	/**
	 * @brief Asset 정보 구조체 (디버그/UI용)
	 */
	struct AssetInfo
	{
		std::string path;
		AssetType type = AssetType::Unknown;
		AssetState state = AssetState::Unloaded;
		Core::size_t memoryUsage = 0;
		Core::uint32 refCount = 0;
	};

	//=========================================================================
	// 유틸리티 함수
	//=========================================================================

	/**
	 * @brief AssetType을 문자열로 변환
	 * @param type Asset 타입
	 * @return 타입 이름 문자열
	 */
	inline constexpr const char* AssetTypeToString(AssetType type)
	{
		switch (type)
		{
		case AssetType::Mesh:
			return "Mesh";
		case AssetType::Texture:
			return "Texture";
		case AssetType::Material:
			return "Material";
		case AssetType::Model:
			return "Model";
		case AssetType::Animation:
			return "Animation";
		default:
			return "Unknown";
		}
	}

	/**
	 * @brief AssetState를 문자열로 변환
	 * @param state Asset 상태
	 * @return 상태 이름 문자열
	 */
	inline constexpr const char* AssetStateToString(AssetState state)
	{
		switch (state)
		{
		case AssetState::Unloaded:
			return "Unloaded";
		case AssetState::Queued:
			return "Queued";
		case AssetState::Loading:
			return "Loading";
		case AssetState::Loaded:
			return "Loaded";
		case AssetState::Failed:
			return "Failed";
		default:
			return "Unknown";
		}
	}


	/**
	 * @brief 파일 확장자에서 AssetType 추론
	 * @param extension 파일 확장자 (예: ".gltf", ".dds")
	 * @return 추론된 AssetType, 알 수 없으면 Unknown
	 */
	inline AssetType GetAssetTypeFromExtension(const std::string& extension)
	{
		// 소문자 변환
		std::string ext = extension;
		for (char& c : ext)
		{
			if (c >= 'A' && c <= 'Z')
			{
				c = c + ('a' - 'A');
			}
		}

		// 확장자 매핑 테이블
		static const std::unordered_map<std::string, AssetType> sExtensionMap = {
			// Model
			{ ".gltf", AssetType::Model },
			{ ".glb",  AssetType::Model },

			// Texture
			{ ".dds",  AssetType::Texture },
			{ ".png",  AssetType::Texture },
			{ ".jpg",  AssetType::Texture },
			{ ".jpeg", AssetType::Texture },
			{ ".tga",  AssetType::Texture },
			{ ".bmp",  AssetType::Texture },

			// Material (향후 확장)
			{ ".mat",  AssetType::Material },
			{ ".json", AssetType::Material },
		};

		auto it = sExtensionMap.find(ext);
		if (it != sExtensionMap.end())
		{
			return it->second;
		}

		return AssetType::Unknown;
	}

	/**
	 * @brief 파일 경로에서 확장자 추출
	 * @param path 파일 경로
	 * @return 확장자 문자열 (점 포함, 예: ".gltf")
	 */
	inline std::string GetFileExtension(const std::string& path)
	{
		size_t dotPos = path.rfind('.');
		if (dotPos != std::string::npos)
		{
			return path.substr(dotPos);
		}
		return "";
	}

	/**
	 * @brief 파일 경로에서 AssetType 추론
	 * @param path 파일 경로
	 * @return 추론된 AssetType
	 */
	inline AssetType GetAssetTypeFromPath(const std::string& path)
	{
		return GetAssetTypeFromExtension(GetFileExtension(path));
	}

} // namespace Framework
