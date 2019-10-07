#include "common/http_server.hpp"

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = net::ip::tcp;
namespace http = boost::beast::http;
using namespace http;

int main(int argc, char** argv) {
    auto address = boost::asio::ip::make_address("::1");
    auto threads = argc > 1 ? std::strtoul(argv[1], nullptr, 0) : std::thread::hardware_concurrency();
    auto httpServer = std::make_shared<TestHttpServer<string_body>>(threads);

    httpServer->start(address, 48159, threads - 1);
    httpServer->addRoute(boost::beast::http::verb::get, "/test", [](auto request) {
        response<string_body> res{ status::ok, request.version() };
        res.set(field::server, BOOST_BEAST_VERSION_STRING);
        res.set(field::content_type, "text/plain");
        res.keep_alive(request.keep_alive());
        res.body() = std::string(256, 'a');
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

    httpServer->getIoContext().run();

    for (auto & thread : httpServer->getIoThreads()) {
        thread.join();
    }
}
