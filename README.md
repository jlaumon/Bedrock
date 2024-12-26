# Bedrock 🪨

Bedrock is an STL alternative. Smaller, simpler, in many case faster. It's meant to be **comfy** (it's a bed) for **me** (made of rock, maybe not to everyone's taste).

Currently Windows only. Supports MSVC and Clang.

```c++
Vector<int>         // Roughly equivalent to std::vector<int>, with extra useful methods (Find, SwapErase, etc.)
Span<int>           // Roughly equivalent to std::span<int>
String              // Roughly equivalent to std::string
StringView          // Roughly equivalent to std::string_view
HashMap<int, int>   // Dense open addressed (Robin Hood) hash map. Key-value pairs are stored contiguously.
HashSet<int>        // Same as HashMap, but without values.

// All containers also come in different allocator flavors.
TempVector<int>     // Allocates from a thread local arena. Falls back to the heap if it runs out.
FixedVector<int>    // Allocates from a fixed-size arena embedded in the container.
VMemVector<int>     // Allocates from a virtual memory arena embedded in the container. Can grow while keeping a stable address.
ArenaVector<int>    // Allocates from an externally provided arena.

```
