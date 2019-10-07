#pragma once

#include <boost/asio/coroutine.hpp>
#include <boost/beast.hpp>
#include <boost/optional.hpp>
#include <iostream>

template <class Body> class TestHttpServer;

template <class Body>
class Session
        : public boost::asio::coroutine
        , public std::enable_shared_from_this<Session<Body>> {
public:
    Session(std::shared_ptr<TestHttpServer<Body>> server, boost::asio::ip::tcp::socket && socket)
            : stream(std::move(socket)), server(std::move(server)) { }

    void run() {
        do_read(false, {}, 0);
    }

private:
#include <boost/asio/yield.hpp>
    void do_read(bool close,
                 boost::beast::error_code ec,
                 std::size_t bytes_transferred) {
        reenter(*this) {
            for(;;) {
                parser.emplace();
                parser->body_limit(10'000);
                stream.expires_after(std::chrono::seconds(10));

                yield boost::beast::http::async_read(stream, buffer, parser->get(), boost::beast::bind_front_handler(&Session::do_read, this->shared_from_this(), false));

                if (ec == boost::beast::http::error::end_of_stream) {
                    break;
                }

                if (ec) {
                    std::cerr << "err: " << ec.message();
                    break;
                }

                response = std::make_shared<boost::beast::http::response<boost::beast::http::string_body>>
                        (std::move(server->requestHandler(std::move(parser->release()))));

                yield boost::beast::http::async_write(stream, *response, boost::beast::bind_front_handler(&Session::do_read, this->shared_from_this(), response->need_eof()));

                if (ec) {
                    std::cerr << "err: " << ec.message();
                    return;
                }

                if (close) {
                    break;
                }

                response.reset();
            }

            do_close();
        }
    }
#include <boost/asio/unyield.hpp>

    void do_close() {
        boost::beast::error_code ec;
        stream.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
    }

    std::shared_ptr<boost::beast::http::response<boost::beast::http::string_body>> response;

    boost::beast::tcp_stream stream;
    boost::beast::flat_buffer buffer;
    boost::optional<boost::beast::http::request_parser<Body>> parser;
    std::shared_ptr<TestHttpServer<Body>> server;
};
