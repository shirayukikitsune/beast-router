#pragma once

#include "Token.hpp"

#include <boost/beast/http.hpp>
#include <string>

namespace kitsune::web {

    template <class Handler, class Body, class Fields = boost::beast::http::fields>
    class Router {
    public:
        explicit Router(boost::beast::http::verb method)
                : method(method) {}

        [[nodiscard]] boost::beast::http::verb getMethod() const {
            return method;
        }

        Handler* findHandler(const boost::beast::http::request<Body, Fields>& request) {
            return rootToken.findMatch(request.target().to_string());
        }

        void addHandler(const std::string& path, Handler handler) {
            rootToken.addSubToken(path, handler);
        }

    private:
        Token<Handler, char> rootToken;

        boost::beast::http::verb method;
    };

}
