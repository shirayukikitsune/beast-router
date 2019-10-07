add_library(test-commons)

target_sources(test-commons PUBLIC
        http_server.hpp
        listener.hpp
        http_session.hpp)
target_link_libraries(test-commons PUBLIC beast_router INTERFACE pthread)
