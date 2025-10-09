#include "Core/Memory/LinearAllocator.h"
#include <iostream>

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "    Memory Allocator Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Create LinearAllocator
    Core::Memory::LinearAllocator allocator(1024);

    std::cout << "Created LinearAllocator with 1024 bytes" << std::endl;
    std::cout << "Initial state:" << std::endl;
    std::cout << "  - Allocated: " << allocator.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Free: " << allocator.GetFreeSpace() << " bytes" << std::endl;
    std::cout << std::endl;

    // Test 1: Basic allocation
    std::cout << "Test 1: Allocate 100 bytes" << std::endl;
    void* ptr1 = allocator.Allocate(100);
    std::cout << "  - Pointer: " << ptr1 << std::endl;
    std::cout << "  - Allocated: " << allocator.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Free: " << allocator.GetFreeSpace() << " bytes" << std::endl;
    std::cout << std::endl;

    // Test 2: Multiple allocations
    std::cout << "Test 2: Allocate 200 bytes" << std::endl;
    void* ptr2 = allocator.Allocate(200);
    std::cout << "  - Pointer: " << ptr2 << std::endl;
    std::cout << "  - Allocated: " << allocator.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Free: " << allocator.GetFreeSpace() << " bytes" << std::endl;
    std::cout << "  - Allocation count: " << allocator.GetAllocationCount() << std::endl;
    std::cout << std::endl;

    // Test 3: Aligned allocation
    std::cout << "Test 3: Allocate 64 bytes with 64-byte alignment" << std::endl;
    void* ptr3 = allocator.Allocate(64, 64);
    std::cout << "  - Pointer: " << ptr3 << std::endl;
    std::cout << "  - Is aligned: " << (reinterpret_cast<size_t>(ptr3) % 64 == 0 ? "Yes" : "No") << std::endl;
    std::cout << "  - Allocated: " << allocator.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << std::endl;

    // Test 4: Reset
    std::cout << "Test 4: Reset allocator" << std::endl;
    allocator.Reset();
    std::cout << "  - Allocated: " << allocator.GetAllocatedSize() << " bytes" << std::endl;
    std::cout << "  - Free: " << allocator.GetFreeSpace() << " bytes" << std::endl;
    std::cout << "  - Allocation count: " << allocator.GetAllocationCount() << std::endl;
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "    All tests completed!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}