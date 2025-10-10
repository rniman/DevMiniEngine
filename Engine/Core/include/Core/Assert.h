#pragma once

#include <cassert>
#include <stdexcept>

namespace Core
{
    // Debug Build: using assert 
    // Release Build: exception throw
    #ifdef _DEBUG
        #define CORE_ASSERT(condition, message) \
            assert((condition) && (message))
    #else
        #define CORE_ASSERT(condition, message) \
            if (!(condition)) { \
                throw std::runtime_error(message); \
            }
    #endif

    // Fatal Error (Always check)
    #define CORE_VERIFY(condition, message) \
        if (!(condition)) { \
            throw std::runtime_error(message); \
        }

} // namespace Core