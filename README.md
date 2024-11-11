# Bedrock 🪨

Bedrock is an STL alternative. Smaller, simpler, in many case faster. It's meant to be **comfy** (it's a bed) for **me** (made of rock, maybe not to everyone's taste).

```c++

Vector<int>           vector;        // Roughly equivalent to std::vector<int>, with extra useful methods (Find, SwapErase, etc.)
Span<int>             span;          // Roughly equivalent to std::span<int>
String                string;        // Roughly equivalent to std::string
StringView            string_view;   // Roughly equivalent to std::string_view
HashMap<int, int>     hash_map;      // Dense open addressed (Robin Hood) hash map. Key-value pairs are stored contiguously.
HashSet<int>          hash_set;      // Same as HashMap, but without values.

// All containers also come in a Temp flavor that doesn't use the heap, and instead allocate from a stack-like memory allocator.
// Allocation is extremely fast, and containers can resize without moving elements as long as they're the last allocation.
TempVector<int>       temp_vector;
TempString            temp_string;
TempHashMap<int, int> temp_hash_map;
TempHashSet<int>      temp_hash_set;

```
