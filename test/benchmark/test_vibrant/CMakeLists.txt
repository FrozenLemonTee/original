# test/benchmark/test_vibrant/CMakeLists.txt

file(GLOB VIBRANT_BENCHMARKS CONFIGURE_DEPENDS "bench_*.cpp")

add_executable(vibrant_benchmark ${VIBRANT_BENCHMARKS})

if(MSVC)
    target_compile_options(vibrant_benchmark PRIVATE "/bigobj")
    target_compile_options(vibrant_benchmark PRIVATE "/Zc:inline")
endif()

target_link_libraries(vibrant_benchmark PRIVATE benchmark::benchmark)

target_link_libraries(vibrant_benchmark PRIVATE original)

if(WIN32)
    target_link_libraries(vibrant_benchmark PRIVATE ws2_32)
endif()

add_custom_target(
        run_vibrant_benchmark
        COMMAND $<TARGET_FILE:vibrant_benchmark>
        DEPENDS vibrant_benchmark
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
