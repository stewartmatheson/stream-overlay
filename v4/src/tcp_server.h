#pragma once

#include <cstdint>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

class TcpServer {
public:
    using CommandCallback = std::function<void(std::string)>;

    bool start(uint16_t port = 7777);
    void stop();

    // Drain all queued commands, calling cb for each.
    void poll(const CommandCallback& cb);

private:
    void accept_loop();
    void handle_client(uintptr_t sock);

    uintptr_t listen_sock_ = ~uintptr_t(0);
    std::thread thread_;
    bool running_ = false;

    std::mutex mtx_;
    std::queue<std::string> queue_;
};
