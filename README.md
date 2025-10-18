# Original



## 简介

Original是一个C++基础工具库,也是本人的第一个正式项目，用于学习C++相关基础以及STL、Boost等库。项目的目标是实现STL的主要内容以及Boost的某些相关功能，具体已经实现和未来计划实现的模块内容在下文列出，计划实现的部分会根据情况进行调整，不一定都实现，以最终项目为准。如有问题可以在[Issues](https://github.com/FrozenLemonTee/original/issues)中提出，也欢迎一起参与到本项目的实现中来，如Fork等。

## 文档
[文档-Original](https://documents-original.vercel.app/)

## 环境要求

为了编译和使用 Original，请确保开发环境满足以下最低版本要求：

- **C++ 标准**: C++23
- **CMake**: 3.31 或更高版本
- **编译器**（任选其一）：
    - **GCC**: 13.0 或更高版本
    - **Clang**: 20.0 或更高版本
    - **MSVC**（Visual Studio 2022）：17.10 (版本 14.44.35207) 或更高版本

> 注意：如果使用 GCC 或 Clang，请确保支持 `-std=c++23` 标志。

## 安装

这里以项目`hello_original`为例：
```text
├─CMakeLists.txt
└─main.cpp
```

方法一 使用Cmake远程拉取（推荐）：

配置`CMakeLists.txt`：
```cmake
cmake_minimum_required(VERSION 3.31)
project(hello_original)

set(CMAKE_CXX_STANDARD 23)

set(BUILD_TESTING OFF CACHE BOOL "Disable tests in the fetched project")

include(FetchContent)

FetchContent_Declare(
        original
        GIT_REPOSITORY git@github.com:FrozenLemonTee/original.git
        GIT_TAG master
)

FetchContent_MakeAvailable(original)

add_executable(hello_original main.cpp)


target_link_libraries(hello_original PRIVATE original)
```
方法二 使用Cmake本地构建、安装：

在本项目文件夹下运行下列命令：
```shell
cmake -P install.cmake
```
将生成的`original`文件夹复制到项目`hello_original`中。

复制后项目的结构如下：
```text
├─original
├─CMakeLists.txt
└─main.cpp
```

配置`CMakeLists.txt`：
```cmake
cmake_minimum_required(VERSION 3.31)
project(hello_original)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

list(APPEND CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/original")
find_package(original REQUIRED)

add_executable(hello_original main.cpp)
target_link_libraries(hello_original PRIVATE original)
```

接下来展示测试Demo：

`main.cpp`：
```c++
#include <iostream>
#include "original.h"

int main() {
    const original::array a = {"hello world!"};
    std::cout << a << std::endl;
    return 0;
}
```
输出：
```text
array("hello world!")
```

## 模块进度

#### Core

包含基本算法、容器等工具。

##### 容器：

定长容器：定长数组 array，位集合 bitSet，变长容器：变长数组 vector，单向链表 forwardChain，双向链表 chain，块状链表 blocksList，关联容器：映射表 hashMap/treeMap，集合 hashSet/treeSet，跳跃表JSet/JMap

##### 容器接口：

格式化输出接口 printable，元素比较接口 comparable，堆对象深拷贝接口 cloneable，可迭代接口 iterable

##### 算法库：

布尔算法：allOf/anyOf/noneOf...，非修改算法 find/count/equal...，修改算法 fill/swap/forEach/...，排序算法 sort/stableSort/introSort...

##### 容器适配器：

栈 stack，队列 queue，双端队列 deque，优先队列 prique

##### 通用类型封装类：

存在值可选封装 alternative

##### 算法适配器：

迭代器 iterator/iterAdaptor，变换器 transform/transformStream，过滤器 filter/filterStream，比较器 comparator

##### 异常安全：

运行时异常 error/outOfBoundError/unSupportedMethodError/allocateError...，编译期错误 staticError

##### 编译期工具：

二元组 couple，多元组 tuple

##### 内存管理：

自动指针 ownerPtr/strongPtr/weakPtr，分配器 allocatorBase/allocator/objPoolAllocator，删除器 deleterBase/deleter，单例模式 singleton


#### Vibrant

并发模块，正在实现。

##### 线程：

基类 threadBase，POSIX类线程 pThread，线程 thread

##### 临界区管理：

互斥量 mutexBase/pMutex，锁管理器 lockGuard/uniqueLock/multiLock，信号量 semaphore/semaphoreGuard

##### 线程同步：

条件变量 conditionBase/pCondition，线程同步点 syncPoint

##### 原子操作：

原子变量 atomic

##### 跨线程生产/消费:

跨线程生产者 async::promise，跨线程消费者 async::futureBase/async::future/async::sharedFuture

##### 时间表示：

时间段 time::duration，时间戳 time::point，UTC时间 time::UTCTime

##### 任务调度：

任务包装类 taskBase/task，任务委派器 taskDelegator

##### 协程：

生成器 coroutine::generator

#### matrix

计划实现，包含张量，线性代数工具功能。

#### graph

计划实现，包含图论结构和图算法。


