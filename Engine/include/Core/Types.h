#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <map>

namespace Core
{
    //=============================================================================
    // 기본 정수 타입
    //=============================================================================
    using int8 = std::int8_t;
    using int16 = std::int16_t;
    using int32 = std::int32_t;
    using int64 = std::int64_t;

    using uint8 = std::uint8_t;
    using uint16 = std::uint16_t;
    using uint32 = std::uint32_t;
    using uint64 = std::uint64_t;

    //=============================================================================
    // 부동소수점 타입
    //=============================================================================
    using float32 = float;
    using float64 = double;

    //=============================================================================
    // 크기 및 포인터 타입
    //=============================================================================
    using size_t = std::size_t;
    using ptrdiff_t = std::ptrdiff_t;

    //=============================================================================
    // 문자열 타입
    //=============================================================================
    using String = std::string;
    using WString = std::wstring;  // Windows Unicode

    //=============================================================================
    // 스마트 포인터
    //=============================================================================
    template<typename T>
    using UniquePtr = std::unique_ptr<T>;

    template<typename T>
    using SharedPtr = std::shared_ptr<T>;

    template<typename T>
    using WeakPtr = std::weak_ptr<T>;

    //=============================================================================
    // 컨테이너
    //=============================================================================
    template<typename T>
    using Vector = std::vector<T>;

    template<typename T, size_t N>
    using Array = std::array<T, N>;

    template<typename Key, typename Value>
    using HashMap = std::unordered_map<Key, Value>;

    template<typename Key, typename Value>
    using Map = std::map<Key, Value>;

    //=============================================================================
    // 헬퍼 함수
    //=============================================================================

    /**
     * @brief UniquePtr 생성 헬퍼
     *
     * @tparam T 생성할 객체 타입
     * @tparam Args 생성자 인자 타입들
     * @param args 생성자에 전달할 인자들
     * @return 생성된 UniquePtr<T>
     */
    template<typename T, typename... Args>
    UniquePtr<T> MakeUnique(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    /**
     * @brief SharedPtr 생성 헬퍼
     *
     * @tparam T 생성할 객체 타입
     * @tparam Args 생성자 인자 타입들
     * @param args 생성자에 전달할 인자들
     * @return 생성된 SharedPtr<T>
     */
    template<typename T, typename... Args>
    SharedPtr<T> MakeShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace Core