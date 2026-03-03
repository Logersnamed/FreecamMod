# Add libraries with CMakeLists.txt
add_subdirectory(MinHook)
add_subdirectory(mINI)

# Link
add_library(extern_all INTERFACE)

target_link_libraries(extern_all
    INTERFACE
        minhook
        mINI
)

add_library(extern::all ALIAS extern_all)