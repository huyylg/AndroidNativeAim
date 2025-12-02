#pragma once
#include <ncnn/net.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <memory>
#include "utils/Timer.h"

struct Detection {
    cv::Rect box;
    float conf;
    int classId;
};

class Detector {
public:
    Detector(const std::string& param, const std::string& bin);
    ~Detector();
    bool load();
    void start(ScreenCapture* cap);
    std::vector<Detection> detect(const cv::Mat& frame);
    void warmup();

private:
    ncnn::Net net_;
    std::unique_ptr<std::thread> thread_;
    ScreenCapture* cap_ = nullptr;
    Timer timer_;
    bool vulkan_ = false;
    std::vector<Detection> results_;
    // Thread-safe queue for frames
};
