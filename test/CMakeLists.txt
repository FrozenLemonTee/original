# test/CMakeLists.txt

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /Zi")
else()
    if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -g")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -g")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
    endif()
endif()

# test cases
add_subdirectory(other)
add_subdirectory(unit_test)