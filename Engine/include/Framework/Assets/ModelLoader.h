/**
 * @file ModelLoader.h
 * @brief Assimp 기반 모델 로더
 *
 * glTF 2.0 모델 파일을 로드하여 엔진에서 사용 가능한 형태로 변환합니다.
 * 메시, 머티리얼, 텍스처, 계층 구조 정보를 추출합니다.
 *
 * @note Phase 4.2: Model Loading
 */
#pragma once
#include "Framework/Assets/MeshAsset.h"
#include "Core/Types.h"
#include "Math/MathTypes.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/TextureType.h"
#include <string>
#include <vector>

namespace Framework
{
	// 전방 선언
	class ModelAsset;
	class AssetManager;

	//=========================================================================
	// 텍스처 정보
	//=========================================================================

	/**
	 * @brief 로드된 텍스처 정보
	 */
	struct LoadedTextureInfo
	{
		std::string path;                                      // 텍스처 파일 경로 (상대 경로)
		Graphics::TextureType type = Graphics::TextureType::Albedo;  // 텍스처 용도
	};

	//=========================================================================
	// 머티리얼 정보
	//=========================================================================

	/**
	 * @brief 로드된 머티리얼 데이터 (PBR)
	 */
	struct LoadedMaterialData
	{
		std::string name;

		// PBR 기본값
		Math::Vector4 baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
		Core::float32 metallicFactor = 0.0f;
		Core::float32 roughnessFactor = 1.0f;
		Math::Vector3 emissiveFactor = { 0.0f, 0.0f, 0.0f };

		// 텍스처 목록
		std::vector<LoadedTextureInfo> textures;

		// 텍스처 존재 여부 헬퍼
		bool HasTexture(Graphics::TextureType type) const
		{
			for (const auto& tex : textures)
			{
				if (tex.type == type)
				{
					return true;
				}
			}
			return false;
		}

		// 특정 타입 텍스처 경로 반환
		const std::string* GetTexturePath(Graphics::TextureType type) const
		{
			for (const auto& tex : textures)
			{
				if (tex.type == type)
				{
					return &tex.path;
				}
			}
			return nullptr;
		}
	};

	//=========================================================================
	// 메시 데이터
	//=========================================================================

	/**
	 * @brief 로드된 메시 데이터 (CPU 측)
	 *
	 * @note SubmeshInfo는 MeshAsset.h에 정의됨 (단일 정의 원칙)
	 */
	struct LoadedMeshData
	{
		std::string name;
		std::vector<Graphics::StandardVertex> vertices;
		std::vector<Core::uint32> indices;
		std::vector<SubmeshInfo> submeshes;  // MeshAsset.h의 SubmeshInfo 사용

		// 바운딩 정보
		Math::Vector3 aabbMin = Math::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
		Math::Vector3 aabbMax = Math::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		void Clear()
		{
			name.clear();
			vertices.clear();
			indices.clear();
			submeshes.clear();
			aabbMin = Math::Vector3(FLT_MAX, FLT_MAX, FLT_MAX);
			aabbMax = Math::Vector3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
		}

		bool IsEmpty() const { return vertices.empty(); }
	};

	//=========================================================================
	// 노드 정보
	//=========================================================================

	/**
	 * @brief 노드 정보 (계층 구조용)
	 */
	struct LoadedNodeInfo
	{
		std::string name;
		Math::Matrix4x4 localTransform;
		Core::int32 parentIndex = -1;     // -1이면 루트
		Core::int32 meshIndex = -1;       // -1이면 메시 없음
	};

	//=========================================================================
	// 모델 데이터 (전체)
	//=========================================================================

	/**
	 * @brief 로드된 모델 데이터 (전체)
	 */
	struct LoadedModelData
	{
		std::string name;
		std::vector<LoadedMeshData> meshes;
		std::vector<LoadedMaterialData> materials;
		std::vector<LoadedNodeInfo> nodes;

		void Clear()
		{
			name.clear();
			meshes.clear();
			materials.clear();
			nodes.clear();
		}

		bool IsEmpty() const { return meshes.empty(); }

		// 통계
		Core::uint32 GetTotalVertexCount() const
		{
			Core::uint32 count = 0;
			for (const auto& mesh : meshes)
			{
				count += static_cast<Core::uint32>(mesh.vertices.size());
			}
			return count;
		}

		Core::uint32 GetTotalIndexCount() const
		{
			Core::uint32 count = 0;
			for (const auto& mesh : meshes)
			{
				count += static_cast<Core::uint32>(mesh.indices.size());
			}
			return count;
		}

		Core::uint32 GetTotalTextureCount() const
		{
			Core::uint32 count = 0;
			for (const auto& mat : materials)
			{
				count += static_cast<Core::uint32>(mat.textures.size());
			}
			return count;
		}
	};

	//=========================================================================
	// ModelLoader 클래스
	//=========================================================================

	/**
	 * @brief Assimp 기반 모델 로더
	 *
	 * glTF 2.0 파일을 로드하여 엔진에서 사용 가능한 형태로 변환합니다.
	 * Tangent는 MikkTSpace로 계산합니다.
	 *
	 * @example
	 * // 단일 메시 로드
	 * LoadedMeshData meshData;
	 * if (ModelLoader::LoadMesh("Assets/Models/Box.glb", meshData))
	 * {
	 *     // meshData.vertices, meshData.indices 사용
	 * }
	 *
	 * @example
	 * // 전체 모델 로드 (메시 + 머티리얼 + 텍스처 경로)
	 * LoadedModelData modelData;
	 * if (ModelLoader::LoadModel("Assets/Models/Helmet.glb", modelData))
	 * {
	 *     // modelData.meshes, modelData.materials 사용
	 * }
	 */
	class ModelLoader
	{
	public:
		/**
		 * @brief 단일 메시 로드 (첫 번째 메시만)
		 *
		 * 간단한 모델 로딩에 사용합니다.
		 * 머티리얼/텍스처 정보는 포함되지 않습니다.
		 *
		 * @param filePath 모델 파일 경로
		 * @param outMeshData 출력 메시 데이터
		 * @return 성공 여부
		 */
		static bool LoadMesh(const std::string& filePath, LoadedMeshData& outMeshData);

		/**
		 * @brief 전체 모델 로드 (모든 메시 + 머티리얼 + 텍스처 경로 + 계층 구조)
		 *
		 * @param filePath 모델 파일 경로
		 * @param outModelData 출력 모델 데이터
		 * @return 성공 여부
		 */
		static bool LoadModel(const std::string& filePath, LoadedModelData& outModelData);

	private:
		ModelLoader() = delete;
	};

} // namespace Framework
