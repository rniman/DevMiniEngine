#include "Core/Memory/StackAllocator.h"
#include <iostream>

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "    Stack Allocator Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Create StackAllocator
    Core::Memory::StackAllocator stack(1024);

    std::cout << "Created StackAllocator with 1024 bytes" << std::endl;
    std::cout << "Initial state:" << std::endl;
    std::cout << "  - Allocated: " << stack.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Free: " << stack.GetFreeSpace() << " bytes" << std::endl;
    std::cout << std::endl;

    // Test 1: Basic allocation
    std::cout << "Test 1: Allocate 100 bytes" << std::endl;
    void* ptr1 = stack.Allocate(100, sizeof(void*));
    std::cout << "  - Pointer: " << ptr1 << std::endl;
    std::cout << "  - Allocated: " << stack.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Free: " << stack.GetFreeSpace() << " bytes" << std::endl;
    std::cout << std::endl;

    // Test 2: Nested allocations with marker
    std::cout << "Test 2: Nested allocations with marker" << std::endl;
    auto marker1 = stack.GetMarker();
    std::cout << "  - Marker 1 at: " << marker1 << " bytes" << std::endl;

    void* ptr2 = stack.Allocate(200, sizeof(void*));
    void* ptr3 = stack.Allocate(150, sizeof(void*));
    std::cout << "  - After 2 allocations: " << stack.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Allocation count: " << stack.GetAllocationCount() << std::endl;
    std::cout << std::endl;

    // Test 3: Free to marker
    std::cout << "Test 3: Free to marker" << std::endl;
    stack.FreeToMarker(marker1);
    std::cout << "  - After FreeToMarker: " << stack.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Free: " << stack.GetFreeSpace() << " bytes" << std::endl;
    std::cout << std::endl;

    // Test 4: Multiple scope levels
    std::cout << "Test 4: Multiple scope levels" << std::endl;
    auto marker2 = stack.GetMarker();
    {
        void* temp1 = stack.Allocate(50, sizeof(void*));
        std::cout << "  - Level 1 allocated: " << stack.GetAllocatedSize() << " bytes" << std::endl;

        auto marker3 = stack.GetMarker();
        {
            void* temp2 = stack.Allocate(80, sizeof(void*));
            void* temp3 = stack.Allocate(60, sizeof(void*));
            std::cout << "  - Level 2 allocated: " << stack.GetAllocatedSize() << " bytes" << std::endl;
        }
        stack.FreeToMarker(marker3);
        std::cout << "  - After level 2 freed: " << stack.GetAllocatedSize() << " bytes" << std::endl;
    }
    stack.FreeToMarker(marker2);
    std::cout << "  - After level 1 freed: " << stack.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << std::endl;

    // Test 5: Reset
    std::cout << "Test 5: Reset allocator" << std::endl;
    stack.Allocate(300, sizeof(void*));
    std::cout << "  - Before reset: " << stack.GetAllocatedSize() << " bytes" << std::endl;
    stack.Reset();
    std::cout << "  - After reset: " << stack.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Allocation count: " << stack.GetAllocationCount() << std::endl;
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "    All tests completed!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}