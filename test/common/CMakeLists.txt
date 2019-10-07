add_library(test-commons INTERFACE)

target_sources(test-commons INTERFACE
        http_server.hpp
        listener.hpp
        http_session.hpp)
target_link_libraries(test-commons INTERFACE beast_router pthread)
