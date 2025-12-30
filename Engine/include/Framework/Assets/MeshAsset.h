/**
 * @file MeshAsset.h
 * @brief 메시 데이터를 담는 Asset 클래스
 *
 * 파일에서 로드된 정점, 인덱스 데이터를 보유합니다.
 * ResourceManager를 통해 GPU Mesh로 변환됩니다.
 *
 * @note Phase 4.2: 실제 데이터 저장 구현
 */
#pragma once
#include "Framework/Assets/IAsset.h"
#include "Graphics/VertexTypes.h"
#include "Math/MathTypes.h"
#include "Core/Types.h"
#include <string>
#include <vector>

namespace Framework
{
	//=========================================================================
	// 서브메시 정보
	//=========================================================================

	/**
	 * @brief 서브메시 정보
	 *
	 * 하나의 MeshAsset이 여러 머티리얼을 사용할 때 각 부분을 정의합니다.
	 */
	struct SubmeshInfo
	{
		Core::uint32 startIndex = 0;      // 인덱스 버퍼 시작 위치
		Core::uint32 indexCount = 0;      // 인덱스 개수
		Core::uint32 baseVertex = 0;      // 정점 버퍼 시작 위치 (DrawIndexedInstanced용)
		Core::uint32 materialIndex = 0;   // 머티리얼 인덱스
	};

	//=========================================================================
	// 메시 데이터 정책
	//=========================================================================

	/**
	 * @brief 메시 CPU 데이터 유지 정책
	 */
	enum class MeshDataPolicy : Core::uint8
	{
		ReleaseAfterUpload,  // 기본값: GPU 업로드 후 해제
		KeepSourceData       // Physics/Raycast용 유지
	};

	//=========================================================================
	// MeshAsset 클래스
	//=========================================================================

	/**
	 * @brief 메시 Asset 클래스
	 *
	 * glTF 등에서 로드된 메시의 CPU 측 데이터를 보유합니다.
	 * 정점 위치, 노말, UV, 탄젠트 및 인덱스 데이터를 포함합니다.
	 *
	 * @note ResourceManager::CreateMeshFromAsset()으로 GPU Mesh 생성
	 * @note 기본적으로 GPU 업로드 후 CPU 데이터 해제
	 */
	class MeshAsset : public IAsset
	{
	public:
		MeshAsset();
		~MeshAsset() override = default;

		//=========================================================================
		// IAsset 인터페이스 구현
		//=========================================================================

		AssetType GetType() const override { return AssetType::Mesh; }
		const std::string& GetPath() const override { return mPath; }
		AssetState GetState() const override { return mState; }
		Core::size_t GetMemoryUsage() const override;

		//=========================================================================
		// 정점/인덱스 데이터 접근
		//=========================================================================

		/** @brief 정점 배열 반환 */
		const std::vector<Graphics::StandardVertex>& GetVertices() const { return mVertices; }

		/** @brief 인덱스 배열 반환 */
		const std::vector<Core::uint32>& GetIndices() const { return mIndices; }

		/** @brief 서브메시 배열 반환 */
		const std::vector<SubmeshInfo>& GetSubmeshes() const { return mSubmeshes; }

		/** @brief 정점 개수 (ReleaseSourceData 후에도 유효) */
		Core::uint32 GetVertexCount() const { return mVertexCount; }

		/** @brief 인덱스 개수 (ReleaseSourceData 후에도 유효) */
		Core::uint32 GetIndexCount() const { return mIndexCount; }

		/** @brief 서브메시 개수 */
		Core::uint32 GetSubmeshCount() const { return static_cast<Core::uint32>(mSubmeshes.size()); }

		/** @brief 특정 서브메시 정보 반환 */
		const SubmeshInfo& GetSubmesh(Core::uint32 index) const;

		//=========================================================================
		// 바운딩 정보
		//=========================================================================

		/** @brief AABB 최소점 */
		const Math::Vector3& GetAABBMin() const { return mAABBMin; }

		/** @brief AABB 최대점 */
		const Math::Vector3& GetAABBMax() const { return mAABBMax; }

