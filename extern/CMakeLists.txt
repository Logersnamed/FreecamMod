# Add libraries with CMakeLists.txt
add_subdirectory(MinHook)
add_subdirectory(mINI)
add_subdirectory(UniversalWndProcHook)

# Link
add_library(extern_all INTERFACE)

target_link_libraries(extern_all
    INTERFACE
        minhook
        mINI
        universal_wndproc_hook
)

add_library(extern::all ALIAS extern_all)