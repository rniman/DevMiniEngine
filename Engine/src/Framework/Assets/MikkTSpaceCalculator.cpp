/**
 * @file MikkTSpaceCalculator.cpp
 * @brief MikkTSpaceCalculator 구현
 */
#include "pch.h"
#include "Framework/Assets/MikkTSpaceCalculator.h"
#include "mikktspace.h"

namespace Framework
{
	//=========================================================================
	// MikkTSpace 콜백용 컨텍스트 데이터
	//=========================================================================

	struct MikkTSpaceUserData
	{
		const Math::Vector3* positions;
		const Math::Vector3* normals;
		const Math::Vector2* texCoords;
		const void* indices;          // uint16 또는 uint32
		bool use32BitIndices;
		Core::size_t vertexCount;
		Core::size_t triangleCount;
		Math::Vector4* outTangents;
	};

	//=========================================================================
	// 인덱스 접근 헬퍼
	//=========================================================================

	static inline Core::uint32 GetIndex(const MikkTSpaceUserData* data, Core::size_t triangleIndex, Core::size_t vertexIndex)
	{
		Core::size_t index = triangleIndex * 3 + vertexIndex;
		if (data->use32BitIndices)
		{
			return static_cast<const Core::uint32*>(data->indices)[index];
		}
		else
		{
			return static_cast<Core::uint32>(static_cast<const Core::uint16*>(data->indices)[index]);
		}
	}

	//=========================================================================
	// MikkTSpace 콜백 함수들
	//=========================================================================

	/**
	 * @brief 삼각형 개수 반환
	 */
	static int MikkGetNumFaces(const SMikkTSpaceContext* pContext)
	{
		const MikkTSpaceUserData* data = static_cast<const MikkTSpaceUserData*>(pContext->m_pUserData);
		return static_cast<int>(data->triangleCount);
	}

	/**
	 * @brief 페이스당 버텍스 개수 반환 (삼각형이므로 항상 3)
	 */
	static int MikkGetNumVerticesOfFace(const SMikkTSpaceContext* pContext, int iFace)
	{
		(void)pContext;
		(void)iFace;
		return 3;
	}

