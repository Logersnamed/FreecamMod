find_package(Git QUIET)

if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --tags --always --dirty
        WORKING_DIRECTORY ${SRC_DIR}
        OUTPUT_VARIABLE MOD_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --short HEAD
        WORKING_DIRECTORY ${SRC_DIR}
        OUTPUT_VARIABLE MOD_GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
endif()

if(NOT MOD_VERSION)
    set(MOD_VERSION "unknown")
endif()
if(NOT MOD_GIT_HASH)
    set(MOD_GIT_HASH "unknown")
endif()

configure_file(
    ${SRC_DIR}/cmake/build_info.h.in
    ${DST_DIR}/build_info.h
    @ONLY
)