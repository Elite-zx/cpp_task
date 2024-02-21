#include <iostream>
#include <type_traits>

// 在任何块作用域内……
{
    // 在栈上静态分配拥有自动存储期的存储，对任何对象类型 “T” 足够大。
    alignas(T) unsigned char buf[sizeof(T)];

    T* tptr = new(buf) T; // 构造一个 “T” 对象，将它直接置于
                          // 你预分配的位于内存地址 “buf” 的存储。

    tptr->~T();           // 如果程序依赖对象的析构函数的副作用，你必须**手动**调用它。
}                         // 离开此块作用域自动解分配 “buf”。
