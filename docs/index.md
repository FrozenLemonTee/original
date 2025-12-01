# Original, A C++ Tool Library



## 📖 简介

Original是一个C++基础工具库,也是本人的第一个正式项目，用于学习C++相关基础以及STL、Boost等库。项目的目标是实现STL的主要内容以及Boost的某些相关功能，具体已经实现和未来计划实现的模块内容在下文列出，计划实现的部分会根据情况进行调整，不一定都实现，以最终项目为准。

[Github地址](https://github.com/FrozenLemonTee/original)

对于相关类和方法文档的查看，

1. 网页端：直接点击上方菜单选项；

2. 移动端：点击左上方菜单折叠栏，展开菜单后点击菜单选项。

如有问题可以在[Issues](https://github.com/FrozenLemonTee/original/issues)中提出，也欢迎一起参与到本项目的实现中来，如Fork等。



## 📚 文档版本选择

### 🏷️ 稳定版本 (Stable)
- **版本**: master 分支最新版本
- [进入稳定版文档](../stable/index.html)

### 🔥 最新版本 (Latest)
- **版本**: test 分支最新构建
- [进入最新版文档](../latest/index.html)

### 📋 历史版本 (Historical Versions)
- **版本**: 按发布标签
- [查看所有历史版本](../versions/index.html)

---


## 环境要求

为了编译和使用 Original，请确保开发环境满足以下最低版本要求：

- **C++ 标准**: C++23
- **CMake**: 3.31 或更高版本
- **编译器**（任选其一）：
    - **GCC**: 13.0 或更高版本
    - **Clang**: 20.0 或更高版本
    - **MSVC**（Visual Studio 2022）：17.10 (版本 14.44.35207) 或更高版本

> 注意：如果使用 GCC 或 Clang，请确保支持 `-std=c++23` 标志。

## 🚀 快速开始

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

## 📊 模块进度

<details>
<summary><strong>Core</strong>（基础模块：容器 / 算法 / 内存等）</summary>

### 容器
- **array**（定长数组）
- **bitSet**（位集合）
- **vector**（动态数组）
- **forwardChain**（单向链表）
- **chain**（双向链表）
- **blocksList**（块链表 / 分段链表）
- **hashMap**（哈希映射表）
- **treeMap**（树映射表）
- **hashSet**（哈希集合）
- **treeSet**（树集合）
- **JSet / JMap**（跳表集合 / 跳表映射）

### 容器接口
- **printable**（格式化输出接口）
- **comparable**（可比较接口）
- **cloneable**（深拷贝接口）
- **iterable**（可迭代接口）
- **arrayView**（数组视图）

### 算法库
- **allOf / anyOf / noneOf**（布尔判断算法）
- **find / count / equal**（非修改算法）
- **fill / swap / forEach**（修改算法）
- **sort / stableSort / introSort**（排序算法）

### 容器适配器
- **stack**（栈）
- **queue**（队列）
- **deque**（双端队列）
- **prique**（优先队列）

### 通用类型封装
- **alternative**（单类型可选值）

### 算法适配器
- **iterator / iterAdaptor**（迭代器 / 迭代器适配器）
- **transform / transformStream**（变换器）
- **filter / filterStream**（过滤器）
- **comparator**（比较器）

### 异常安全
- **error**（通用错误）
- **outOfBoundError**（越界错误）
- **unSupportedMethodError**（不支持的方法）
- **allocateError**（内存分配失败）
- **staticError**（编译期错误）

### 编译期工具
- **couple**（二元组）
- **tuple**（多元组）

### 内存管理
- **ownerPtr**（唯一所有权指针）
- **strongPtr**（强智能指针）
- **weakPtr**（弱智能指针）
- **allocatorBase / allocator**（通用分配器）
- **objPoolAllocator**（对象池分配器）
- **deleterBase / deleter**（删除器）
- **singleton**（单例模式）

</details>


<details>
<summary><strong>Vibrant</strong>（异步模块 ：多线程 / 协程 / IO 等）</summary>


### 线程
- **threadBase**（线程基类）
- **pThread**（POSIX 线程包装）
- **thread**（通用线程）

### 临界区管理
- **mutexBase / pMutex**（互斥量）
- **lockGuard / uniqueLock / multiLock**（作用域锁 / 独占锁 / 多重锁）
- **semaphore / semaphoreGuard**（信号量）

### 线程同步
- **conditionBase / pCondition**（条件变量）
- **syncPoint**（线程同步点）

### 原子操作
- **atomic**（原子变量）

### 跨线程生产 / 消费
- **async::promise**（承诺值）
- **async::futureBase / future / sharedFuture**（未来值 / 共享未来值）

### 线程安全容器
- **lockedQueue**（互斥锁队列）
- **lockedPrique**（互斥锁优先队列）

### 时间表示
- **time::duration**（时间段）
- **time::point**（时间点）
- **time::UTCTime**（UTC 时间）

### 任务调度
- **taskBase / task**（任务 / 可调度任务）
- **taskDelegator**（任务委派器 / 线程池）

### 协程
- **coroutine::generator**（协程生成器）
- **coroutine::task**（协程异步任务）
- **executor / syncExecutor / threadPoolExecutor**（协程执行器 / 同步事件循环执行器 / 线程池执行器）
</details>


<details>
<summary><strong>matrix</strong>（计划实现：张量 / 线性代数）</summary>

- 张量结构（Tensor）
- 基本线性代数运算（矩阵、向量等）

</details>


<details>
<summary><strong>graph</strong>（计划实现：图结构 / 图算法）</summary>

- 图结构（邻接表、邻接矩阵）
- 图算法（DFS/BFS、最短路、最小生成树等）

</details>

