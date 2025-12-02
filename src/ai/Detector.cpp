#include "Detector.h"
#include <ncnn/cpu.h>
#include "utils/Logger.h"

Detector::Detector(const std::string& param, const std::string& bin) {
    vulkan_ = ncnn::get_gpu_count() > 0;
    if (vulkan_) ncnn::create_gpu_instance();
}

Detector::~Detector() {
    if (vulkan_) ncnn::destroy_gpu_instance();
}

bool Detector::load() {
    net_.opt.use_vulkan_compute = vulkan_;
    net_.opt.num_threads = 4;
    if (net_.load_param(param.c_str()) == -1 || net_.load_model(bin.c_str()) == -1) {
        return false;
    }
    warmup(); // Run 10 inferences
    LOG_INFO("Model loaded, Vulkan: " + std::to_string(vulkan_));
    return true;
}

void Detector::warmup() {
    cv::Mat dummy(640, 640, CV_8UC3);
    for (int i = 0; i < 10; ++i) detect(dummy);
}

std::vector<Detection> Detector::detect(const cv::Mat& frame) {
    // Pre-process: resize 640x640, normalize, BGR2RGB
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(640, 640));
    resized.convertTo(resized, CV_32F, 1/255.0);
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

    ncnn::Mat in = ncnn::Mat::from_pixels(resized.data, ncnn::Mat::PIXEL_RGB, 640, 640);
    in.substract_mean_normalize(0, ncnn::get_default_normalize_mean_values(), ncnn::get_default_normalize_inv_scale_values());

    ncnn::Extractor ex = net_.create_extractor();
    ex.input("images", in);
    ncnn::Mat out;
    ex.extract("output", out);

    // Post-process YOLO: decode boxes, NMS
    std::vector<Detection> dets;
    // Simplified NMS (use ncnn's built-in or implement)
    // Assume out is [1, 25200, 85] for YOLOv5s
    float threshold = 0.5;
    // Loop over outputs, apply NMS
    // ... (implement decode: center_x, center_y, w, h, conf)
    return dets;
}

void Detector::start(ScreenCapture* cap) {
    cap_ = cap;
    thread_ = std::make_unique<std::thread>([&]() {
        while (true) {
            cv::Mat frame = cap_->getFrame();
            results_ = detect(frame);
            if (timer_.elapsed() > 16) { // 60FPS
                LOG_WARN("Detection lag");
            }
            timer_.reset();
        }
    });
}
