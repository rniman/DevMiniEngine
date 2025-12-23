#include "Core/Logging/Logger.h"
#include "Core/Logging/ConsoleSink.h"
#include "Core/Logging/FileSink.h"
#include "Core/Logging/LogMacros.h"
#include <iostream>

int main()
{
    using namespace Core::Logging;

    // Initialize logger
    Logger& logger = Logger::GetInstance();

    // Add console sink
    logger.AddSink(std::make_unique<ConsoleSink>(true));

    // Add file sink
    logger.AddSink(std::make_unique<FileSink>("engine.log"));

    std::cout << "========================================" << std::endl;
    std::cout << "    Logging System Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Test 1: Basic logging
    std::cout << "Test 1: Basic log levels" << std::endl;
    LOG_TRACE("This is a trace message");
    LOG_DEBUG("This is a debug message");
    LOG_INFO("This is an info message");
    LOG_WARN("This is a warning message");
    LOG_ERROR("This is an error message");
    LOG_FATAL("This is a fatal message");
    std::cout << std::endl;

    // Test 2: Formatted messages
    std::cout << "Test 2: Formatted messages" << std::endl;
    int value = 42;
    float pi = 3.14159f;
    const char* name = "DevMiniEngine";

    LOG_INFO("Integer: %d", value);
    LOG_INFO("Float: %.2f", pi);
    LOG_INFO("String: %s", name);
    LOG_INFO("Multiple: %s has value %d and pi %.2f", name, value, pi);
    std::cout << std::endl;

    // Test 3: Category logging
    std::cout << "Test 3: Category-specific logging" << std::endl;
    LOG_GFX_INFO("Rendering %d triangles", 1000);
    LOG(Warn, Physics, "Collision detected at position (%.1f, %.1f, %.1f)", 10.5f, 20.3f, 5.8f);
    std::cout << std::endl;

    // Test 4: Level filtering
    std::cout << "Test 4: Level filtering (set to Warn)" << std::endl;
    logger.SetMinLevel(LogLevel::Warn);

    LOG_TRACE("This trace will be filtered");
    LOG_DEBUG("This debug will be filtered");
    LOG_INFO("This info will be filtered");
    LOG_WARN("This warning will show");
    LOG_ERROR("This error will show");
    std::cout << std::endl;

    // Reset level
    logger.SetMinLevel(LogLevel::Trace);

    // Test 5: Flush
    std::cout << "Test 5: Flush logs" << std::endl;
    LOG_INFO("Flushing all sinks...");
    logger.Flush();
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "    All tests completed!" << std::endl;
    std::cout << "    Check 'engine.log' for file output" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}

//#include "Core/Memory/LinearAllocator.h"
//#include "Core/Memory/StackAllocator.h"
//#include "Core/Memory/PoolAllocator.h"
//#include "Core/Logging/Logger.h"
//#include "Core/Logging/ConsoleSink.h"
//#include "Core/Logging/LogMacros.h"
//
//struct GameObject
//{
//    int id;
//    float position[3];
//    char name[32];
//};
//
//int main()
//{
//    // Initialize logging
//    auto& logger = Core::Logging::Logger::GetInstance();
//    logger.AddSink(std::make_unique<Core::Logging::ConsoleSink>(true));
//
//    LOG_INFO("=== Integrated Memory + Logging Test ===");
//
//    // Test 1: LinearAllocator with logging
//    {
//        LOG_INFO("Test 1: LinearAllocator");
//        Core::Memory::LinearAllocator frameAlloc(1024 * 1024);
//
//        void* data1 = frameAlloc.Allocate(100);
//        LOG_DEBUG("Allocated 100 bytes at 0x%p", data1);
//
//        void* data2 = frameAlloc.Allocate(200);
//        LOG_DEBUG("Allocated 200 bytes at 0x%p", data2);
//
//        LOG_INFO("Total allocated: %zu bytes", frameAlloc.GetAllocatedSize());
//
//        frameAlloc.Reset();
//        LOG_INFO("After reset: %zu bytes", frameAlloc.GetAllocatedSize());
//    }
//
//    // Test 2: PoolAllocator with logging
//    {
//        LOG_INFO("Test 2: PoolAllocator");
//        Core::Memory::PoolAllocator pool(sizeof(GameObject), 10);
//
//        GameObject* obj1 = static_cast<GameObject*>(
//            pool.Allocate(sizeof(GameObject), sizeof(void*))
//            );
//        obj1->id = 1;
//        LOG_DEBUG("Created GameObject ID: %d", obj1->id);
//
//        GameObject* obj2 = static_cast<GameObject*>(
//            pool.Allocate(sizeof(GameObject), sizeof(void*))
//            );
//        obj2->id = 2;
//        LOG_DEBUG("Created GameObject ID: %d", obj2->id);
//
//        LOG_INFO("Pool usage: %zu/%zu chunks",
//            pool.GetAllocationCount(), pool.GetChunkCount());
//
//        pool.Deallocate(obj1);
//        LOG_DEBUG("Freed GameObject ID: 1");
//
//        LOG_INFO("After free: %zu/%zu chunks",
//            pool.GetAllocationCount(), pool.GetChunkCount());
//    }
//
//    // Test 3: StackAllocator with logging
//    {
//        LOG_INFO("Test 3: StackAllocator with nested scopes");
//        Core::Memory::StackAllocator stack(1024 * 1024);
//
//        auto marker1 = stack.GetMarker();
//        LOG_DEBUG("Marker 1 set at %zu", marker1);
//
//        void* data1 = stack.Allocate(512);
//        LOG_DEBUG("Allocated 512 bytes");
//
//        {
//            auto marker2 = stack.GetMarker();
//            LOG_DEBUG("Marker 2 set at %zu", marker2);
//
//            void* data2 = stack.Allocate(256);
//            LOG_DEBUG("Allocated 256 bytes (inner scope)");
//
//            LOG_INFO("Peak usage: %zu bytes", stack.GetAllocatedSize());
//
//            stack.FreeToMarker(marker2);
//            LOG_DEBUG("Freed to marker 2");
//        }
//
//        LOG_INFO("After inner scope: %zu bytes", stack.GetAllocatedSize());
//
//        stack.FreeToMarker(marker1);
//        LOG_DEBUG("Freed to marker 1");
//        LOG_INFO("Final: %zu bytes", stack.GetAllocatedSize());
//    }
//
//    LOG_INFO("=== All tests completed ===");
//
//    return 0;
//}
