add_library(beast_router)

target_sources(beast_router PUBLIC library.cpp)
target_sources(beast_router INTERFACE library.h Token.hpp)

target_include_directories(beast_router INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
