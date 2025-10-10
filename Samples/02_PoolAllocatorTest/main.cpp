#include "Core/Memory/PoolAllocator.h"
#include <iostream>

struct TestObject
{
    int id;
    float value;
    char name[32];
};

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "    Pool Allocator Test" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << std::endl;

    // Create PoolAllocator for TestObject
    Core::Memory::PoolAllocator pool(sizeof(TestObject), 10);

    std::cout << "Created PoolAllocator:" << std::endl;
    std::cout << "  - Chunk size: " << pool.GetChunkSize() << " bytes" << std::endl;
    std::cout << "  - Chunk count: " << pool.GetChunkCount() << std::endl;
    std::cout << "  - Free chunks: " << pool.GetFreeChunkCount() << std::endl;
    std::cout << std::endl;

    // Test 1: Allocate single object
    std::cout << "Test 1: Allocate single object" << std::endl;
    TestObject* obj1 = static_cast<TestObject*>(pool.Allocate(sizeof(TestObject), sizeof(void*)));
    obj1->id = 1;
    obj1->value = 3.14f;
    std::cout << "  - Allocated: " << obj1 << std::endl;
    std::cout << "  - Free chunks: " << pool.GetFreeChunkCount() << std::endl;
    std::cout << std::endl;

    // Test 2: Allocate multiple objects
    std::cout << "Test 2: Allocate 5 objects" << std::endl;
    TestObject* objects[5];
    for (int i = 0; i < 5; ++i)
    {
        objects[i] = static_cast<TestObject*>(pool.Allocate(sizeof(TestObject), sizeof(void*)));
        objects[i]->id = i + 2;
        objects[i]->value = static_cast<float>(i);
    }
    std::cout << "  - Allocated: " << pool.GetAllocationCount() << " objects" << std::endl;
    std::cout << "  - Free chunks: " << pool.GetFreeChunkCount() << std::endl;
    std::cout << std::endl;

    // Test 3: Deallocate some objects
    std::cout << "Test 3: Deallocate 2 objects" << std::endl;
    pool.Deallocate(objects[1]);
    pool.Deallocate(objects[3]);
    std::cout << "  - Allocated: " << pool.GetAllocationCount() << " objects" << std::endl;
    std::cout << "  - Free chunks: " << pool.GetFreeChunkCount() << std::endl;
    std::cout << std::endl;

    // Test 4: Reuse deallocated chunks
    std::cout << "Test 4: Reuse deallocated chunks" << std::endl;
    TestObject* obj2 = static_cast<TestObject*>(pool.Allocate(sizeof(TestObject), sizeof(void*)));
    obj2->id = 100;
    std::cout << "  - Allocated: " << obj2 << std::endl;
    std::cout << "  - Free chunks: " << pool.GetFreeChunkCount() << std::endl;
    std::cout << std::endl;

    // Test 5: Reset pool
    std::cout << "Test 5: Reset pool" << std::endl;
    pool.Reset();
    std::cout << "  - Allocated: " << pool.GetAllocationCount() << " objects" << std::endl;
    std::cout << "  - Free chunks: " << pool.GetFreeChunkCount() << std::endl;
    std::cout << std::endl;

    std::cout << "========================================" << std::endl;
    std::cout << "    All tests completed!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}