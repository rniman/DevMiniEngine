#include "Core/Logging/FileSink.h"
#include <iomanip>
#include <sstream>

namespace Core
{
    namespace Logging
    {
        FileSink::FileSink(const std::string& filename)
            : mFilename(filename)
        {
            mFile.open(filename, std::ios::out | std::ios::app);
        }

        FileSink::~FileSink()
        {
            if (mFile.is_open())
            {
                mFile.close();
            }
        }

        void FileSink::Write(const LogMessage& msg)
        {
            if (mFile.is_open())
            {
                mFile << FormatLogMessage(msg) << std::endl;
            }
        }

        void FileSink::Flush()
        {
            if (mFile.is_open())
            {
                mFile.flush();
            }
        }

        std::string FileSink::FormatLogMessage(const LogMessage& msg)
        {
            std::stringstream ss;

            // Timestamp
            auto time = std::chrono::system_clock::to_time_t(msg.timestamp);
            std::tm tm;
            localtime_s(&tm, &time);

            ss << "[" << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
            ss << "[" << LogLevelToString(msg.level) << "] ";
            ss << "[" << LogCategoryToString(msg.category) << "] ";
            ss << msg.message;
            ss << " (" << msg.file << ":" << msg.line << ")";

            return ss.str();
        }

    } // namespace Logging
} // namespace Core