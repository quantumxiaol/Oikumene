#pragma once

#include <chrono>
#include <string>

namespace oikumene::http {

struct Response {
    int status_code = 0;
    std::string body;
    std::string error;
    long latency_ms = 0;
};

[[nodiscard]] Response Get(const std::string& host,
                           int port,
                           const std::string& path,
                           std::chrono::milliseconds timeout);

[[nodiscard]] Response PostJson(const std::string& host,
                                int port,
                                const std::string& path,
                                const std::string& body,
                                std::chrono::milliseconds timeout);

}  // namespace oikumene::http
