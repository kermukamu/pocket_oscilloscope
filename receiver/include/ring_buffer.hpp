#pragma once
#include <vector>
#include <cstddef>

class RingBuffer {
public:
    explicit RingBuffer(size_t size) : data(size, 0.0f) {}

    void push(float v) {
        data[head] = v;
        head = (head + 1) % data.size();
        if (head == 0) filled = true;
    }

    void copyOrdered(std::vector<float>& out) const {
        out.clear();

        if (!filled) {
            out.insert(out.end(), data.begin(), data.begin() + head);
        } else {
            out.insert(out.end(), data.begin() + head, data.end());
            out.insert(out.end(), data.begin(), data.begin() + head);
        }
    }

private:
    std::vector<float> data;
    size_t head = 0;
    bool filled = false;
};