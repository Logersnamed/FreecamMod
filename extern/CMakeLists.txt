# Add libraries with CMakeLists.txt
add_subdirectory(MinHook)

# Add Header-only libraries
# add_library(MinHook INTERFACE)
# target_include_directories(MinHook INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/MinHook)

# Link
add_library(extern_all INTERFACE)

target_link_libraries(extern_all
    INTERFACE
        minhook
)

add_library(extern::all ALIAS extern_all)