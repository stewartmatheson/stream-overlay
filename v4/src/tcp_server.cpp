#include "tcp_server.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

bool TcpServer::start(uint16_t port) {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return false;

    listen_sock_ = static_cast<uintptr_t>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listen_sock_ == static_cast<uintptr_t>(INVALID_SOCKET)) return false;

    // Allow quick restart
    int opt = 1;
    setsockopt(static_cast<SOCKET>(listen_sock_), SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = htons(port);

    if (bind(static_cast<SOCKET>(listen_sock_), reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR)
        return false;
    if (listen(static_cast<SOCKET>(listen_sock_), SOMAXCONN) == SOCKET_ERROR)
        return false;

    running_ = true;
    thread_ = std::thread(&TcpServer::accept_loop, this);
    return true;
}

void TcpServer::stop() {
    running_ = false;
    if (listen_sock_ != static_cast<uintptr_t>(INVALID_SOCKET)) {
        closesocket(static_cast<SOCKET>(listen_sock_));
        listen_sock_ = static_cast<uintptr_t>(INVALID_SOCKET);
    }
    if (thread_.joinable()) thread_.join();
    WSACleanup();
}

void TcpServer::poll(const CommandCallback& cb) {
    std::queue<std::string> local;
    {
        std::lock_guard lk(mtx_);
        std::swap(local, queue_);
    }
    while (!local.empty()) {
        cb(std::move(local.front()));
        local.pop();
    }
}

void TcpServer::accept_loop() {
    while (running_) {
        SOCKET client = accept(static_cast<SOCKET>(listen_sock_), nullptr, nullptr);
        if (client == INVALID_SOCKET) break;
        std::thread(&TcpServer::handle_client, this, static_cast<uintptr_t>(client)).detach();
    }
}

void TcpServer::handle_client(uintptr_t sock) {
    SOCKET s = static_cast<SOCKET>(sock);
    std::string buf;
    char tmp[512];

    while (true) {
        int n = recv(s, tmp, sizeof(tmp), 0);
        if (n <= 0) break;
        buf.append(tmp, n);

        // Process complete lines
        size_t pos;
        while ((pos = buf.find('\n')) != std::string::npos) {
            std::string line = buf.substr(0, pos);
            buf.erase(0, pos + 1);

            // Trim \r
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty()) continue;

            std::lock_guard lk(mtx_);
            queue_.push(std::move(line));
        }
    }

    closesocket(s);
}
