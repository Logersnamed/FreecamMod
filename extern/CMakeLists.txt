# Add libraries with CMakeLists.txt
add_subdirectory(MinHook)
add_subdirectory(mINI)
add_subdirectory(ModUtils)
add_subdirectory(MinSpeedhack)

# Link
add_library(extern_all INTERFACE)

target_link_libraries(extern_all
    INTERFACE
        minhook
        mINI
        ModUtils
        minspeedhack
)

add_library(extern::all ALIAS extern_all)