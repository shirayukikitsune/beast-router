#include "common/http_server.hpp"

#include <boost/asio/spawn.hpp>
#include <catch2/catch.hpp>
#include <cstring>
#include <sstream>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace http = boost::beast::http;
using namespace http;

auto makeRequest(beast::tcp_stream &stream, verb method, const std::string& target) {
    request<empty_body> req{method, target, 11U};
    req.set(http::field::host, "localhost");
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    write(stream, req);
    beast::flat_buffer buffer;
    response<string_body> res;
    read(stream, buffer, res);

    return res;
}

SCENARIO("A route should be matched", "[Router]") {
    GIVEN("A router for GET /test") {
        auto address = boost::asio::ip::make_address("::1");
        auto httpServer = std::make_shared<TestHttpServer<string_body>>();
        boost::asio::io_context ioc;

        httpServer->start(address, 48159);
        httpServer->addRoute(boost::beast::http::verb::get, "/test", [](auto request) {
            response<string_body> res{ status::ok, request.version() };
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, "text/plain");
            res.keep_alive(request.keep_alive());
            res.body() = "success";
            res.prepare_payload();
            return res;
        });
        httpServer->setNotFoundHandler([](auto && request) {
            response<string_body> res{ status::not_found, request.version() };
            res.set(field::server, BOOST_BEAST_VERSION_STRING);
            res.set(field::content_type, "text/plain");
            res.keep_alive(request.keep_alive());
            res.body() = "not found";
            res.prepare_payload();
            return res;
        });

        tcp::resolver resolver(ioc);
        auto const results = resolver.resolve("localhost", "48159");
        beast::tcp_stream stream(ioc);
        stream.connect(results);

        WHEN("performing a GET /test") {
            auto res = makeRequest(stream, verb::get, "/test");

            THEN("it should have the expected body") {
                REQUIRE(res.result() == status::ok);
                REQUIRE(res.body() == "success");
            }
        }

        WHEN("performing a HEAD /fail") {
            auto res = makeRequest(stream, verb::head, "/fail");

            THEN("it should call the not found handler") {
                REQUIRE(res.result() == status::not_found);
                REQUIRE(res.body() == "not found");
            }
        }

        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        if(ec && ec != beast::errc::not_connected) {
            throw beast::system_error{ec};
        }

        httpServer->stop();
    }
}
