add_executable(router-test test.cpp)

target_compile_definitions(router-test PUBLIC CATCH_CONFIG_MAIN CATCH_CONFIG_ENABLE_BENCHMARKING)
target_include_directories(router-test PUBLIC ..)
target_link_libraries(router-test PUBLIC beast_router test-commons Catch2::Catch2 boost_coroutine)

add_test(
    NAME
        router-test
    COMMAND
        router-test
)