/**
 * @file ModelLoader.cpp
 * @brief ModelLoader 구현
 */
#include "pch.h"
#include "Framework/Assets/ModelLoader.h"
#include "Framework/Assets/MikkTSpaceCalculator.h"
#include "Core/Logging/LogMacros.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

#include <filesystem>

namespace Framework
{
	//=========================================================================
	// Assimp 플래그 설정
	//=========================================================================

	static constexpr unsigned int ASSIMP_LOAD_FLAGS =
		aiProcess_Triangulate |
		aiProcess_GenNormals |
		aiProcess_JoinIdenticalVertices |
		aiProcess_ImproveCacheLocality |
		aiProcess_GenBoundingBoxes |
		aiProcess_ConvertToLeftHanded;  // MakeLeftHanded + FlipUVs + FlipWindingOrder
	// aiProcess_CalcTangentSpace 제외 - MikkTSpace 사용

//=========================================================================
// 내부 유틸리티 함수 (Assimp 타입 사용)
//=========================================================================

	namespace
	{
		/**
		 * @brief 모델 파일의 디렉토리 경로 추출
		 */
		std::string GetDirectoryPath(const std::string& filePath)
		{
			std::filesystem::path p(filePath);
			return p.parent_path().string();
		}

		/**
		 * @brief aiMatrix4x4 → Math::Matrix4x4 변환
		 */
		Math::Matrix4x4 ConvertMatrix(const aiMatrix4x4& aiMat)
		{
			Math::Matrix4x4 result;
			result.m[0][0] = aiMat.a1; result.m[0][1] = aiMat.a2; result.m[0][2] = aiMat.a3; result.m[0][3] = aiMat.a4;
			result.m[1][0] = aiMat.b1; result.m[1][1] = aiMat.b2; result.m[1][2] = aiMat.b3; result.m[1][3] = aiMat.b4;
			result.m[2][0] = aiMat.c1; result.m[2][1] = aiMat.c2; result.m[2][2] = aiMat.c3; result.m[2][3] = aiMat.c4;
			result.m[3][0] = aiMat.d1; result.m[3][1] = aiMat.d2; result.m[3][2] = aiMat.d3; result.m[3][3] = aiMat.d4;
			return result;
		}

		/**
		 * @brief AABB 업데이트
		 */
		void UpdateAABB(
			const Math::Vector3& position,
			Math::Vector3& aabbMin,
			Math::Vector3& aabbMax
		)
		{
			aabbMin.x = std::min(aabbMin.x, position.x);
			aabbMin.y = std::min(aabbMin.y, position.y);
			aabbMin.z = std::min(aabbMin.z, position.z);
			aabbMax.x = std::max(aabbMax.x, position.x);
			aabbMax.y = std::max(aabbMax.y, position.y);
			aabbMax.z = std::max(aabbMax.z, position.z);
		}

		/**
		 * @brief Assimp 텍스처 타입 → 엔진 텍스처 타입 변환
		 */
		Graphics::TextureType ConvertTextureType(aiTextureType aiType)
		{
			switch (aiType)
			{
			case aiTextureType_DIFFUSE:
			case aiTextureType_BASE_COLOR:
				return Graphics::TextureType::Albedo;

			case aiTextureType_NORMALS:
			case aiTextureType_NORMAL_CAMERA:
				return Graphics::TextureType::Normal;

			case aiTextureType_METALNESS:
				return Graphics::TextureType::Metallic;

			case aiTextureType_DIFFUSE_ROUGHNESS:
				return Graphics::TextureType::Roughness;

			case aiTextureType_AMBIENT_OCCLUSION:
			case aiTextureType_LIGHTMAP:
				return Graphics::TextureType::AmbientOcclusion;

			case aiTextureType_EMISSIVE:
			case aiTextureType_EMISSION_COLOR:
				return Graphics::TextureType::Emissive;

			default:
				return Graphics::TextureType::Albedo;
			}
		}

