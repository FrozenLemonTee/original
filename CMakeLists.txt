# CMakeLists.txt Main

cmake_minimum_required(VERSION 3.31)
project(original LANGUAGES CXX)
set(ORIGINAL_VERSION 0.1.2)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -pthread")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

include_directories(${CMAKE_SOURCE_DIR})
set(ORIGINAL_SRC_DIR ${CMAKE_SOURCE_DIR}/src)
file(GLOB ORIGINAL_HEADERS
        "${ORIGINAL_SRC_DIR}/original.h"
        "${ORIGINAL_SRC_DIR}/core/*.h"
        "${ORIGINAL_SRC_DIR}/vibrant/*.h"
)
add_library(original STATIC ${ORIGINAL_HEADERS} src/original.cpp)

target_include_directories(original PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/core>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/vibrant>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS original
        EXPORT originalTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES src/original.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(DIRECTORY src/core/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/core
        FILES_MATCHING PATTERN "*.h"
)

install(EXPORT originalTargets
        FILE originalTargets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/original
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/originalConfig.cmake.in"
        "${CMAKE_BINARY_DIR}/originalConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/original
)

write_basic_package_version_file(
        "${CMAKE_BINARY_DIR}/originalConfigVersion.cmake"
        VERSION ${ORIGINAL_VERSION}
        COMPATIBILITY SameMajorVersion
)

install(FILES
        "${CMAKE_BINARY_DIR}/originalConfig.cmake"
        "${CMAKE_BINARY_DIR}/originalConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/original
)

option(BUILD_TESTING "Build the testing directories" ON)

if (BUILD_TESTING)
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
    add_subdirectory(test/other)
    add_subdirectory(test/unit_test)
endif ()