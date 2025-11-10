#include "ECSRotatingCubeApp.h"

#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/LogMacros.h"

#include <exception>
#include <memory>

int main()
{
	// 로깅 시스템 초기화
	auto& logger = Core::Logging::Logger::GetInstance();
	logger.AddSink(std::make_unique<Core::Logging::ConsoleSink>(true));

	try
	{
		LOG_INFO("=== 09_ECSRotatingCube Sample Started ===");

		ECSRotatingCubeApp app;
		app.Run();

		LOG_INFO("=== Sample Ended Successfully ===");
		return 0;
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("Fatal error: %s", e.what());
		return -1;
	}
}