		/** @brief AABB 중심점 */
		Math::Vector3 GetAABBCenter() const;

		/** @brief AABB 크기 (extent) */
		Math::Vector3 GetAABBExtent() const;

		/** @brief 바운딩 구 중심 */
		const Math::Vector3& GetBoundingSphereCenter() const { return mBoundingSphereCenter; }

		/** @brief 바운딩 구 반지름 */
		Core::float32 GetBoundingSphereRadius() const { return mBoundingSphereRadius; }

		//=========================================================================
		// 인덱스 포맷 정보
		//=========================================================================

		/**
		 * @brief 16비트 인덱스 사용 가능 여부
		 * @return 정점 수가 65535 이하면 true
		 */
		bool CanUse16BitIndices() const { return mVertexCount <= 65535; }

		/**
		 * @brief 16비트 인덱스 배열 생성
		 * @return 변환된 16비트 인덱스 배열 (정점 수 초과 시 빈 배열)
		 */
		std::vector<Core::uint16> GetIndices16() const;

		//=========================================================================
		// 메모리 관리
		//=========================================================================

		/**
		 * @brief 데이터 정책 설정
		 * @param policy 유지 정책
		 * @note GPU 업로드 전에 설정해야 함
		 */
		void SetDataPolicy(MeshDataPolicy policy) { mDataPolicy = policy; }

		/** @brief 현재 데이터 정책 */
		MeshDataPolicy GetDataPolicy() const { return mDataPolicy; }

		/**
		 * @brief CPU 데이터 해제
		 *
		 * GPU 업로드 완료 후 메모리 절약을 위해 호출합니다.
		 * KeepSourceData 정책인 경우 호출해도 무시됩니다.
		 */
		void ReleaseSourceData();

		/** @brief CPU 데이터 존재 여부 */
		bool HasSourceData() const { return !mSourceDataReleased; }

		//=========================================================================
		// 데이터 설정 (AssetManager/ModelLoader 전용)
		//=========================================================================

		/**
		 * @brief 정점 데이터 설정
		 * @param vertices 정점 배열 (이동)
		 */
		void SetVertices(std::vector<Graphics::StandardVertex>&& vertices);

		/**
		 * @brief 인덱스 데이터 설정
		 * @param indices 인덱스 배열 (이동)
		 */
		void SetIndices(std::vector<Core::uint32>&& indices);

		/**
		 * @brief 서브메시 데이터 설정
		 * @param submeshes 서브메시 배열 (이동)
		 */
		void SetSubmeshes(std::vector<SubmeshInfo>&& submeshes);

		/**
		 * @brief AABB 설정
		 * @param min 최소점
		 * @param max 최대점
		 */
		void SetAABB(const Math::Vector3& min, const Math::Vector3& max);

		/**
		 * @brief 바운딩 구 계산 (AABB 기반)
		 *
		 * SetAABB() 호출 후 자동으로 호출됩니다.
		 */
		void CalculateBoundingSphere();

	private:
		friend class AssetManager;

		std::string mPath;
		AssetState mState = AssetState::Unloaded;

		// 정점/인덱스 데이터
		std::vector<Graphics::StandardVertex> mVertices;
		std::vector<Core::uint32> mIndices;
		std::vector<SubmeshInfo> mSubmeshes;

		// 캐시된 카운트 (ReleaseSourceData 후에도 유효)
		Core::uint32 mVertexCount = 0;
		Core::uint32 mIndexCount = 0;

		// 바운딩 정보
		Math::Vector3 mAABBMin = Math::Vector3(0.0f, 0.0f, 0.0f);
		Math::Vector3 mAABBMax = Math::Vector3(0.0f, 0.0f, 0.0f);
		Math::Vector3 mBoundingSphereCenter = Math::Vector3(0.0f, 0.0f, 0.0f);
		Core::float32 mBoundingSphereRadius = 0.0f;

		// 데이터 정책
		MeshDataPolicy mDataPolicy = MeshDataPolicy::ReleaseAfterUpload;
		bool mSourceDataReleased = false;
	};

} // namespace Framework