		/**
		 * @brief 머티리얼에서 특정 타입의 텍스처 경로 추출
		 */
		bool ExtractTexturePath(
			const aiMaterial* material,
			aiTextureType aiType,
			const std::string& modelDirectory,
			LoadedTextureInfo& outTexInfo
		)
		{
			if (material->GetTextureCount(aiType) == 0)
			{
				return false;
			}

			aiString texPath;
			if (material->GetTexture(aiType, 0, &texPath) != AI_SUCCESS)
			{
				return false;
			}

			std::string pathStr = texPath.C_Str();

			// 빈 경로 체크
			if (pathStr.empty())
			{
				return false;
			}

			// 내장 텍스처 체크 (glTF에서 *0, *1 형식)
			if (pathStr[0] == '*')
			{
				// 내장 텍스처는 현재 미지원, 경로만 기록
				outTexInfo.path = pathStr;
				outTexInfo.type = ConvertTextureType(aiType);
				LOG_DEBUG("[ModelLoader] Embedded texture found: %s (type: %d)", pathStr.c_str(), static_cast<int>(aiType));
				return true;
			}

			// 상대 경로 조합
			std::filesystem::path fullPath = std::filesystem::path(modelDirectory) / pathStr;
			outTexInfo.path = fullPath.string();
			outTexInfo.type = ConvertTextureType(aiType);

			return true;
		}

		/**
		 * @brief Assimp aiMaterial에서 머티리얼 데이터 추출
		 */
		void ProcessMaterial(
			const aiMaterial* material,
			const std::string& modelDirectory,
			LoadedMaterialData& outMaterialData
		)
		{
			// 머티리얼 이름
			aiString name;
			if (material->Get(AI_MATKEY_NAME, name) == AI_SUCCESS)
			{
				outMaterialData.name = name.C_Str();
			}
			else
			{
				outMaterialData.name = "Unnamed";
			}

			// Base Color Factor (glTF PBR)
			aiColor4D baseColor;
			if (material->Get(AI_MATKEY_BASE_COLOR, baseColor) == AI_SUCCESS)
			{
				outMaterialData.baseColorFactor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };
			}
			else if (material->Get(AI_MATKEY_COLOR_DIFFUSE, baseColor) == AI_SUCCESS)
			{
				// Fallback to diffuse color
				outMaterialData.baseColorFactor = { baseColor.r, baseColor.g, baseColor.b, baseColor.a };
			}

			// Metallic Factor
			ai_real metallic = 0.0f;
			if (material->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS)
			{
				outMaterialData.metallicFactor = static_cast<Core::float32>(metallic);
			}

			// Roughness Factor
			ai_real roughness = 1.0f;
			if (material->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS)
			{
				outMaterialData.roughnessFactor = static_cast<Core::float32>(roughness);
			}

			// Emissive Factor
			aiColor3D emissive;
			if (material->Get(AI_MATKEY_COLOR_EMISSIVE, emissive) == AI_SUCCESS)
			{
				outMaterialData.emissiveFactor = { emissive.r, emissive.g, emissive.b };
			}

			// 텍스처 추출 (지원하는 모든 타입)
			static const aiTextureType textureTypes[] = {
				aiTextureType_DIFFUSE,
				aiTextureType_BASE_COLOR,
				aiTextureType_NORMALS,
				aiTextureType_METALNESS,
				aiTextureType_DIFFUSE_ROUGHNESS,
				aiTextureType_AMBIENT_OCCLUSION,
				aiTextureType_EMISSIVE
			};

			for (aiTextureType aiType : textureTypes)
			{
				LoadedTextureInfo texInfo;
				if (ExtractTexturePath(material, aiType, modelDirectory, texInfo))
				{
					// 중복 타입 체크 (같은 TextureType이 이미 있으면 스킵)
					bool duplicate = false;
					for (const auto& existing : outMaterialData.textures)
					{
						if (existing.type == texInfo.type)
						{
							duplicate = true;
							break;
						}
					}

					if (!duplicate)
					{
						outMaterialData.textures.push_back(texInfo);
					}
				}
			}

