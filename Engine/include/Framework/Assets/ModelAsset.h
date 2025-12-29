/**
 * @file ModelAsset.h
 * @brief glTF 모델을 담는 Asset 클래스
 *
 * glTF 파일에서 로드된 전체 모델 데이터를 보유합니다.
 * 여러 Mesh, Material, Texture 및 노드 계층구조를 포함합니다.
 */
#pragma once
#include "Framework/Assets/IAsset.h"
#include "Core/Types.h"
#include <string>
#include <vector>

namespace Framework
{
	// 전방 선언
	class MeshAsset;
	class MaterialAsset;
	class TextureAsset;

	/**
	 * @brief 모델 노드 정보 (계층 구조용)
	 */
	struct ModelNode
	{
		std::string name;
		Core::int32 parentIndex = -1;			// -1이면 루트
		std::vector<Core::uint32> childIndices;

		// 로컬 트랜스폼
		// TODO: Phase 4.2 - Transform 데이터
		// Math::Vector3 translation = Math::Vector3::Zero;
		// Math::Quaternion rotation = Math::Quaternion::Identity;
		// Math::Vector3 scale = Math::Vector3::One;

		// 이 노드가 참조하는 메시 인덱스 (-1이면 없음)
		Core::int32 meshIndex = -1;
	};

	/**
	 * @brief 모델 Asset 클래스 (glTF 컨테이너)
	 *
	 * glTF 파일의 전체 내용을 보유합니다.
	 * - Meshes: 정점/인덱스 데이터
	 * - Materials: PBR 머티리얼 정의
	 * - Textures: 텍스처 데이터
	 * - Nodes: 계층 구조 (선택적)
	 *
	 * @note Mesh는 필수, Material/Texture/Hierarchy는 선택적
	 * @note Phase 4.1: 구조만 정의, 실제 데이터 로딩은 Phase 4.2
	 */
	class ModelAsset : public IAsset
	{
	public:
		ModelAsset();
		~ModelAsset() override = default;

		//=========================================================================
		// IAsset 인터페이스 구현
		//=========================================================================

		AssetType GetType() const override { return AssetType::Model; }
		const std::string& GetPath() const override { return mPath; }
		AssetState GetState() const override { return mState; }
		Core::size_t GetMemoryUsage() const override;

		//=========================================================================
		// ModelAsset 전용 API
		//=========================================================================

		/**
		 * @brief 메시 개수 반환
		 * @return 포함된 메시 수 (최소 1개)
		 */
		Core::uint32 GetMeshCount() const { return static_cast<Core::uint32>(mMeshIndices.size()); }

		/**
		 * @brief 머티리얼 개수 반환
		 * @return 포함된 머티리얼 수 (0일 수 있음)
		 */
		Core::uint32 GetMaterialCount() const { return static_cast<Core::uint32>(mMaterialIndices.size()); }

		/**
		 * @brief 텍스처 개수 반환
		 * @return 포함된 텍스처 수 (0일 수 있음)
		 */
		Core::uint32 GetTextureCount() const { return static_cast<Core::uint32>(mTextureIndices.size()); }

		/**
		 * @brief 노드 개수 반환
		 * @return 계층 구조 노드 수
		 */
		Core::uint32 GetNodeCount() const { return static_cast<Core::uint32>(mNodes.size()); }

		/**
		 * @brief 머티리얼 존재 여부
		 * @return 머티리얼이 1개 이상이면 true
		 */
		bool HasMaterials() const { return !mMaterialIndices.empty(); }

		/**
		 * @brief 텍스처 존재 여부
		 * @return 텍스처가 1개 이상이면 true
		 */
		bool HasTextures() const { return !mTextureIndices.empty(); }

		/**
		 * @brief 계층 구조 존재 여부
		 * @return 노드가 2개 이상이면 true (단일 노드는 계층 아님)
		 */
		bool HasHierarchy() const { return mNodes.size() > 1; }

		// TODO: Phase 4.2 - 개별 Asset 접근
		// const MeshAsset* GetMesh(Core::uint32 index) const;
		// const MaterialAsset* GetMaterial(Core::uint32 index) const;
		// const TextureAsset* GetTexture(Core::uint32 index) const;
		// const ModelNode& GetNode(Core::uint32 index) const;

		// TODO: Phase 4.2 - 루트 노드 접근
		// const std::vector<Core::uint32>& GetRootNodeIndices() const;

	private:
		friend class AssetManager;

		std::string mPath;
		AssetState mState = AssetState::Unloaded;

		// 포함된 Asset들의 인덱스 (AssetManager 캐시 참조용)
		// 실제 Asset 데이터는 AssetManager가 별도 관리
		std::vector<Core::uint32> mMeshIndices;
		std::vector<Core::uint32> mMaterialIndices;
		std::vector<Core::uint32> mTextureIndices;

		// 노드 계층 구조
		std::vector<ModelNode> mNodes;
		std::vector<Core::uint32> mRootNodeIndices;
	};

} // namespace Framework
