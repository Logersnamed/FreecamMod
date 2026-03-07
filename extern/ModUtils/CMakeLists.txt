add_library(ModUtils INTERFACE)

target_include_directories(ModUtils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ModUtils::ModUtils ALIAS ModUtils)