			LOG_DEBUG("[ModelLoader] Material '%s': BaseColor(%.2f,%.2f,%.2f), Metallic=%.2f, Roughness=%.2f, Textures=%zu",
				outMaterialData.name.c_str(),
				outMaterialData.baseColorFactor.x,
				outMaterialData.baseColorFactor.y,
				outMaterialData.baseColorFactor.z,
				outMaterialData.metallicFactor,
				outMaterialData.roughnessFactor,
				outMaterialData.textures.size());
		}

		/**
		 * @brief Assimp aiMesh를 LoadedMeshData로 변환
		 */
		bool ProcessMesh(const aiMesh* mesh, LoadedMeshData& outMeshData)
		{
			if (!mesh || mesh->mNumVertices == 0)
			{
				return false;
			}

			outMeshData.Clear();
			outMeshData.name = mesh->mName.C_Str();

			const Core::uint32 vertexCount = mesh->mNumVertices;

			// 임시 배열 (MikkTSpace용)
			std::vector<Math::Vector3> positions(vertexCount);
			std::vector<Math::Vector3> normals(vertexCount);
			std::vector<Math::Vector2> texCoords(vertexCount);

			// 정점 데이터 추출
			for (Core::uint32 i = 0; i < vertexCount; ++i)
			{
				// Position
				positions[i].x = mesh->mVertices[i].x;
				positions[i].y = mesh->mVertices[i].y;
				positions[i].z = mesh->mVertices[i].z;

				// AABB 업데이트
				UpdateAABB(positions[i], outMeshData.aabbMin, outMeshData.aabbMax);

				// Normal
				if (mesh->HasNormals())
				{
					normals[i].x = mesh->mNormals[i].x;
					normals[i].y = mesh->mNormals[i].y;
					normals[i].z = mesh->mNormals[i].z;
				}
				else
				{
					normals[i] = Math::Vector3(0.0f, 1.0f, 0.0f);
				}

				// TexCoord (첫 번째 UV 채널만)
				if (mesh->HasTextureCoords(0))
				{
					texCoords[i].x = mesh->mTextureCoords[0][i].x;
					texCoords[i].y = mesh->mTextureCoords[0][i].y;
				}
				else
				{
					texCoords[i] = Math::Vector2(0.0f, 0.0f);
				}
			}

			// 인덱스 추출
			std::vector<Core::uint32> indices;
			indices.reserve(mesh->mNumFaces * 3);

			for (Core::uint32 i = 0; i < mesh->mNumFaces; ++i)
			{
				const aiFace& face = mesh->mFaces[i];
				for (Core::uint32 j = 0; j < face.mNumIndices; ++j)
				{
					indices.push_back(face.mIndices[j]);
				}
			}

			// MikkTSpace로 Tangent 계산
			std::vector<Math::Vector4> tangents;
			if (!MikkTSpaceCalculator::Calculate(positions, normals, texCoords, indices, tangents))
			{
				LOG_WARN("[ModelLoader] MikkTSpace calculation failed, using default tangents");
				tangents.resize(vertexCount, Math::Vector4(1.0f, 0.0f, 0.0f, 1.0f));
			}

			// StandardVertex 배열 생성
			outMeshData.vertices.resize(vertexCount);
			for (Core::uint32 i = 0; i < vertexCount; ++i)
			{
				outMeshData.vertices[i].position = positions[i];
				outMeshData.vertices[i].normal = normals[i];
				outMeshData.vertices[i].texCoord = texCoords[i];
				outMeshData.vertices[i].tangent = tangents[i];
			}

			// 인덱스 복사
			outMeshData.indices = std::move(indices);

			// 단일 서브메시 (전체)
			SubmeshInfo submesh;
			submesh.startIndex = 0;
			submesh.indexCount = static_cast<Core::uint32>(outMeshData.indices.size());
			submesh.baseVertex = 0;
			submesh.materialIndex = mesh->mMaterialIndex;
			outMeshData.submeshes.push_back(submesh);

			return true;
		}

