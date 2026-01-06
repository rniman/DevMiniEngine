/**
 * @file main.cpp
 * @brief 11_ModelViewer 엔트리 포인트
 *
 * Phase 4.1: Asset Pipeline 데모
 * - AssetManager 기본 기능 테스트
 * - 프로시저럴 Sphere 렌더링
 */
#include "ModelViewerApp.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/FileSink.h"
#include "Core/Logging/Logger.h"
#include "Core/Logging/LogMacros.h"
#include <exception>
#include <memory>

int main()
{
	// 로깅 시스템 초기화
	auto& logger = Core::Logging::Logger::GetInstance();
	logger.AddSink(std::make_unique<Core::Logging::ConsoleSink>(true));
	// logger.AddSink(std::make_unique<Core::Logging::FileSink>("11_MODELVIEWER_LOG.txt"));

	try
	{
		LOG_INFO("=== 11_ModelViewer Sample Started ===");

		ModelViewerApp app;
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
