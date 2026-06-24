#include "oikumene/network/http_client.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cstring>
#include <sstream>
#include <string>

#if defined(_WIN32)
#define NOMINMAX
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace oikumene::http {
namespace {

#if defined(_WIN32)
using SocketHandle = SOCKET;
constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;

void CloseSocket(SocketHandle socket) {
    closesocket(socket);
}

bool EnsureSocketRuntime(Response& response) {
    static bool initialized = false;
    if (initialized) {
        return true;
    }
    WSADATA data{};
    if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
        response.error = "WSAStartup failed";
        return false;
    }
    initialized = true;
    return true;
}
#else
using SocketHandle = int;
constexpr SocketHandle kInvalidSocket = -1;

void CloseSocket(SocketHandle socket) {
    close(socket);
}

bool EnsureSocketRuntime(Response&) {
    return true;
}
#endif

bool SetTimeouts(SocketHandle socket, std::chrono::milliseconds timeout, Response& response) {
#if defined(_WIN32)
    const DWORD timeout_ms = static_cast<DWORD>(timeout.count());
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms)) !=
            0 ||
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms)) !=
            0) {
        response.error = "setsockopt timeout failed";
        return false;
    }
#else
    timeval tv{};
    tv.tv_sec = static_cast<long>(timeout.count() / 1000);
    tv.tv_usec = static_cast<int>((timeout.count() % 1000) * 1000);
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) != 0 ||
        setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) != 0) {
        response.error = "setsockopt timeout failed";
        return false;
    }
#endif
    return true;
}

std::string StripHttpHeaders(const std::string& raw) {
    const auto separator = raw.find("\r\n\r\n");
    if (separator == std::string::npos) {
        return {};
    }
    return raw.substr(separator + 4);
}

int ParseStatusCode(const std::string& raw) {
    std::istringstream stream(raw);
    std::string http_version;
    int status_code = 0;
    stream >> http_version >> status_code;
    return status_code;
}

Response SendRequest(const std::string& host, int port, const std::string& raw_request,
                     std::chrono::milliseconds timeout) {
    const auto start = std::chrono::steady_clock::now();

    Response response;
    if (!EnsureSocketRuntime(response)) {
        return response;
    }

    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfo* result = nullptr;
    const auto port_string = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_string.c_str(), &hints, &result) != 0) {
        response.error = "getaddrinfo failed";
        return response;
    }

    SocketHandle socket_handle = kInvalidSocket;
    for (auto* ptr = result; ptr != nullptr; ptr = ptr->ai_next) {
        socket_handle = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (socket_handle == kInvalidSocket) {
            continue;
        }

        if (!SetTimeouts(socket_handle, timeout, response)) {
            CloseSocket(socket_handle);
            socket_handle = kInvalidSocket;
            continue;
        }

        if (connect(socket_handle, ptr->ai_addr, static_cast<int>(ptr->ai_addrlen)) == 0) {
            break;
        }

        CloseSocket(socket_handle);
        socket_handle = kInvalidSocket;
    }

    freeaddrinfo(result);

    if (socket_handle == kInvalidSocket) {
        if (response.error.empty()) {
            response.error = "connect failed";
        }
        return response;
    }

    const char* request_data = raw_request.data();
    auto bytes_remaining = raw_request.size();
    while (bytes_remaining > 0) {
#if defined(_WIN32)
        const auto sent = send(socket_handle, request_data, static_cast<int>(bytes_remaining), 0);
#else
        const auto sent = send(socket_handle, request_data, bytes_remaining, 0);
#endif
        if (sent <= 0) {
            response.error = "send failed";
            CloseSocket(socket_handle);
            return response;
        }
        request_data += sent;
        bytes_remaining -= static_cast<std::size_t>(sent);
    }

    std::string raw_response;
    std::array<char, 4096> buffer{};
    while (true) {
#if defined(_WIN32)
        const auto received = recv(socket_handle, buffer.data(), static_cast<int>(buffer.size()), 0);
#else
        const auto received = recv(socket_handle, buffer.data(), buffer.size(), 0);
#endif
        if (received > 0) {
            raw_response.append(buffer.data(), static_cast<std::size_t>(received));
            continue;
        }
        break;
    }

    CloseSocket(socket_handle);

    response.status_code = ParseStatusCode(raw_response);
    response.body = StripHttpHeaders(raw_response);
    response.latency_ms = static_cast<long>(
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count());

    if (raw_response.empty()) {
        response.error = "empty response";
    }

    return response;
}

} // namespace

Response Get(const std::string& host, int port, const std::string& path, std::chrono::milliseconds timeout) {
    std::ostringstream request;
    request << "GET " << path << " HTTP/1.1\r\n"
            << "Host: " << host << ":" << port << "\r\n"
            << "Accept: application/json\r\n"
            << "Connection: close\r\n\r\n";
    return SendRequest(host, port, request.str(), timeout);
}

Response PostJson(const std::string& host, int port, const std::string& path, const std::string& body,
                  std::chrono::milliseconds timeout) {
    std::ostringstream request;
    request << "POST " << path << " HTTP/1.1\r\n"
            << "Host: " << host << ":" << port << "\r\n"
            << "Accept: application/json\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << body;
    return SendRequest(host, port, request.str(), timeout);
}

} // namespace oikumene::http
