#pragma once

#include "Core/Logging/LogTypes.h"

namespace Core
{
    namespace Logging
    {
        /**
         * @brief 로그 출력 대상을 위한 기본 인터페이스
         *
         * @note 새로운 출력 대상 추가 시 이 클래스를 상속하여 구현
         */
        class LogSink
        {
        public:
            virtual ~LogSink() = default;

            /**
             * @brief 로그 메시지 출력
             */
            virtual void Write(const LogMessage& msg) = 0;

            /**
             * @brief 버퍼링된 메시지 즉시 출력
             */
            virtual void Flush() {}

        protected:
            LogSink() = default;

            // 복사 금지
            LogSink(const LogSink&) = delete;
            LogSink& operator=(const LogSink&) = delete;
        };

    } // namespace Logging
} // namespace Core