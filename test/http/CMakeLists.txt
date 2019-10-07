add_executable(http-server server.cpp)

target_include_directories(http-server PUBLIC ..)
target_link_libraries(http-server PUBLIC beast_router test-commons boost_coroutine)
