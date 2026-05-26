# test/benchmark/CMakeLists.txt

if(NOT TARGET benchmark::benchmark)
    include(FetchContent)

    set(BENCHMARK_VERSION_STABLE v1.8.5 CACHE STRING "Stable Google Benchmark version")
    set(BENCHMARK_VERSION_LATEST v1.9.4 CACHE STRING "Latest Google Benchmark version")
    set(BENCHMARK_VERSION ${BENCHMARK_VERSION_LATEST} CACHE STRING "Selected Google Benchmark version")

    if(NOT DEFINED BENCHMARK_REPO)
        find_package(Git QUIET)
        if(NOT GIT_FOUND)
            message(FATAL_ERROR "Git is required to fetch Google Benchmark!")
        endif()

        set(REPO_PRIORITY_LIST
                "git@github.com:google/benchmark.git"
                "https://github.com/google/benchmark.git"
                "https://gitee.com/mirrors/google-benchmark.git"
        )

        foreach(repo IN LISTS REPO_PRIORITY_LIST)
            execute_process(
                    COMMAND ${GIT_EXECUTABLE} ls-remote ${repo} -q
                    RESULT_VARIABLE result
                    OUTPUT_QUIET
                    ERROR_QUIET
                    TIMEOUT 5
            )
            if(result EQUAL 0)
                set(BENCHMARK_REPO ${repo} CACHE STRING "Selected Google Benchmark repository")
                set(BENCHMARK_VERSION ${BENCHMARK_VERSION_LATEST} CACHE STRING "Selected Google Benchmark version")
                message(STATUS "Selected Google Benchmark repository: ${repo}")
                break()
            endif()
        endforeach()

        if(NOT BENCHMARK_REPO)
            message(FATAL_ERROR "All Google Benchmark repositories are unavailable!")
        endif()
    endif()

    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "Disable Google Benchmark tests" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable Google Benchmark GTest tests" FORCE)
    set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "Disable Google Benchmark docs install" FORCE)
    set(BENCHMARK_INSTALL_TOOLS OFF CACHE BOOL "Disable Google Benchmark tools install" FORCE)

    FetchContent_Declare(
            GBenchmark
            GIT_REPOSITORY ${BENCHMARK_REPO}
            GIT_TAG        ${BENCHMARK_VERSION}
            GIT_SHALLOW    TRUE
    )

    FetchContent_MakeAvailable(GBenchmark)
endif()

add_subdirectory(test_vibrant)
