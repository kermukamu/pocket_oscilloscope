#include "udp_receiver.hpp"

#include <sstream>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

UdpReceiver::UdpReceiver(int port) : port_(port) {}

UdpReceiver::~UdpReceiver() {
    stop();
}

void UdpReceiver::start() {
    running_ = true;
    worker_ = std::thread(&UdpReceiver::run, this);
}

void UdpReceiver::stop() {
    running_ = false;
    if (worker_.joinable())
        worker_.join();
}

std::vector<int> UdpReceiver::takeSamples() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<int> out;
    out.swap(pending_);
    return out;
}

void UdpReceiver::run() {
#ifdef _WIN32
    WSADATA wsaData{};
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif

    int sockfd = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons((unsigned short)port_);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(sockfd);
        WSACleanup();
#else
        close(sockfd);
#endif
        return;
    }

    char buffer[4096];

    while (running_) {
        sockaddr_in src{};
#ifdef _WIN32
        int srclen = sizeof(src);
        int len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&src, &srclen);
#else
        socklen_t srclen = sizeof(src);
        int len = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&src, &srclen);
#endif
        if (len <= 0) continue;

        buffer[len] = '\0';
        std::stringstream ss(buffer);
        std::string token;
        bool first = true;
        std::vector<int> local;

        while (std::getline(ss, token, ',')) {
            if (first) {
                first = false; // skip loop_count
                continue;
            }
            try {
                local.push_back(std::stoi(token));
            } catch (...) {
            }
        }

        if (!local.empty()) {
            std::lock_guard<std::mutex> lock(mutex_);
            pending_.insert(pending_.end(), local.begin(), local.end());
        }
    }

#ifdef _WIN32
    closesocket(sockfd);
    WSACleanup();
#else
    close(sockfd);
#endif
}