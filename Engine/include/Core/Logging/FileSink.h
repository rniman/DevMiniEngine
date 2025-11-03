#pragma once

#include "Core/Logging/LogSink.h"
#include <fstream>
#include <string>

namespace Core
{
    namespace Logging
    {
        /**
         * @brief 파일 출력 Sink
         * 
         * @note append 모드로 파일을 열어 기존 로그를 보존합니다.
         * @note 스레드 안전성은 Logger의 뮤텍스에 의존합니다.
         */
        class FileSink : public LogSink
        {
        public:
            /**
             * @brief 파일 경로를 지정하여 FileSink 생성
             * 
             * @param filename 로그 파일 경로
             */
            explicit FileSink(const std::string& filename);

            ~FileSink() override;

            void Write(const LogMessage& msg) override;

            /**
             * @brief 버퍼를 디스크에 즉시 기록
             */
            void Flush() override;

        private:
            std::string FormatLogMessage(const LogMessage& msg);

            std::ofstream mFile;
            std::string mFilename;
        };

    } // namespace Logging
} // namespace Core