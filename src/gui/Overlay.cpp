#include "Overlay.h"
#include "utils/Logger.h"

void Overlay::start() {
    // Launch Termux:X11 server
    system("termux-x11 :0 &");
    // Draw loop
    while (true) {
        // Get frame, draw boxes, display
        LOG_INFO("FPS: 60"); // Counter
    }
}

void Overlay::drawBox(const cv::Rect& box, cv::Mat& frame) {
    cv::rectangle(frame, box, cv::Scalar(0,255,0), 2);
}