		/**
		 * @brief Assimp aiNode 계층 구조 처리 (재귀)
		 */
		void ProcessNode(
			const aiNode* node,
			const aiScene* scene,
			Core::int32 parentIndex,
			LoadedModelData& outModelData
		)
		{
			if (!node)
			{
				return;
			}

			// 현재 노드 정보 저장
			LoadedNodeInfo nodeInfo;
			nodeInfo.name = node->mName.C_Str();
			nodeInfo.localTransform = ConvertMatrix(node->mTransformation);
			nodeInfo.parentIndex = parentIndex;

			// 메시 인덱스 (첫 번째 메시만)
			if (node->mNumMeshes > 0)
			{
				nodeInfo.meshIndex = static_cast<Core::int32>(node->mMeshes[0]);
			}
			else
			{
				nodeInfo.meshIndex = -1;
			}

			Core::int32 currentIndex = static_cast<Core::int32>(outModelData.nodes.size());
			outModelData.nodes.push_back(nodeInfo);

			// 자식 노드 재귀 처리
			for (unsigned int i = 0; i < node->mNumChildren; ++i)
			{
				ProcessNode(node->mChildren[i], scene, currentIndex, outModelData);
			}
		}

	} // anonymous namespace

	//=========================================================================
	// ModelLoader 공개 API 구현
	//=========================================================================

	bool ModelLoader::LoadMesh(const std::string& filePath, LoadedMeshData& outMeshData)
	{
		outMeshData.Clear();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath, ASSIMP_LOAD_FLAGS);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_ERROR("[ModelLoader] Failed to load: %s - %s", filePath.c_str(), importer.GetErrorString());
			return false;
		}

		if (scene->mNumMeshes == 0)
		{
			LOG_ERROR("[ModelLoader] No meshes found in: %s", filePath.c_str());
			return false;
		}

		// 첫 번째 메시만 로드
		if (!ProcessMesh(scene->mMeshes[0], outMeshData))
		{
			LOG_ERROR("[ModelLoader] Failed to process mesh in: %s", filePath.c_str());
			return false;
		}

		LOG_INFO("[ModelLoader] Loaded mesh: %s (%zu vertices, %zu indices)",
			filePath.c_str(), outMeshData.vertices.size(), outMeshData.indices.size());

		return true;
	}

	bool ModelLoader::LoadModel(const std::string& filePath, LoadedModelData& outModelData)
	{
		outModelData.Clear();

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filePath, ASSIMP_LOAD_FLAGS);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_ERROR("[ModelLoader] Failed to load: %s - %s", filePath.c_str(), importer.GetErrorString());
			return false;
		}

		// 모델 디렉토리 (텍스처 경로 조합용)
		std::string modelDirectory = GetDirectoryPath(filePath);

		// 파일명 추출
		size_t lastSlash = filePath.find_last_of("/\\");
		size_t lastDot = filePath.find_last_of('.');
		if (lastSlash == std::string::npos) lastSlash = 0;
		else lastSlash++;
		if (lastDot == std::string::npos || lastDot < lastSlash) lastDot = filePath.length();
		outModelData.name = filePath.substr(lastSlash, lastDot - lastSlash);

		// 머티리얼 처리
		outModelData.materials.resize(scene->mNumMaterials);
		for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
		{
			ProcessMaterial(scene->mMaterials[i], modelDirectory, outModelData.materials[i]);
		}

		// 메시 처리
		outModelData.meshes.resize(scene->mNumMeshes);
		for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
		{
			if (!ProcessMesh(scene->mMeshes[i], outModelData.meshes[i]))
			{
				LOG_WARN("[ModelLoader] Failed to process mesh %u in: %s", i, filePath.c_str());
			}
		}

		// 노드 계층 구조 처리
		ProcessNode(scene->mRootNode, scene, -1, outModelData);

		LOG_INFO("[ModelLoader] Loaded model: %s (%zu meshes, %zu materials, %zu nodes, %u textures)",
			filePath.c_str(),
			outModelData.meshes.size(),
			outModelData.materials.size(),
			outModelData.nodes.size(),
			outModelData.GetTotalTextureCount());

		return true;
	}

} // namespace Framework
