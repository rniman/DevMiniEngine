/**
 * @file ModelAsset.cpp
 * @brief ModelAsset 클래스 구현
 */
#include "pch.h"
#include "Framework/Assets/ModelAsset.h"

namespace Framework
{
	ModelAsset::ModelAsset()
		: mPath()
		, mState(AssetState::Unloaded)
	{
	}

	Core::size_t ModelAsset::GetMemoryUsage() const
	{
		Core::size_t usage = sizeof(ModelAsset);

		// 문자열 메모리
		usage += mPath.capacity();

		// 인덱스 벡터 메모리
		usage += mMeshIndices.capacity() * sizeof(Core::uint32);
		usage += mMaterialIndices.capacity() * sizeof(Core::uint32);
		usage += mTextureIndices.capacity() * sizeof(Core::uint32);
		usage += mRootNodeIndices.capacity() * sizeof(Core::uint32);

		// 노드 메모리
		usage += mNodes.capacity() * sizeof(ModelNode);
		for (const auto& node : mNodes)
		{
			usage += node.name.capacity();
			usage += node.childIndices.capacity() * sizeof(Core::uint32);
		}

		return usage;
	}

} // namespace Framework
