#pragma once

#include <vector>
#include <thread>
#include <atomic>
#include <mutex>

class UdpReceiver {
public:
    explicit UdpReceiver(int port);
    ~UdpReceiver();

    void start();
    void stop();

    std::vector<int> takeSamples();

private:
    void run();

    int port_;
    std::atomic<bool> running_{false};
    std::thread worker_;

    std::mutex mutex_;
    std::vector<int> pending_;
};