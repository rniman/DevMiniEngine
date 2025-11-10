#include "pch.h"
#include "Graphics/DX12/DX12ShaderCompiler.h"
#include <fstream>

using namespace std;

namespace Graphics
{
	bool DX12ShaderCompiler::CompileFromFile(
		const wstring& filePath,
		const string& entryPoint,
		const string& target,
		ID3DBlob** outBlob
	)
	{
		// 빌드 구성에 따른 컴파일 플래그 설정
		UINT compileFlags = 0;
#ifdef _DEBUG
		// Debug: 디버그 심볼 포함 및 최적화 비활성화로 디버깅 용이성 확보
		compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		// Release: 최고 수준 최적화로 런타임 성능 최대화
		compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#endif

		ComPtr<ID3DBlob> errorBlob;

		// D3DCompileFromFile: 파일에서 직접 컴파일 (메모리 로딩 불필요)
		// D3D_COMPILE_STANDARD_FILE_INCLUDE: #include 지시자 자동 처리
		HRESULT hr = D3DCompileFromFile(
			filePath.c_str(),
			nullptr,                               // 매크로 정의 없음
			D3D_COMPILE_STANDARD_FILE_INCLUDE,     // 표준 include 핸들러
			entryPoint.c_str(),
			target.c_str(),
			compileFlags,
			0,                                     // 효과 컴파일 플래그 (사용 안 함)
			outBlob,
			errorBlob.GetAddressOf()
		);

		if (FAILED(hr))
		{
			// 컴파일 에러 메시지가 있으면 출력 (문법 오류, 타입 불일치 등)
			if (errorBlob)
			{
				LOG_ERROR("Shader compilation failed: %s", static_cast<const char*>(errorBlob->GetBufferPointer()));
			}
			else
			{
				// errorBlob이 없는 경우 파일 없음 또는 시스템 오류
				LOG_ERROR("Shader compilation failed with HRESULT: 0x%08X", hr);
			}
			return false;
		}

		LOG_INFO("Shader compiled successfully: %ls (%s)",
			filePath.c_str(),
			target.c_str()
		);
		return true;
	}

	bool DX12ShaderCompiler::LoadCompiledShader(
		const wstring& csoFilePath,
		ID3DBlob** outBlob
	)
	{
		// 1단계: CSO 파일을 메모리로 읽기
		vector<char> csoData;
		if (!ReadFileToMemory(csoFilePath, csoData))
		{
			return false;
		}

		// 2단계: 빈 ID3DBlob 생성
		HRESULT hr = D3DCreateBlob(csoData.size(), outBlob);
		if (FAILED(hr))
		{
			LOG_ERROR("D3DCreateBlob failed: 0x%08X", hr);
			return false;
		}

		// 3단계: 파일 데이터를 Blob으로 복사
		// memcpy 사용 이유: 단순 바이트 복사로 가장 빠름
		memcpy(
			(*outBlob)->GetBufferPointer(),
			csoData.data(),
			csoData.size()
		);

		LOG_INFO("CSO loaded successfully: %ls", csoFilePath.c_str());
		return true;
	}

	bool DX12ShaderCompiler::ReadFileToMemory(
		const wstring& filePath,
		vector<char>& outData
	)
	{
		// ios::ate: 파일 끝에서 시작하여 tellg()로 크기 즉시 확인 가능
		ifstream file(filePath, ios::binary | ios::ate);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to open file: %ls", filePath.c_str());
			return false;
		}

		// 현재 위치(파일 끝)에서 크기 확인 후 처음으로 되돌림
		streamsize fileSize = file.tellg();
		file.seekg(0, ios::beg);

		// 한 번에 전체 파일 읽기 (작은 셰이더 파일에 효율적)
		outData.resize(static_cast<size_t>(fileSize));
		if (!file.read(outData.data(), fileSize))
		{
			LOG_ERROR("Failed to read file: %ls", filePath.c_str());
			return false;
		}

		return true;
	}
} // namespace Graphics
