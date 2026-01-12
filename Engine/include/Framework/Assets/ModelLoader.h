/**
 * @file ModelLoader.h
 * @brief Assimp 기반 모델 로더
 *
 * glTF 2.0 모델 파일을 로드하여 엔진에서 사용 가능한 형태로 변환합니다.
 * 메시, 머티리얼, 텍스처, 계층 구조 정보를 추출합니다.
 *
 * @note Phase 4.2: Model Loading
 * @note Phase 4.2+: 임베디드 텍스처 지원
 */
#pragma once
#include "Framework/Assets/MeshAsset.h"
#include "Core/Types.h"
#include "Math/MathTypes.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/TextureType.h"
#include "Graphics/SubmeshInfo.h"
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
	 *
	 * 외부 텍스처와 임베디드 텍스처(glb) 모두 지원합니다.
	 *
	 * 외부 텍스처: path에 파일 경로, isEmbedded = false
	 * 임베디드 텍스처: path에 "*0" 형식, embeddedData에 바이너리 데이터
	 */
	struct LoadedTextureInfo
	{
		std::string path;                                           // 파일 경로 또는 "*index" 형식
		Graphics::TextureType type = Graphics::TextureType::Albedo; // 텍스처 용도

		// 임베디드 텍스처 데이터 (Phase 4.2+)
		std::vector<Core::uint8> embeddedData;  // PNG/JPG 바이너리 또는 RGBA 원시 데이터
		Core::uint32 width = 0;                 // 원시 데이터(비압축)일 때 너비
		Core::uint32 height = 0;                // 원시 데이터(비압축)일 때 높이
		bool isEmbedded = false;                // 임베디드 텍스처 여부
		bool isCompressed = true;               // true: PNG/JPG 압축, false: RGBA 원시

		/** @brief 임베디드 데이터 존재 여부 */
		bool HasEmbeddedData() const { return isEmbedded && !embeddedData.empty(); }

		/** @brief 데이터 클리어 */
		void Clear()
		{
			path.clear();
			type = Graphics::TextureType::Albedo;
			embeddedData.clear();
			embeddedData.shrink_to_fit();
			width = 0;
			height = 0;
			isEmbedded = false;
			isCompressed = true;
		}
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
		const std::string& GetTexturePath(Graphics::TextureType type) const
		{
			static const std::string empty;
			for (const auto& tex : textures)
			{
				if (tex.type == type)
				{
					return tex.path;
				}
			}
			return empty;
		}

		// 특정 타입 텍스처 정보 반환 (임베디드 데이터 접근용)
		const LoadedTextureInfo* GetTextureInfo(Graphics::TextureType type) const
		{
			for (const auto& tex : textures)
			{
				if (tex.type == type)
				{
					return &tex;
				}
			}
			return nullptr;
		}

		void Clear()
		{
			name.clear();
			baseColorFactor = { 1.0f, 1.0f, 1.0f, 1.0f };
			metallicFactor = 0.0f;
			roughnessFactor = 1.0f;
			emissiveFactor = { 0.0f, 0.0f, 0.0f };
			textures.clear();
		}
	};

	//=========================================================================
	// 메시 데이터
	//=========================================================================

	/**
	 * @brief 로드된 메시 데이터
	 *
	 * MeshAsset과 동일한 구조로, 직접 이동 가능합니다.
	 */
	struct LoadedMeshData
	{
		std::string name;

		std::vector<Graphics::StandardVertex> vertices;
		std::vector<Core::uint32> indices;
		std::vector<Graphics::SubmeshInfo> submeshes;

		Math::Vector3 aabbMin = { FLT_MAX, FLT_MAX, FLT_MAX };
		Math::Vector3 aabbMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		Core::int32 materialIndex = -1;

		void Clear()
		{
			name.clear();
			vertices.clear();
			indices.clear();
			submeshes.clear();
			aabbMin = { FLT_MAX, FLT_MAX, FLT_MAX };
			aabbMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
			materialIndex = -1;
		}
	};

	//=========================================================================
	// 노드 정보 (계층 구조)
	//=========================================================================

	/**
	 * @brief 모델 노드 정보
	 */
	struct LoadedNodeInfo
	{
		std::string name;
		Math::Matrix4x4 localTransform;
		Core::int32 parentIndex = -1;
		std::vector<Core::uint32> meshIndices;

		void Clear()
		{
			name.clear();
			localTransform = Math::Matrix4x4::Identity();
			parentIndex = -1;
			meshIndices.clear();
		}
	};

	//=========================================================================
	// Transform 분해 결과
	//=========================================================================

	/**
	 * @brief Matrix4x4를 Position/Rotation/Scale로 분해한 결과
	 *
	 * glTF 노드의 localTransform(Matrix4x4)을 ECS TransformComponent의
	 * position/rotation/scale로 변환할 때 사용합니다.
	 *
	 * @note Assimp의 aiMatrix4x4::Decompose()를 내부적으로 활용
	 * @note Phase 4.5 노드 계층 → ECS 변환에서 사용
	 */
	struct DecomposedTransform
	{
		Math::Vector3 position = Math::Vector3::Zero();
		Math::Quaternion rotation = Math::Quaternion::Identity();
		Math::Vector3 scale = Math::Vector3::One();

		/// 비균등 스케일 여부 (경고 출력용)
		/// scale.x, y, z 간 차이가 10% 이상일 때 true
		bool hasNonUniformScale = false;

		void Clear()
		{
			position = Math::Vector3::Zero();
			rotation = Math::Quaternion::Identity();
			scale = Math::Vector3::One();
			hasNonUniformScale = false;
		}
	};


	//=========================================================================
	// 전체 모델 데이터
	//=========================================================================

	/**
	 * @brief 로드된 모델 전체 데이터
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

		Core::uint32 GetEmbeddedTextureCount() const
		{
			Core::uint32 count = 0;
			for (const auto& mat : materials)
			{
				for (const auto& tex : mat.textures)
				{
					if (tex.isEmbedded)
					{
						++count;
					}
				}
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
	 * 정적 함수만 제공하는 유틸리티 클래스입니다.
	 */
	class ModelLoader
	{
	public:
		ModelLoader() = delete;

		/**
		 * @brief 단일 메시 로드 (첫 번째 메시만)
		 *
		 * @param filePath 모델 파일 경로
		 * @param outMeshData 출력: 메시 데이터
		 * @return 성공 여부
		 */
		static bool LoadMesh(const std::string& filePath, LoadedMeshData& outMeshData);

		/**
		 * @brief 전체 모델 로드 (메시, 머티리얼, 노드 포함)
		 *
		 * @param filePath 모델 파일 경로
		 * @param outModelData 출력: 모델 데이터
		 * @return 성공 여부
		 *
		 * @note glb 파일의 임베디드 텍스처도 자동으로 추출됩니다
		 */
		static bool LoadModel(const std::string& filePath, LoadedModelData& outModelData);

		/**
		 * @brief 4x4 변환 행렬을 Position/Rotation/Scale로 분해
		 *
		 * glTF 노드의 localTransform을 ECS TransformComponent 초기화에
		 * 사용할 수 있는 형태로 변환합니다.
		 *
		 * @param matrix 분해할 변환 행렬
		 * @return 분해된 Transform 데이터 (position, rotation, scale)
		 *
		 * @note 비균등 스케일(max/min 비율 > 1.1) 시 hasNonUniformScale = true
		 * @note 비균등 스케일 + 회전 조합 시 정확한 분해가 불가능할 수 있음
		 *
		 * @example
		 * DecomposedTransform trs = ModelLoader::DecomposeMatrix(node.localTransform);
		 * transform.position = trs.position;
		 * transform.rotation = trs.rotation;
		 * transform.scale = trs.scale;
		 */
		static DecomposedTransform DecomposeMatrix(const Math::Matrix4x4& matrix);
	};

} // namespace Framework
