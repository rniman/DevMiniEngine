/**
 * @file HierarchyBuilder.h
 * @brief glTF 노드 계층 → ECS Entity 계층 변환기
 *
 * LoadedModelData의 노드 트리 구조를 ECS Entity 계층으로 변환합니다.
 * 각 노드에 대해 Entity를 생성하고, 부모-자식 관계를 설정하며,
 * 메시가 있는 노드에는 MeshComponent와 MaterialComponent를 추가합니다.
 *
 * @note Phase 4.5: ModelViewerApp에서 분리
 */
#pragma once
#include "Framework/Resources/ResourceId.h"
#include "Math/MathTypes.h"
#include "Core/Types.h"
#include "ECS/Entity.h"
#include <vector>
#include <string>

namespace ECS
{
	class Registry;
	class SystemManager;
}

namespace Framework
{
	struct LoadedModelData;
	class ResourceManager;

	//=========================================================================
	// 계층 빌드 결과
	//=========================================================================

	/**
	 * @brief 모델 계층 생성 결과
	 *
	 * HierarchyBuilder::Build() 함수의 반환 타입입니다.
	 * 루트 Entity, 전체 Entity 목록, 렌더링 대상 Entity 목록을 포함합니다.
	 */
	struct HierarchyBuildResult
	{
		/// 계층 구조의 루트 Entity
		ECS::Entity rootEntity;

		/// 생성된 모든 Entity (루트 포함)
		std::vector<ECS::Entity> allEntities;

		/// MeshComponent를 가진 Entity만 (렌더링 대상)
		std::vector<ECS::Entity> renderableEntities;

		/// 유효성 검사
		bool IsValid() const { return rootEntity.IsValid(); }

		/// 초기화
		void Clear()
		{
			rootEntity = ECS::Entity::Invalid();
			allEntities.clear();
			renderableEntities.clear();
		}

		/// 생성된 Entity 수
		Core::size_t GetTotalCount() const { return allEntities.size(); }

		/// 렌더링 대상 Entity 수
		Core::size_t GetRenderableCount() const { return renderableEntities.size(); }
	};

	//=========================================================================
	// 계층 빌드 설정
	//=========================================================================

	/**
	 * @brief 계층 빌드 옵션
	 */
	struct HierarchyBuildOptions
	{
		/// 루트 Entity의 월드 위치 오프셋
		Math::Vector3 rootPosition = Math::Vector3::Zero();

		/// 루트 Entity의 스케일 (기본: 1,1,1)
		Math::Vector3 rootScale = { 1.0f, 1.0f, 1.0f };

		/// 업로드 후 소스 데이터 해제 여부
		bool releaseSourceDataAfterUpload = true;
	};

	//=========================================================================
	// HierarchyBuilder 클래스
	//=========================================================================

	/**
	 * @brief glTF 노드 계층 → ECS Entity 계층 변환기
	 *
	 * 정적 유틸리티 클래스로, 모델 로딩 후 ECS Entity 생성을 담당합니다.
	 *
	 * 사용 예시:
	 * @code
	 * LoadedModelData modelData;
	 * ModelLoader::LoadModel("model.glb", modelData);
	 *
	 * std::vector<ResourceId> materialIds = resourceManager.CreateMaterialsFromModelData(...);
	 *
	 * HierarchyBuildResult result = HierarchyBuilder::Build(
	 *     modelData,
	 *     materialIds,
	 *     registry,
	 *     systemManager,
	 *     resourceManager
	 * );
	 * @endcode
	 */
	class HierarchyBuilder
	{
	public:
		// 인스턴스화 금지
		HierarchyBuilder() = delete;

		/**
		 * @brief 모델 계층을 ECS Entity로 변환
		 *
		 * @param modelData 로드된 모델 데이터
		 * @param materialIds 미리 생성된 Material ID 목록
		 * @param registry ECS Registry
		 * @param systemManager ECS SystemManager (TransformSystem 접근용)
		 * @param resourceManager 리소스 매니저 (Mesh 생성용)
		 * @param options 빌드 옵션 (위치, 스케일 등)
		 * @return 생성된 Entity 정보
		 */
		static HierarchyBuildResult Build(
			const LoadedModelData& modelData,
			const std::vector<ResourceId>& materialIds,
			ECS::Registry& registry,
			ECS::SystemManager& systemManager,
			ResourceManager& resourceManager,
			const HierarchyBuildOptions& options = {}
		);

	private:
		/**
		 * @brief 노드의 메시들을 병합하여 GPU 리소스 생성
		 *
		 * 노드가 참조하는 meshIndices의 메시들을 하나로 병합하고
		 * 서브메시 정보를 생성합니다.
		 *
		 * @param meshIndices 병합할 메시 인덱스 목록
		 * @param modelData 원본 모델 데이터
		 * @param meshName 생성할 리소스 이름
		 * @param resourceManager 리소스 매니저
		 * @param releaseAfterUpload 업로드 후 소스 데이터 해제 여부
		 * @return 생성된 MeshResource ID (실패 시 Invalid)
		 */
		static ResourceId CreateMeshFromNodeIndices(
			const std::vector<Core::uint32>& meshIndices,
			const LoadedModelData& modelData,
			const std::string& meshName,
			ResourceManager& resourceManager,
			bool releaseAfterUpload
		);
	};

} // namespace Framework
