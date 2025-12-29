/**
 * @file IAsset.h
 * @brief Asset 시스템의 기본 인터페이스
 *
 * 모든 Asset 클래스는 이 인터페이스를 상속해야 합니다.
 * Asset은 파일 기반의 데이터를 나타내며, GPU Resource와 분리됩니다.
 */
#pragma once
#include "Framework/Assets/AssetTypes.h"
#include "Core/Types.h"
#include <string>

namespace Framework
{
	/**
	 * @brief Asset의 기본 인터페이스
	 *
	 * 모든 Asset(MeshAsset, TextureAsset, ModelAsset 등)은 이 인터페이스를 구현합니다.
	 * Asset은 파일에서 로드된 CPU 측 데이터를 보유하며,
	 * ResourceManager를 통해 GPU Resource로 변환됩니다.
	 *
	 * @note Asset의 생명주기는 AssetManager가 관리합니다.
	 *       직접 생성/삭제하지 마세요.
	 */
	class IAsset
	{
	public:
		virtual ~IAsset() = default;

		//=========================================================================
		// 순수 가상 함수 (파생 클래스에서 구현 필수)
		//=========================================================================

		/**
		 * @brief Asset 타입 반환
		 * @return 이 Asset의 타입 (Mesh, Texture, Material, Model 등)
		 */
		virtual AssetType GetType() const = 0;

		/**
		 * @brief Asset 파일 경로 반환
		 * @return 상대 경로 (예: "Models/Helmet.gltf")
		 */
		virtual const std::string& GetPath() const = 0;

		/**
		 * @brief Asset 로딩 상태 반환
		 * @return 현재 로딩 상태
		 */
		virtual AssetState GetState() const = 0;

		/**
		 * @brief Asset이 사용하는 메모리 크기 반환
		 * @return 메모리 사용량 (바이트)
		 */
		virtual Core::size_t GetMemoryUsage() const = 0;

		//=========================================================================
		// 공통 유틸리티 메서드
		//=========================================================================

		/**
		 * @brief Asset이 로드 완료되었는지 확인
		 * @return Loaded 상태이면 true
		 */
		bool IsLoaded() const { return GetState() == AssetState::Loaded; }

		/**
		 * @brief Asset 로드가 실패했는지 확인
		 * @return Failed 상태이면 true
		 */
		bool IsFailed() const { return GetState() == AssetState::Failed; }

		/**
		 * @brief Asset이 로딩 중인지 확인
		 * @return Queued 또는 Loading 상태이면 true
		 */
		bool IsLoading() const
		{
			AssetState state = GetState();
			return state == AssetState::Queued || state == AssetState::Loading;
		}

		/**
		 * @brief Asset이 유효한지 확인 (로드 완료 또는 로딩 중)
		 * @return Unloaded나 Failed가 아니면 true
		 */
		bool IsValid() const
		{
			AssetState state = GetState();
			return state != AssetState::Unloaded && state != AssetState::Failed;
		}

	protected:
		IAsset() = default;

		// 복사 금지 (AssetManager가 unique_ptr로 소유)
		IAsset(const IAsset&) = delete;
		IAsset& operator=(const IAsset&) = delete;

		// 이동 금지 (포인터로 참조되므로 주소 변경 방지)
		IAsset(IAsset&&) = delete;
		IAsset& operator=(IAsset&&) = delete;
	};

} // namespace Framework
