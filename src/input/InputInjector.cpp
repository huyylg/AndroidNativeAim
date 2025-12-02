#include "InputInjector.h"
#include <unistd.h>
#include <linux/input.h>
#include <sys/syscall.h> // For uinput if root
#include "utils/Logger.h"

InputInjector::InputInjector() {}

void InputInjector::start(AimController* aim) {
    aim_ = aim;
    std::thread([this]() {
        while (true) {
            cv::Point2f target = aim_->getAimPoint();
            injectSwipe(360, 640, target.x, target.y, 50); // Center to target, 50ms
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 60Hz
        }
    }).detach();
}

void InputInjector::injectTap(float x, float y) {
    // Non-root: input tap x y
    char cmd[100];
    snprintf(cmd, sizeof(cmd), "input tap %f %f", x, y);
    system(cmd);
    // Root: uinput events
    if (getuid() == 0) {
        // Write to /dev/uinput
    }
    // Timing: clock_nanosleep
    struct timespec ts = {0, 10000000}; // 10ms
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, nullptr);
}

void InputInjector::injectSwipe(float x1, float y1, float x2, float y2, int duration) {
    char cmd[100];
    snprintf(cmd, sizeof(cmd), "input swipe %f %f %f %f %d", x1, y1, x2, y2, duration);
    system(cmd);
    // Multi-touch: extend for fingers
    // Rate limit: <100 events/sec
    static auto last = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count() < 10) return;
    last = now;
}
