add_executable(tokenizer-test test.cpp)

target_compile_definitions(tokenizer-test PUBLIC CATCH_CONFIG_MAIN CATCH_CONFIG_ENABLE_BENCHMARKING)
target_include_directories(tokenizer-test PUBLIC ..)
target_link_libraries(tokenizer-test PUBLIC beast_router test-commons Catch2::Catch2)

add_test(
    NAME
        tokenizer-test
    COMMAND
        tokenizer-test
)