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
    // Basic Integer Types
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
    // Floating Point
    //=============================================================================
    using float32 = float;
    using float64 = double;

    //=============================================================================
    // Size and Pointer Types
    //=============================================================================
    using size_t = std::size_t;
    using ptrdiff_t = std::ptrdiff_t;

    //=============================================================================
    // String Types
    //=============================================================================
    using String = std::string;
    using WString = std::wstring;  // Windows Unicode

    //=============================================================================
    // Smart Pointers
    //=============================================================================
    template<typename T>
    using UniquePtr = std::unique_ptr<T>;

    template<typename T>
    using SharedPtr = std::shared_ptr<T>;

    template<typename T>
    using WeakPtr = std::weak_ptr<T>;

    //=============================================================================
    // Containers
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
    // Helper Functions
    //=============================================================================

    template<typename T, typename... Args>
    UniquePtr<T> MakeUnique(Args&&... args)
    {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template<typename T, typename... Args>
    SharedPtr<T> MakeShared(Args&&... args)
    {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

} // namespace Core