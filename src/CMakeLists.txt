add_library(beast_router INTERFACE)

target_sources(beast_router INTERFACE Router.hpp Token.hpp)

target_include_directories(beast_router INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
