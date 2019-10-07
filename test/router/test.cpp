#include "common/http_server.hpp"

#include <boost/asio/spawn.hpp>
// #include <catch2/catch.hpp>
#include <sstream>

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace http = boost::beast::http;
using namespace http;

void do_session(std::string const& host,
        std::string const& port,
        std::string const& target,
        int version,
        net::io_context& ioc,
        net::yield_context yield) {
    beast::error_code ec;

    // These objects perform our I/O
    tcp::resolver resolver(ioc);
    beast::tcp_stream stream(ioc);

    // Look up the domain name
    auto const results = resolver.async_resolve(host, port, yield[ec]);
    if(ec)
        return;

    // Set the timeout.
    stream.expires_after(std::chrono::seconds(30));

    // Make the connection on the IP address we get from a lookup
    stream.async_connect(results, yield[ec]);
    if(ec)
        return;

    // Set up an HTTP GET request message
    http::request<http::string_body> req{http::verb::get, target, version};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

    // Set the timeout.
    stream.expires_after(std::chrono::seconds(30));

    // Send the HTTP request to the remote host
    http::async_write(stream, req, yield[ec]);
    if(ec)
        return;

    // This buffer is used for reading and must be persisted
    beast::flat_buffer b;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::async_read(stream, b, res, yield[ec]);
    if(ec)
        return;

    // Write the message to standard out
    std::cout << res << std::endl;

    // Gracefully close the socket
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if(ec && ec != beast::errc::not_connected)
        return;

    // If we get here then the connection is closed gracefully

}

int main(int argc, char** argv) {
    auto address = boost::asio::ip::make_address("::1");
    auto httpServer = std::make_shared<TestHttpServer<string_body>>();
    httpServer->start(address, 48159);

    boost::asio::io_context ioc;

    httpServer->addRoute(boost::beast::http::verb::get, "/test", [](auto request) {
        response<string_body> res{ status::ok, request.version() };
        res.set(field::server, BOOST_BEAST_VERSION_STRING);
        res.set(field::content_type, "text/plain");
        res.keep_alive(request.keep_alive());
        res.body() = "success";
        res.prepare_payload();
        return res;
    });

    httpServer->setNotFoundHandler([](auto request) {
        response<string_body> res{ status::not_found, request.version() };
        res.set(field::server, BOOST_BEAST_VERSION_STRING);
        res.set(field::content_type, "text/plain");
        res.keep_alive(request.keep_alive());
        res.body() = "not found";
        res.prepare_payload();
        return res;
    });

    boost::asio::spawn(ioc, std::bind(&do_session,
                                      std::string("localhost"),
                                      std::string("48159"),
                                      std::string("/test"),
                                      11,
                                      std::ref(ioc),
                                      std::placeholders::_1));

    ioc.run();

}

#if 0
SCENARIO("A route should be matched", "[Router]") {
    GIVEN("A router for GET /test") {
        WHEN("performing a GET /test") {
            auto address = boost::asio::ip::make_address("::1");
            auto httpServer = std::make_shared<TestHttpServer<string_body>>();
            httpServer->start(address, 48159);

            boost::asio::io_context ioc;

            httpServer->addRoute(boost::beast::http::verb::get, "/test", [](auto request) {
                response<string_body> res{ status::ok, request.version() };
                res.set(field::server, BOOST_BEAST_VERSION_STRING);
                res.set(field::content_type, "text/plain");
                res.keep_alive(request.keep_alive());
                res.body() = "success";
                res.prepare_payload();
                return res;
            });

            httpServer->setNotFoundHandler([](auto request) {
                response<string_body> res{ status::not_found, request.version() };
                res.set(field::server, BOOST_BEAST_VERSION_STRING);
                res.set(field::content_type, "text/plain");
                res.keep_alive(request.keep_alive());
                res.body() = "not found";
                res.prepare_payload();
                return res;
            });

            boost::asio::spawn(ioc, std::bind(&do_session,
                                              std::string("localhost"),
                                              std::string("48159"),
                                              std::string("/test"),
                                              11,
                                              std::ref(ioc),
                                              std::placeholders::_1));
            tcp::resolver resolver(ioc);
            beast::tcp_stream stream(ioc);
            auto const results = resolver.resolve("localhost", "48159");
            stream.connect(results);

            request<empty_body> req{verb::get, "/test", 11U};

            ioc.run();
        }
    }
}
#endif