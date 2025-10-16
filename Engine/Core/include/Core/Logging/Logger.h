#pragma once

#include "Core/Logging/LogTypes.h"
#include "Core/Logging/LogSink.h"
#include "Core/Types.h"
#include <vector>
#include <memory>
#include <mutex>

namespace Core
{
    namespace Logging
    {
        /**
         * @brief 메인 로깅 시스템 (싱글톤)
         *
         * @note 스레드 안전: 여러 스레드에서 동시 호출 가능
         * @note 매크로 사용 권장 (LOG_INFO, LOG_ERROR 등)
         */
        class Logger
        {
        public:
            static Logger& GetInstance();

            /**
             * @brief 로그 메시지 기록
             *
             * @note 최소 로그 레벨보다 낮은 메시지는 필터링됨
             */
            void Log(
                LogLevel level,
                LogCategory category,
                const std::string& message,
                const char* file,
                int line
            );

            /**
             * @brief 출력 대상(Sink) 추가
             * @param sink 추가할 LogSink (소유권 이전)
             */
            void AddSink(std::unique_ptr<LogSink> sink);

            /**
             * @brief 모든 Sink 제거
             */
            void ClearSinks();

            /**
             * @brief 최소 로그 레벨 설정
             */
            void SetMinLevel(LogLevel level) { mMinLevel = level; }

            /**
             * @brief 최소 로그 레벨 조회
             */
            LogLevel GetMinLevel() const { return mMinLevel; }

            /**
             * @brief 모든 Sink의 버퍼를 즉시 출력
             */
            void Flush();

        private:
            Logger() = default;
            ~Logger() = default;

            // 복사 및 이동 금지 (싱글톤)
            Logger(const Logger&) = delete;
            Logger& operator=(const Logger&) = delete;

            std::vector<std::unique_ptr<LogSink>> mSinks;
            LogLevel mMinLevel = LogLevel::Trace;
            std::mutex mMutex;
        };

    } // namespace Logging
} // namespace Core