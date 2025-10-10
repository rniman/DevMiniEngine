#pragma once

#include <cassert>
#include <stdexcept>

namespace Core
{
	/**
	 * @brief Debug-only assertion
	 * @note Compiled out in Release builds
	 */
	#ifdef _DEBUG
		#define CORE_ASSERT(condition, message) \
			assert((condition) && (message))
	#else
		#define CORE_ASSERT(condition, message) \
			if (!(condition)) { \
				throw std::runtime_error(message); \
			}
	#endif

	 /**
	  * @brief Always-checked assertion
	  * @note Active in both Debug and Release builds
	  */
	#define CORE_VERIFY(condition, message) \
		if (!(condition)) { \
			throw std::runtime_error(message); \
		}

} // namespace Core