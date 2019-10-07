#include <catch2/catch.hpp>
#include <Token.hpp>

SCENARIO("tokens can be created", "[Token]") {
    GIVEN("A Token for /test") {
        Token<std::string> token("/test", "success");

        REQUIRE(token.getMatcherString() == "/test");
        REQUIRE((*token.getHandler()) == "success");

        WHEN("matching against \"/test\"") {
            auto match = token.findMatch("/test");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match) == "success");
            }
        }

        WHEN("matching against \"/\"") {
            auto match = token.findMatch("/");

            THEN("an invalid match must be returned") {
                REQUIRE(match == nullptr);
            }
        }

        WHEN("matching against \"/test/fail\"") {
            auto match = token.findMatch("/test/fail");

            THEN("an invalid match must be returned") {
                REQUIRE(match == nullptr);
            }
        }
    }
}

SCENARIO("sub tokens can be created", "[Token]") {
    GIVEN("A Token for /test and for /test/sub") {
        Token<std::string> token("/test", "success");
        token.addSubToken("/test/sub", "sub success");

        REQUIRE(token.getMatcherString() == "/test");
        REQUIRE((*token.getHandler()) == "success");

        WHEN("matching against \"/test\"") {
            auto match = token.findMatch("/test");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match) == "success");
            }
        }

        WHEN("matching against \"/\"") {
            auto match = token.findMatch("/");

            THEN("an invalid match must be returned") {
                REQUIRE(match == nullptr);
            }
        }

        WHEN("matching against \"/test/fail\"") {
            auto match = token.findMatch("/test/fail");

            THEN("an invalid match must be returned") {
                REQUIRE(match == nullptr);
            }
        }

        WHEN("matching against \"/test/sub\"") {
            auto match = token.findMatch("/test/sub");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match) == "sub success");
            }
        }
    }
}

SCENARIO("token replacement when child tokens are inserted", "[Token]") {
    GIVEN("A token for \"/test\"") {
        Token<std::string> token("/test", "success");
        token.addSubToken("/team", "sub success");

        REQUIRE(token.getMatcherString() == "/te");
        REQUIRE(token.getHandler() == nullptr);

        WHEN("matching against \"/test\"") {
            auto match = token.findMatch("/test");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match) == "success");
            }
        }

        WHEN("matching against \"/team\"") {
            auto match = token.findMatch("/team");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match) == "sub success");
            }
        }
    }
}

TEST_CASE("Token Benchmark", "[benchmark,Token]") {
    BENCHMARK_ADVANCED("direct routing")(Catch::Benchmark::Chronometer meter) {
        Token<std::string> tokenizer("/benchmark", "benchmark");

        meter.measure([&tokenizer] {
            return tokenizer.findMatch("/benchmark");
        });
    };

    BENCHMARK_ADVANCED("sub routing")(Catch::Benchmark::Chronometer meter) {
        Token<std::string> tokenizer("/1/benchmark", "benchmark");
        tokenizer.addSubToken("/2/benchmark", "benchmark");
        tokenizer.addSubToken("/1/benchmark/1234", "benchmark");
        tokenizer.addSubToken("/1/benchmark/foo", "benchmark");

        meter.measure([&tokenizer] {
            return tokenizer.findMatch("/1/benchmark/1234");
        });
    };
}
