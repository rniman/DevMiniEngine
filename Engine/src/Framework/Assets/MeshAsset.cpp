/**
 * @file MeshAsset.cpp
 * @brief MeshAsset 클래스 구현
 */
#include "pch.h"
#include "Framework/Assets/MeshAsset.h"
#include "Framework/Assets/ModelLoader.h"  
#include "Core/Logging/LogMacros.h"
#include <cmath>

namespace Framework
{
	//=========================================================================
	// 정적 더미 서브메시 (범위 초과 시 반환)
	//=========================================================================

	static const Graphics::SubmeshInfo sEmptySubmesh = {};

	//=========================================================================
	// 생성자
	//=========================================================================

	MeshAsset::MeshAsset()
		: mPath()
		, mState(AssetState::Unloaded)
	{
	}

	//=========================================================================
	// IAsset 인터페이스
	//=========================================================================

	Core::size_t MeshAsset::GetMemoryUsage() const
	{
		Core::size_t usage = sizeof(MeshAsset);

		// 문자열 메모리
		usage += mPath.capacity();

		// CPU 데이터 (해제되지 않은 경우만)
		if (!mSourceDataReleased)
		{
			usage += mVertices.capacity() * sizeof(Graphics::StandardVertex);
			usage += mIndices.capacity() * sizeof(Core::uint32);
			usage += mSubmeshes.capacity() * sizeof(Graphics::SubmeshInfo);
		}

		return usage;
	}

	std::unique_ptr<MeshAsset> MeshAsset::MergeFromMeshData(const std::vector<const LoadedMeshData*>& meshes)
	{
		if (meshes.empty())
		{
			LOG_WARN("[MeshAsset] MergeFromMeshData: empty mesh list");
			return nullptr;
		}

		// 유효한 메시만 필터링
		std::vector<const LoadedMeshData*> validMeshes;
		validMeshes.reserve(meshes.size());

		for (const auto* mesh : meshes)
		{
			if (mesh && !mesh->vertices.empty())
			{
				validMeshes.push_back(mesh);
			}
		}

		if (validMeshes.empty())
		{
			LOG_WARN("[MeshAsset] MergeFromMeshData: no valid meshes");
			return nullptr;
		}

		// 결과 데이터
		std::vector<Graphics::StandardVertex> allVertices;
		std::vector<Core::uint32> allIndices;
		std::vector<Graphics::SubmeshInfo> submeshes;

		// AABB 초기화
		Math::Vector3 aabbMin = { FLT_MAX, FLT_MAX, FLT_MAX };
		Math::Vector3 aabbMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

		// 총 크기 예측 (reserve용)
		Core::size_t totalVertices = 0;
		Core::size_t totalIndices = 0;
		for (const auto* mesh : validMeshes)
		{
			totalVertices += mesh->vertices.size();
			totalIndices += mesh->indices.size();
		}
		allVertices.reserve(totalVertices);
		allIndices.reserve(totalIndices);
		submeshes.reserve(validMeshes.size());

		// 병합
		Core::uint32 baseVertex = 0;
		Core::uint32 baseIndex = 0;

		for (Core::size_t i = 0; i < validMeshes.size(); ++i)
		{
			const auto* meshData = validMeshes[i];

			// 서브메시 정보 생성
			Graphics::SubmeshInfo submesh;
			submesh.startIndex = baseIndex;
			submesh.indexCount = static_cast<Core::uint32>(meshData->indices.size());
			submesh.baseVertex = baseVertex;
			submesh.materialIndex = static_cast<Core::uint32>(i);
			submeshes.push_back(submesh);

			// 버텍스 추가
			allVertices.insert(
				allVertices.end(),
				meshData->vertices.begin(),
				meshData->vertices.end()
			);

			// 인덱스 추가
			allIndices.insert(
				allIndices.end(),
				meshData->indices.begin(),
				meshData->indices.end()
			);

			// AABB 병합
			aabbMin.x = std::min(aabbMin.x, meshData->aabbMin.x);
			aabbMin.y = std::min(aabbMin.y, meshData->aabbMin.y);
			aabbMin.z = std::min(aabbMin.z, meshData->aabbMin.z);
			aabbMax.x = std::max(aabbMax.x, meshData->aabbMax.x);
			aabbMax.y = std::max(aabbMax.y, meshData->aabbMax.y);
			aabbMax.z = std::max(aabbMax.z, meshData->aabbMax.z);

			// 오프셋 갱신
			baseVertex += static_cast<Core::uint32>(meshData->vertices.size());
			baseIndex += static_cast<Core::uint32>(meshData->indices.size());
		}

		// MeshAsset 생성
		auto result = std::make_unique<MeshAsset>();
		result->SetVertices(std::move(allVertices));
		result->SetIndices(std::move(allIndices));
		result->SetSubmeshes(std::move(submeshes));
		result->SetAABB(aabbMin, aabbMax);

		LOG_DEBUG(
			"[MeshAsset] Merged %zu meshes: %u vertices, %u indices, %u submeshes",
			validMeshes.size(),
			result->GetVertexCount(),
			result->GetIndexCount(),
			result->GetSubmeshCount()
		);

		return result;
	}

