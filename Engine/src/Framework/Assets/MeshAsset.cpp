/**
 * @file MeshAsset.cpp
 * @brief MeshAsset 클래스 구현
 */
#include "pch.h"
#include "Framework/Assets/MeshAsset.h"

namespace Framework
{
	MeshAsset::MeshAsset()
		: mPath()
		, mState(AssetState::Unloaded)
	{
	}

	Core::size_t MeshAsset::GetMemoryUsage() const
	{
		Core::size_t usage = sizeof(MeshAsset);

		// 문자열 메모리
		usage += mPath.capacity();

		// TODO: Phase 4.2 - 정점/인덱스 데이터 메모리
		// usage += mVertices.capacity() * sizeof(Vertex);
		// usage += mIndices.capacity() * sizeof(Core::uint32);

		return usage;
	}

} // namespace Framework
