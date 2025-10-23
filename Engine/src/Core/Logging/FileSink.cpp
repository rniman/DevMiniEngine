#include "Core/Logging/FileSink.h"
#include <iomanip>
#include <sstream>

using namespace std;

namespace Core
{
    namespace Logging
    {
        FileSink::FileSink(const string& filename)
            : mFilename(filename)
        {
            mFile.open(filename, ios::out | ios::app);

            if (!mFile.is_open())
            {
                // NOTE: 로깅 시스템 초기화 중이므로 LOG_ERROR 사용 불가
                // stderr로 직접 출력하거나 예외 던지기
                throw runtime_error("Failed to open log file: " + filename);
            }
        }

        FileSink::~FileSink()
        {
            if (mFile.is_open())
            {
                mFile.flush();  // 버퍼 내용 강제 기록
            }
            // close()는 ofstream 소멸자에서 자동 호출됨
        }

        void FileSink::Write(const LogMessage& msg)
        {
            if (mFile.is_open())
            {
                mFile << FormatLogMessage(msg) << endl;
            }
        }

        void FileSink::Flush()
        {
            if (mFile.is_open())
            {
                mFile.flush();
            }
        }

        string FileSink::FormatLogMessage(const LogMessage& msg)
        {
            stringstream ss;

            // 파일 로그는 검색과 정렬을 위해 ISO 8601 형식 사용 (YYYY-MM-DD HH:MM:SS)
            // ConsoleSink는 간결성을 위해 시간만 표시
            auto time = chrono::system_clock::to_time_t(msg.timestamp);
            tm tm;
            localtime_s(&tm, &time);

            ss << "[" << put_time(&tm, "%Y-%m-%d %H:%M:%S") << "] ";
            ss << "[" << LogLevelToString(msg.level) << "] ";
            ss << "[" << LogCategoryToString(msg.category) << "] ";
            ss << msg.message;

            // 파일 로그는 항상 파일 위치 포함 (디버깅 용이성)
            ss << " (" << msg.file << ":" << msg.line << ")";

            return ss.str();
        }

    } // namespace Logging
} // namespace Core