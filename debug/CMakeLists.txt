# debug/CMakeLists.txt

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "MinGW")
        message(STATUS "[GDB CONFIG] Detected GNU/MinGW toolchain, setting up GDB auto-load permissions")

        if(WIN32)
            file(TO_CMAKE_PATH "$ENV{USERPROFILE}" USER_HOME)
        else()
            file(TO_CMAKE_PATH "$ENV{HOME}" USER_HOME)
        endif()

        set(GDB_GLOBAL_INIT "${USER_HOME}/.config/gdb/gdbinit")
        if(NOT EXISTS "${GDB_GLOBAL_INIT}")
            set(GDB_GLOBAL_INIT "${USER_HOME}/.gdbinit")
        endif()

        set(PROJECT_GDBINIT "${CMAKE_SOURCE_DIR}/.gdbinit")

        set(GDB_CONFIG_LINES
                "set auto-load local-gdbinit on
add-auto-load-safe-path ${PROJECT_GDBINIT}\n"
        )

        get_filename_component(GDB_CONFIG_DIR "${GDB_GLOBAL_INIT}" DIRECTORY)
        file(MAKE_DIRECTORY "${GDB_CONFIG_DIR}")

        if(EXISTS "${GDB_GLOBAL_INIT}")
            file(READ "${GDB_GLOBAL_INIT}" EXISTING_CONTENT)
            string(FIND "${EXISTING_CONTENT}" "${PROJECT_GDBINIT}" ALREADY_ADDED)
        else()
            set(ALREADY_ADDED -1)
        endif()

        if(ALREADY_ADDED EQUAL -1)
            file(APPEND "${GDB_GLOBAL_INIT}" "\n# Added by Original CMake auto-config\n${GDB_CONFIG_LINES}")
            message(STATUS "[GDB CONFIG] Added safe-path for ${PROJECT_GDBINIT} in ${GDB_GLOBAL_INIT}")
        else()
            message(STATUS "[GDB CONFIG] Safe-path already configured in ${GDB_GLOBAL_INIT}")
        endif()
    endif()
endif()
