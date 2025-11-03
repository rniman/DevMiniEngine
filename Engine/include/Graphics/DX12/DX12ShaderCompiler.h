#pragma once
#include "Graphics/GraphicsTypes.h"
#include <string>

namespace Graphics
{

	/**
	 * @brief HLSL 셰이더 컴파일 및 CSO 로딩을 담당하는 유틸리티 클래스
	 *
	 * 런타임 HLSL 컴파일과 사전 컴파일된 CSO 파일 로딩을 지원합니다.
	 * D3DCompile API를 래핑하여 일관된 인터페이스를 제공합니다.
	 *
	 * @note 스레드 안전하지 않음
	 * @warning 컴파일 실패 시 디버그 출력에 에러 메시지 표시
	 */
	class DX12ShaderCompiler
	{
	public:
		DX12ShaderCompiler() = default;
		~DX12ShaderCompiler() = default;

		DX12ShaderCompiler(const DX12ShaderCompiler&) = delete;
		DX12ShaderCompiler& operator=(const DX12ShaderCompiler&) = delete;

		/**
		 * @brief HLSL 파일을 런타임에 컴파일
		 *
		 * 디버그 빌드에서는 디버그 플래그가 활성화되며,
		 * 컴파일 실패 시 에러 메시지를 디버그 출력에 표시합니다.
		 *
		 * @param filePath 컴파일할 HLSL 파일 경로 (예: L"Shaders/Basic.hlsl")
		 * @param entryPoint 진입점 함수 이름 (예: "VSMain", "PSMain")
		 * @param target 셰이더 모델 타겟 (예: "vs_5_1", "ps_5_1")
		 * @param outBlob 컴파일된 바이트코드를 받을 Blob 포인터
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note outBlob은 호출자가 Release()로 해제 필요
		 * @warning 파일이 존재하지 않거나 문법 오류 시 false 반환
		 */
		bool CompileFromFile(
			const std::wstring& filePath,
			const std::string& entryPoint,
			const std::string& target,
			ID3DBlob** outBlob
		);

		/**
		 * @brief 사전 컴파일된 CSO 파일을 로드
		 *
		 * FXC 또는 DXC로 사전 컴파일된 셰이더 객체(.cso) 파일을 메모리로 로드합니다.
		 * 런타임 컴파일보다 빠르며 배포 빌드에 권장됩니다.
		 *
		 * @param csoFilePath CSO 파일 경로 (예: L"Shaders/Compiled/VertexShader.cso")
		 * @param outBlob 로드된 바이트코드를 받을 Blob 포인터
		 * @return 성공 시 true, 실패 시 false
		 *
		 * @note outBlob은 호출자가 Release()로 해제 필요
		 * @warning 파일이 존재하지 않거나 읽기 실패 시 false 반환
		 */
		bool LoadCompiledShader(
			const std::wstring& csoFilePath,
			ID3DBlob** outBlob
		);

	private:
		/**
		 * @brief 파일 내용을 메모리로 읽는 헬퍼 함수
		 *
		 * @param filePath 읽을 파일 경로
		 * @param outData 파일 내용을 저장할 벡터
		 * @return 성공 시 true, 실패 시 false
		 */
		bool ReadFileToMemory(
			const std::wstring& filePath,
			std::vector<char>& outData
		);
	};
} // namespace Graphics