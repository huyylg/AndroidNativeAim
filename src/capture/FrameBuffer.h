#pragma once
#include <opencv2/opencv.hpp>
#include <memory>

class FrameBuffer {
public:
    FrameBuffer(const char* dev);
    cv::Mat capture();
private:
    int fd_;
    void* mapped_;
    int width_, height_;
};
