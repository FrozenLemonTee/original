## 快速导引 — 给 AI 干活时要知道的要点

下面是为在本仓库中自动化修改、修复或新增特性时对 AI 代理最有用的、可检索的事实与可执行示例。

1. 项目概况（大局）
   - 语言/标准：C++（要求 C++23）。主库名为 `original`，以静态库方式构建（见 `CMakeLists.txt` 中的 add_library(original STATIC ...)）。
   - 代码组织：源文件在 `src/`；公开头文件为 `src/original.h` 以及 `src/core/`、`src/vibrant/` 下的 `.h` 文件。
   - 模块划分：`core`（容器/算法/内存）和 `vibrant`（异步/线程/协程）。测试代码位于 `test/`（例如 `test/unit_test/test_core`）。文档在 `docs/`。

2. 构建 & 测试（可复制的命令示例）
   - Visual Studio (multi-config):
     ```powershell
     cmake -S . -B cmake-build-debug-visualstudio2022 -G "Visual Studio 17 2022" -A x64
     cmake --build cmake-build-debug-visualstudio2022 --config Debug
     cmake --install cmake-build-debug-visualstudio2022 --config Debug
     ```
   - Ninja Multi-Config (also supported):
     ```powershell
     cmake -S . -B cmake-build-debug-ninja -G "Ninja Multi-Config" -A x64
     cmake --build cmake-build-debug-ninja --config Debug
     ```
   - Ninja single-config (MinGW / Clang):
     ```powershell
     cmake -S . -B cmake-build-debug-mingw-gcc -G "Ninja"
     cmake --build cmake-build-debug-mingw-gcc -j
     ```
   - 运行测试（在构建目录中或指定构建目录）：
     ```powershell
     ctest --test-dir <build-dir> -C Debug
     ```
   - 本仓库还提供一个简便的本地安装脚本：`cmake -P install.cmake`（见 README 的“方法二”）。

3. CMake / 包与公开 API 注意点（项目特有约定）
   - 项目会导出 CMake package 配置（`cmake/originalConfig.cmake.in` -> 安装时生成 `originalConfig.cmake`），因此改变安装头文件布局或目标名时需要同步更新 package 模板和 `install()` 调用。
   - 编译器/版本门槛在 `CMakeLists.txt` 中强制（GCC>=13, Clang>=20, MSVC>=1944）。不要在修复/增强中假设可用较旧编译器特性。
   - 线程支持通过 `find_package(Threads)` 并将 `Threads::Threads` 链接到 `original`，若新增目标需要线程依赖请复用该模式。

4. 调试与开发辅助
   - `debug/` 下有 `gdbinit.py` 和 `printers/`，提供 GDB pretty-printers / 调试辅助；当添加新的类型（尤其容器/打印接口），更新这些打印器有助于调试体验。
   - 流程：使用合适的 CMake 生成器生成调试构建，然后在 IDE（Visual Studio）或 GDB/LLDB（在 MinGW/Clang 环境）中加载调试符号。

5. 代码/接口约定（可从代码中直接发现并应遵守）
   - 公共 API 位于 `src/original.h` 以及 `src/core/*.h`, `src/vibrant/*.h`，修改这些文件会影响安装包的 include 结构（`install()` 调用依赖文件路径）。
   - 库目标使用 `target_include_directories(... PUBLIC $<BUILD_INTERFACE:...> $<INSTALL_INTERFACE:...>)`，新增源码时请按相同方式公开 include 目录。

6. 测试与 CI 注意事项
   - `BUILD_TESTING` CMake 选项默认开启（ON）；修改测试相关文件时，CI 会期望测试可以被构建并执行（使用 CTest）。
   - 单元测试位于 `test/unit_test/`，写新增测试时请把测试加入相应的 CMake 子目录（参考现有 `test/CMakeLists.txt`）。

7. 给 AI 代理的具体建议（任务执行时的工作流）
   - 变更公共头文件前，找到并更新 `CMakeLists.txt` 中的 `install(FILES ...)` / `install(DIRECTORY ...)` 相关条目，以及 `cmake/originalConfig.cmake.in`（若导出接口发生变化）。
   - 修复构建或编译错误时，先在本地用与 CI 相同的生成器复现（仓库包含 `cmake-build-debug-*`，可参考其中一种），确保通过 `cmake --build` + `ctest` 验证。
   - 当修改容器或 printable 接口时，记得同时检查 `debug/printers/`（pretty-printers）和 `docs/`（若需文档更新）。

如果你想让我把某一部分展开为更详细的检查清单（例如“如何添加新容器并让其通过 CI 和安装测试”），告诉我需要哪一项，我会把步骤和示例 CMake 片段补上。
