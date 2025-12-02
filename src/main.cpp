#include <iostream>
#include "capture/ScreenCapture.h"
#include "ai/Detector.h"
#include "aim/AimController.h"
#include "input/InputInjector.h"
#include "utils/Logger.h"
#include "gui/Overlay.h"

int main() {
    Logger::init("aimbot.log");
    LOG_INFO("Starting AndroidNativeAim");

    ScreenCapture cap;
    if (!cap.init(720, 1280, 60)) {
        LOG_ERROR("Capture init failed");
        return -1;
    }

    Detector det("models/yolov5s.param", "models/yolov5s.bin");
    if (!det.load()) {
        LOG_ERROR("Detector load failed");
        return -1;
    }

    AimController aim;
    InputInjector input;
    Overlay overlay;

    std::thread captureThread([&]() { cap.start(); });
    std::thread detectThread([&]() { det.start(&cap); });
    std::thread aimThread([&]() { aim.start(&det); });
    std::thread inputThread([&]() { input.start(&aim); });
    std::thread guiThread([&]() { overlay.start(); });

    // Run until signal
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    captureThread.join();
    // ... join others
    return 0;
}
