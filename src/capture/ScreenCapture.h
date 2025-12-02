#pragma once
#include <memory>
#include <thread>
#include <vector>
#include <opencv2/opencv.hpp>
#include "utils/Timer.h"

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();
    bool init(int width, int height, int fps);
    void start();
    void stop();
    cv::Mat getFrame();  // Zero-copy access
    bool isRoot() const; // Check root for framebuffer

private:
    std::unique_ptr<std::thread> thread_;
    std::vector<cv::Mat> buffers_; // Triple buffering
    int width_, height_, fps_;
    Timer timer_;
    bool running_ = false;
    // Shared memory via mmap for framebuffer
};
