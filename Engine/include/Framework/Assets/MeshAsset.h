/**
 * @file MeshAsset.h
 * @brief 메시 데이터를 담는 Asset 클래스
 *
 * 파일에서 로드된 정점, 인덱스 데이터를 보유합니다.
 * ResourceManager를 통해 GPU Mesh로 변환됩니다.
 */
#pragma once
#include "Framework/Assets/IAsset.h"
#include "Core/Types.h"
#include <string>
#include <vector>

namespace Framework
{
	/**
	 * @brief 메시 Asset 클래스
	 *
	 * glTF 등에서 로드된 메시의 CPU 측 데이터를 보유합니다.
	 * 정점 위치, 노말, UV, 탄젠트 및 인덱스 데이터를 포함합니다.
	 *
	 * @note Phase 4.1: 구조만 정의, 실제 데이터 로딩은 Phase 4.2
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
		// MeshAsset 전용 API (Phase 4.2에서 구현)
		//=========================================================================

		// TODO: Phase 4.2 - 정점 데이터 접근
		// const std::vector<Vertex>& GetVertices() const;
		// const std::vector<Core::uint32>& GetIndices() const;
		// Core::uint32 GetVertexCount() const;
		// Core::uint32 GetIndexCount() const;

		// TODO: Phase 4.2 - 서브메시 지원
		// Core::uint32 GetSubmeshCount() const;
		// const SubmeshInfo& GetSubmesh(Core::uint32 index) const;

	private:
		friend class AssetManager;

		std::string mPath;
		AssetState mState = AssetState::Unloaded;

		// TODO: Phase 4.2 - 메시 데이터
		// std::vector<Vertex> mVertices;
		// std::vector<Core::uint32> mIndices;
		// std::vector<SubmeshInfo> mSubmeshes;
	};

} // namespace Framework
