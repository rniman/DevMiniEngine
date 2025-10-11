#pragma once

#include "Core/Logging/LogSink.h"
#include <fstream>
#include <string>

namespace Core
{
    namespace Logging
    {
        /**
         * @brief File output sink
         */
        class FileSink : public LogSink
        {
        public:
            explicit FileSink(const std::string& filename);
            ~FileSink() override;

            void Write(const LogMessage& msg) override;
            void Flush() override;

        private:
            std::string FormatLogMessage(const LogMessage& msg);

            std::ofstream mFile;
            std::string mFilename;
        };

    } // namespace Logging
} // namespace Core