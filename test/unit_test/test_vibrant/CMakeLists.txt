# test/unit_test/test_vibrant/CMakeLists.txt

enable_testing()

file(GLOB VIBRANT_TESTS "test_*.cpp")

add_executable(vibrant_test ${VIBRANT_TESTS})

if(MSVC)
    target_compile_options(vibrant_test PRIVATE "/bigobj")
    target_compile_options(vibrant_test PRIVATE "/Zc:inline")
endif()

target_link_libraries(vibrant_test PRIVATE GTest::gtest GTest::gmock GTest::gmock_main GTest::gtest_main)

target_link_libraries(vibrant_test PRIVATE original)

add_test(
        NAME VibrantTests
        COMMAND $<TARGET_FILE:vibrant_test>
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
