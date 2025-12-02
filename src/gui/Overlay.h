#pragma once
#include <opencv2/opencv.hpp>
// Use Termux:X11 or simple console for debug

class Overlay {
public:
    void start();
    void drawBox(const cv::Rect& box, cv::Mat& frame);
private:
    // FPS counter
};
