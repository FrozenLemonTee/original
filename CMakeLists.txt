# CMakeLists.txt Main

cmake_minimum_required(VERSION 3.31)
if(CMAKE_GENERATOR MATCHES "Visual Studio" OR CMAKE_GENERATOR MATCHES "Ninja Multi-Config")
    set(CMAKE_GENERATOR_PLATFORM x64 CACHE STRING "Platform" FORCE)
endif()
project(original LANGUAGES CXX)
set(ORIGINAL_VERSION 0.1.5)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (default Release)" FORCE)
endif()

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
        $<INSTALL_INTERFACE:include/core>
        $<INSTALL_INTERFACE:include/vibrant>
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
install(DIRECTORY src/vibrant/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vibrant
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

add_subdirectory(debug)

option(BUILD_TESTING "Build the testing directories" ON)

if (BUILD_TESTING)
    include(CTest)
    enable_testing()
    add_subdirectory(test)
endif ()