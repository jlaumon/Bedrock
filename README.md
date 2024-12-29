# Bedrock 🪨

Bedrock is a C++20 STL alternative. Smaller, simpler, in many case faster. It's a hobby project, don't expect much more than an interesting implementation reference for things.

Currently Windows only. Supports MSVC and Clang. There are no concrete plans to support more platforms/compilers at this time. 

## Containers and Views

```c++
Vector<int>         // Roughly equivalent to std::vector<int>, with extra useful methods (Find, SwapErase, etc.)
Span<int>           // Roughly equivalent to std::span<int>
String              // Roughly equivalent to std::string
StringView          // Roughly equivalent to std::string_view
HashMap<int, int>   // Dense open addressed (Robin Hood) hash map. Key-value pairs are stored contiguously.
HashSet<int>        // Same as HashMap, but without values.
```

## Allocators 

```c++
// All containers also come in different allocator flavors.
TempVector<int>     // Allocates from a thread local arena. Falls back to the heap if it runs out.
FixedVector<int>    // Allocates from a fixed-size arena embedded in the container.
VMemVector<int>     // Allocates from a virtual memory arena embedded in the container. Can grow while keeping a stable address.
ArenaVector<int>    // Allocates from an externally provided arena.

```

## Tests

Write tests anywhere:

```c++
REGISTER_TEST("Span")
{
    int values[] = { 1, 2, 3, 4, 5 };
    Span test = values;

    Span first_two = test.First(2);
    TEST_TRUE(first_two.Size() == 2);
    TEST_TRUE(first_two[0] == 1);
    TEST_TRUE(first_two[1] == 2);
};
```

The run them with `gRunAllTests()`. 

## Other

Mutex, Atomic, Thread, Semaphore. 
Function, many Type Traits, a few Algorithms...

## Building

Compile every cpp file in Bedrock/. Define `ASSERTS_ENABLED` if you want asserts and tests. That's about it. 