	//=========================================================================
	// 서브메시 접근
	//=========================================================================

	const Graphics::SubmeshInfo& MeshAsset::GetSubmesh(Core::uint32 index) const
	{
		if (index < mSubmeshes.size())
		{
			return mSubmeshes[index];
		}

		LOG_WARN(
			"[MeshAsset] Submesh index out of range: %u (count: %zu)",
			index, mSubmeshes.size()
		);
		return sEmptySubmesh;
	}

	//=========================================================================
	// 바운딩 정보
	//=========================================================================

	Math::Vector3 MeshAsset::GetAABBCenter() const
	{
		return Math::Vector3(
			(mAABBMin.x + mAABBMax.x) * 0.5f,
			(mAABBMin.y + mAABBMax.y) * 0.5f,
			(mAABBMin.z + mAABBMax.z) * 0.5f
		);
	}

	Math::Vector3 MeshAsset::GetAABBExtent() const
	{
		return Math::Vector3(
			(mAABBMax.x - mAABBMin.x) * 0.5f,
			(mAABBMax.y - mAABBMin.y) * 0.5f,
			(mAABBMax.z - mAABBMin.z) * 0.5f
		);
	}

	//=========================================================================
	// 인덱스 변환
	//=========================================================================

	std::vector<Core::uint16> MeshAsset::GetIndices16() const
	{
		if (!CanUse16BitIndices())
		{
			LOG_WARN("[MeshAsset] Cannot convert to 16-bit indices: vertex count %u exceeds 65535",
				mVertexCount);
			return {};
		}

		std::vector<Core::uint16> indices16;
		indices16.reserve(mIndices.size());

		for (Core::uint32 idx : mIndices)
		{
			indices16.push_back(static_cast<Core::uint16>(idx));
		}

		return indices16;
	}

	//=========================================================================
	// 메모리 관리
	//=========================================================================

	void MeshAsset::ReleaseSourceData()
	{
		if (mSourceDataReleased)
		{
			return;
		}

		if (mDataPolicy == MeshDataPolicy::KeepSourceData)
		{
			LOG_DEBUG("[MeshAsset] ReleaseSourceData ignored (policy: KeepSourceData)");
			return;
		}

		// 데이터 해제 (capacity도 해제)
		std::vector<Graphics::StandardVertex>().swap(mVertices);
		std::vector<Core::uint32>().swap(mIndices);
		// submeshes는 유지 (렌더링 정보)

		mSourceDataReleased = true;

		LOG_DEBUG("[MeshAsset] Source data released: %s", mPath.c_str());
	}

	//=========================================================================
	// 데이터 설정
	//=========================================================================

	void MeshAsset::SetVertices(std::vector<Graphics::StandardVertex>&& vertices)
	{
		mVertexCount = static_cast<Core::uint32>(vertices.size());
		mVertices = std::move(vertices);
		mSourceDataReleased = false;
	}

	void MeshAsset::SetIndices(std::vector<Core::uint32>&& indices)
	{
		mIndexCount = static_cast<Core::uint32>(indices.size());
		mIndices = std::move(indices);
		mSourceDataReleased = false;
	}

	void MeshAsset::SetSubmeshes(std::vector<Graphics::SubmeshInfo>&& submeshes)
	{
		mSubmeshes = std::move(submeshes);
	}

	void MeshAsset::SetAABB(const Math::Vector3& min, const Math::Vector3& max)
	{
		mAABBMin = min;
		mAABBMax = max;

		// 바운딩 구 자동 계산
		CalculateBoundingSphere();
	}

	void MeshAsset::CalculateBoundingSphere()
	{
		// AABB 기반 바운딩 구 계산
		mBoundingSphereCenter = GetAABBCenter();

		Math::Vector3 extent = GetAABBExtent();
		mBoundingSphereRadius = std::sqrt(
			extent.x * extent.x +
			extent.y * extent.y +
			extent.z * extent.z
		);
	}

} // namespace Framework
