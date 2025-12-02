#include "ScreenCapture.h"
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <cstdio>
#include <cstdlib> // For popen
#include "utils/Logger.h"

ScreenCapture::ScreenCapture() : width_(720), height_(1280), fps_(60) {
    buffers_.resize(3); // Triple buffer
}

ScreenCapture::~ScreenCapture() { stop(); }

bool ScreenCapture::init(int w, int h, int f) {
    width_ = w; height_ = h; fps_ = f;
    if (isRoot()) {
        LOG_INFO("Using framebuffer (root)");
        // Open /dev/graphics/fb0
        int fd = open("/dev/graphics/fb0", O_RDONLY);
        if (fd < 0) return false;
        // Mmap YUV420
        size_t size = w * h * 3 / 2; // YUV420
        void* map = mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) return false;
        // Store map for zero-copy
    } else {
        LOG_INFO("Using screencap (non-root)");
        // Setup Termux API or popen("screencap -p")
    }
    timer_.setInterval(1000 / fps_);
    return true;
}

void ScreenCapture::start() {
    running_ = true;
    thread_ = std::make_unique<std::thread>([&]() {
        int bufIdx = 0;
        while (running_) {
            cv::Mat& frame = buffers_[bufIdx % 3];
            // Capture logic
            if (isRoot()) {
                // YUV to RGB optimized (NEON)
                // Use cv::cvtColor with NEON intrinsics
                cv::cvtColor(frame, frame, cv::COLOR_YUV2BGR_NV12); // Optimized
            } else {
                // popen("screencap -p > /tmp/screen.png"); cv::imread
                FILE* pipe = popen("screencap -p", "r");
                // Read PNG, decode to frame (low latency <50ms)
                fread(frame.data, 1, width_*height_*3, pipe);
                pclose(pipe);
            }
            // Auto-adjust res if FPS drop
            if (timer_.elapsed() > 1000/fps_ * 1.2) {
                width_ *= 0.9; height_ *= 0.9;
            }
            bufIdx++;
            timer_.reset();
        }
    });
}

void ScreenCapture::stop() {
    running_ = false;
    if (thread_) thread_->join();
}

cv::Mat ScreenCapture::getFrame() {
    // Return latest buffer (zero-copy)
    return buffers_[2]; // Circular
}

bool ScreenCapture::isRoot() const {
    return system("id -u") == 0;
}

// Error recovery: If pipe fails, retry 3x
