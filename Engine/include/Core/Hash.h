// Engine/include/Core/Hash.h
#pragma once
#include "Core/Types.h"
#include <string>

namespace Core
{
	/**
	 * @brief 64비트 FNV-1a 해시 (바이트 기반)
	 *
	 * UTF-8 문자열만 지원
	 * 모든 경로는 UTF-8로 통일
	 */
	inline uint64 Hash64(const char* data, size_t length)
	{
		constexpr uint64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
		constexpr uint64 FNV_PRIME = 1099511628211ULL;

		uint64 hash = FNV_OFFSET_BASIS;
		for (size_t i = 0; i < length; ++i)
		{
			hash ^= static_cast<uint64>(static_cast<unsigned char>(data[i]));
			hash *= FNV_PRIME;
		}
		return hash;
	}

	inline uint64 Hash64(const std::string& str)
	{
		return Hash64(str.c_str(), str.length());
	}

	/**
	 * @brief wstring - UTF-8 변환
	 *
	 * Windows API 호환성을 위해 필요
	 */
	inline std::string WStringToUTF8(const std::wstring& wstr)
	{
		if (wstr.empty())
		{
			return std::string();
		}

#ifdef _WIN32
		int sizeNeeded = WideCharToMultiByte(
			CP_UTF8,
			0,
			wstr.c_str(),
			static_cast<int>(wstr.size()),
			nullptr,
			0,
			nullptr,
			nullptr
		);

		std::string utf8(sizeNeeded, 0);
		WideCharToMultiByte(
			CP_UTF8,
			0,
			wstr.c_str(),
			static_cast<int>(wstr.size()),
			&utf8[0],
			sizeNeeded,
			nullptr,
			nullptr
		);

		return utf8;
#else
		// Linux/Mac: wstring - UTF-8 변환 구현
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.to_bytes(wstr);
#endif
	}

	// wstring - UTF-8 변환 후 해시
	inline uint64 Hash64(const std::wstring& wstr)
	{
		// UTF-16 - UTF-8 변환
		std::string utf8 = WStringToUTF8(wstr);
		return Hash64(utf8);
	}

	/**
	 * @brief UTF-8 - wstring 변환
	 *
	 * Windows API 호환성을 위해 필요
	 */
	inline std::wstring UTF8ToWString(const std::string& str)
	{
		if (str.empty())
		{
			return std::wstring();
		}

#ifdef _WIN32
		int sizeNeeded = MultiByteToWideChar(
			CP_UTF8,
			0,
			str.c_str(),
			static_cast<int>(str.size()),
			nullptr,
			0
		);

		std::wstring wstr(sizeNeeded, 0);
		MultiByteToWideChar(
			CP_UTF8,
			0,
			str.c_str(),
			static_cast<int>(str.size()),
			&wstr[0],
			sizeNeeded
		);

		return wstr;
#else
		// Linux/Mac: UTF-8 - wstring 변환 구현
		std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
		return converter.from_bytes(str);
#endif
	}

} // namespace Core
