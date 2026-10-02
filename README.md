# uglystl
Test STL For Me or Someone weirdo.

I built this because I hate the complexity of C++ move semantics, template errors, and bad compatibility with C.

This is ugly, and it's not a modern standards. But easy to use.

## ai dependency 
- uglystl.hpp : <20% (idea)
- uglyjson.hpp : <10% (`YourJsonData` - `std::variant`)

believe or not, ai code 0%.

evidence : [Link](https://www.youtube.com/watch?v=6wtymP7j0SE)

## code
```cpp
namespace ug;
ug::Vector<int> s(ug::Memory::GetGlobalMemory());
s.Push(100);
s.Push(200);
s.Push(300);
```
## uglyjson
jsonparser implemented 
`MyJsonData` : NO RAII TYPE
`YourJsonData` : implemented by `std::unique_ptr` and `std::variant`

testing. You can use both. (not stability)
`MyJsonParser` : Parser
```cpp
MyJsonParser<YourJsonData> parse;
parse.New(~~);blablaa


```
