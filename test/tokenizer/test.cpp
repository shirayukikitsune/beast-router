#include <catch2/catch.hpp>
#include <Token.hpp>

using kitsune::web::Token;

SCENARIO("tokens can be created", "[Token]") {
    GIVEN("A Token for /test") {
        auto handler = [] { return std::string("success"); };
        Token<std::string()> token("/test", handler);

        REQUIRE(token.getMatcherString() == "/test");
        REQUIRE((*token.getHandler())() == "success");

        WHEN("matching against \"/test\"") {
            auto match = token.findMatch("/test");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match)() == "success");
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

SCENARIO("child tokens can be created", "[Token]") {
    GIVEN("A Token for /test and for /test/sub") {
        auto handler = [] { return std::string("success"); };
        auto childHandler = [] { return std::string("sub success"); };
        Token<std::string()> token{"/test", handler};
        token.addSubToken("/test/sub", childHandler);

        REQUIRE(token.getMatcherString() == "/test");
        REQUIRE((*token.getHandler())() == "success");

        WHEN("matching against \"/test\"") {
            auto match = token.findMatch("/test");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match)() == "success");
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
                REQUIRE((*match)() == "sub success");
            }
        }
    }
}

SCENARIO("token replacement when child tokens are inserted", "[Token]") {
    GIVEN("A token for \"/test\"") {
        auto handler = [] { return std::string("success"); };
        auto childHandler = [] { return std::string("sub success"); };
        Token<std::string()> token("/test", handler);
        token.addSubToken("/team", childHandler);

        REQUIRE(token.getMatcherString() == "/te");
        REQUIRE(token.getHandler() == nullptr);

        WHEN("matching against \"/test\"") {
            auto match = token.findMatch("/test");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match)() == "success");
            }
        }

        WHEN("matching against \"/team\"") {
            auto match = token.findMatch("/team");

            THEN("a valid match must be returned") {
                REQUIRE(match != nullptr);
                REQUIRE((*match)() == "sub success");
            }
        }
    }
}

TEST_CASE("Token Benchmark", "[benchmark,Token]") {
    std::string handler = "benchmark";

    BENCHMARK_ADVANCED("direct routing")(Catch::Benchmark::Chronometer meter) {
        Token<std::string*> tokenizer("/benchmark", &handler);

        meter.measure([&tokenizer] {
            return tokenizer.findMatch("/benchmark");
        });
    };

    BENCHMARK_ADVANCED("sub routing")(Catch::Benchmark::Chronometer meter) {
        Token<std::string> tokenizer("/1/benchmark", handler);
        tokenizer.addSubToken("/2/benchmark", handler);
        tokenizer.addSubToken("/1/benchmark/1234", handler);
        tokenizer.addSubToken("/1/benchmark/foo", handler);

        meter.measure([&tokenizer] {
            return tokenizer.findMatch("/1/benchmark/1234");
        });
    };
}