	/**
	 * @brief 버텍스 위치 반환
	 */
	static void MikkGetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], int iFace, int iVert)
	{
		const MikkTSpaceUserData* data = static_cast<const MikkTSpaceUserData*>(pContext->m_pUserData);
		Core::uint32 index = GetIndex(data, iFace, iVert);
		const Math::Vector3& pos = data->positions[index];
		fvPosOut[0] = pos.x;
		fvPosOut[1] = pos.y;
		fvPosOut[2] = pos.z;
	}

	/**
	 * @brief 버텍스 노말 반환
	 */
	static void MikkGetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], int iFace, int iVert)
	{
		const MikkTSpaceUserData* data = static_cast<const MikkTSpaceUserData*>(pContext->m_pUserData);
		Core::uint32 index = GetIndex(data, iFace, iVert);
		const Math::Vector3& norm = data->normals[index];
		fvNormOut[0] = norm.x;
		fvNormOut[1] = norm.y;
		fvNormOut[2] = norm.z;
	}

	/**
	 * @brief 버텍스 텍스처 좌표 반환
	 */
	static void MikkGetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], int iFace, int iVert)
	{
		const MikkTSpaceUserData* data = static_cast<const MikkTSpaceUserData*>(pContext->m_pUserData);
		Core::uint32 index = GetIndex(data, iFace, iVert);
		const Math::Vector2& uv = data->texCoords[index];
		fvTexcOut[0] = uv.x;
		fvTexcOut[1] = uv.y;
	}

	/**
	 * @brief 계산된 Tangent 저장 (기본 콜백)
	 *
	 * MikkTSpace가 계산한 Tangent를 저장합니다.
	 * fSign은 Bitangent 부호입니다.
	 */
	static void MikkSetTSpaceBasic(
		const SMikkTSpaceContext* pContext,
		const float fvTangent[],
		float fSign,
		int iFace,
		int iVert
	)
	{
		MikkTSpaceUserData* data = static_cast<MikkTSpaceUserData*>(pContext->m_pUserData);
		Core::uint32 index = GetIndex(data, iFace, iVert);

		data->outTangents[index].x = fvTangent[0];
		data->outTangents[index].y = fvTangent[1];
		data->outTangents[index].z = fvTangent[2];
		data->outTangents[index].w = fSign;
	}

	//=========================================================================
	// MikkTSpaceCalculator 구현
	//=========================================================================

	bool MikkTSpaceCalculator::Calculate(
		const std::vector<Math::Vector3>& positions,
		const std::vector<Math::Vector3>& normals,
		const std::vector<Math::Vector2>& texCoords,
		const std::vector<Core::uint16>& indices,
		std::vector<Math::Vector4>& outTangents
	)
	{
		// 입력 검증
		if (positions.empty() || normals.empty() || texCoords.empty() || indices.empty())
		{
			return false;
		}

		if (positions.size() != normals.size() || positions.size() != texCoords.size())
		{
			return false;
		}

		if (indices.size() % 3 != 0)
		{
			return false;
		}

		// 출력 배열 초기화
		outTangents.clear();
		outTangents.resize(positions.size(), Math::Vector4(0.0f, 0.0f, 1.0f, 1.0f));

		// MikkTSpace 컨텍스트 설정
		MikkTSpaceUserData userData = {};
		userData.positions = positions.data();
		userData.normals = normals.data();
		userData.texCoords = texCoords.data();
		userData.indices = indices.data();
		userData.use32BitIndices = false;
		userData.vertexCount = positions.size();
		userData.triangleCount = indices.size() / 3;
		userData.outTangents = outTangents.data();

		// MikkTSpace 인터페이스 설정
		SMikkTSpaceInterface mikkInterface = {};
		mikkInterface.m_getNumFaces = MikkGetNumFaces;
		mikkInterface.m_getNumVerticesOfFace = MikkGetNumVerticesOfFace;
		mikkInterface.m_getPosition = MikkGetPosition;
		mikkInterface.m_getNormal = MikkGetNormal;
		mikkInterface.m_getTexCoord = MikkGetTexCoord;
		mikkInterface.m_setTSpaceBasic = MikkSetTSpaceBasic;
		mikkInterface.m_setTSpace = nullptr;  // 기본 콜백 사용

		// MikkTSpace 컨텍스트
		SMikkTSpaceContext mikkContext = {};
		mikkContext.m_pInterface = &mikkInterface;
		mikkContext.m_pUserData = &userData;

		// Tangent 계산 실행
		tbool result = genTangSpaceDefault(&mikkContext);

		return result != 0;
	}

	bool MikkTSpaceCalculator::Calculate(
		const std::vector<Math::Vector3>& positions,
		const std::vector<Math::Vector3>& normals,
		const std::vector<Math::Vector2>& texCoords,
		const std::vector<Core::uint32>& indices,
		std::vector<Math::Vector4>& outTangents
	)
	{
		// 입력 검증
		if (positions.empty() || normals.empty() || texCoords.empty() || indices.empty())
		{
			return false;
		}

		if (positions.size() != normals.size() || positions.size() != texCoords.size())
		{
			return false;
		}

		if (indices.size() % 3 != 0)
		{
			return false;
		}

		// 출력 배열 초기화
		outTangents.clear();
		outTangents.resize(positions.size(), Math::Vector4(0.0f, 0.0f, 1.0f, 1.0f));

		// MikkTSpace 컨텍스트 설정
		MikkTSpaceUserData userData = {};
		userData.positions = positions.data();
		userData.normals = normals.data();
		userData.texCoords = texCoords.data();
		userData.indices = indices.data();
		userData.use32BitIndices = true;
		userData.vertexCount = positions.size();
		userData.triangleCount = indices.size() / 3;
		userData.outTangents = outTangents.data();

		// MikkTSpace 인터페이스 설정
		SMikkTSpaceInterface mikkInterface = {};
		mikkInterface.m_getNumFaces = MikkGetNumFaces;
		mikkInterface.m_getNumVerticesOfFace = MikkGetNumVerticesOfFace;
		mikkInterface.m_getPosition = MikkGetPosition;
		mikkInterface.m_getNormal = MikkGetNormal;
		mikkInterface.m_getTexCoord = MikkGetTexCoord;
		mikkInterface.m_setTSpaceBasic = MikkSetTSpaceBasic;
		mikkInterface.m_setTSpace = nullptr;

		// MikkTSpace 컨텍스트
		SMikkTSpaceContext mikkContext = {};
		mikkContext.m_pInterface = &mikkInterface;
		mikkContext.m_pUserData = &userData;

		// Tangent 계산 실행
		tbool result = genTangSpaceDefault(&mikkContext);

		return result != 0;
	}

} // namespace Framework
