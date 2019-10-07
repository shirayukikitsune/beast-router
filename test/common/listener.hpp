#pragma once

#include "http_session.hpp"

#include <boost/beast/core/error.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <iostream>
#include <memory>

template <class Body>
class Listener : public boost::asio::coroutine, public std::enable_shared_from_this<Listener<Body>> {
public:
    Listener(boost::asio::io_context &io_context,
            const boost::asio::ip::tcp::endpoint& endpoint,
            std::shared_ptr<TestHttpServer<Body>> server)
    : io_context(io_context)
    , acceptor(boost::asio::make_strand(io_context))
    , socket(boost::asio::make_strand(io_context))
    , server(std::move(server)) {
        acceptor.open(endpoint.protocol());
        acceptor.set_option(boost::asio::socket_base::reuse_address(true));
        acceptor.bind(endpoint);
        acceptor.listen(boost::asio::socket_base::max_listen_connections);
    }

    void run() {
        accept();
    }

    void stop() {
        acceptor.cancel();
        acceptor.close();
    }

private:
#include <boost/asio/yield.hpp>
    void accept(boost::beast::error_code ec = {}) {
        reenter(*this) {
            for (;;) {
                yield acceptor.async_accept(
                        socket,
                        [&] (boost::system::error_code ec) {
                            accept(ec);
                        });

                if (!ec) {
                    std::make_shared<Session<Body>>(server, std::move(socket))->run();
                }

                socket = boost::asio::ip::tcp::socket(boost::asio::make_strand(io_context));
            }
        }
    }
#include <boost/asio/unyield.hpp>

    boost::asio::io_context &io_context;
    boost::asio::ip::tcp::acceptor acceptor;
    boost::asio::ip::tcp::socket socket;
    std::shared_ptr<TestHttpServer<Body>> server;
};
