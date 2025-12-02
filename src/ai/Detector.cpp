#include "Detector.h"
#include <ncnn/cpu.h>
#include <algorithm>
#include <numeric>  // For std::max_element
#include "utils/Logger.h"

Detector::Detector(const std::string& param, const std::string& bin) {
    vulkan_ = ncnn::get_gpu_count() > 0;
    if (vulkan_) {
        ncnn::create_gpu_instance();
        LOG_INFO("Vulkan GPU instance created");
    }
}

Detector::~Detector() {
    if (vulkan_) {
        ncnn::destroy_gpu_instance();
        LOG_INFO("Vulkan GPU instance destroyed");
    }
}

bool Detector::load() {
    net_.opt.use_vulkan_compute = vulkan_;
    net_.opt.num_threads = 4;  // Adjust based on cores
    net_.opt.lightmode = true; // Memory optimization

    if (net_.load_param(param.c_str()) == -1) {
        LOG_ERROR("Failed to load param: " + param);
        return false;
    }
    if (net_.load_model(bin.c_str()) == -1) {
        LOG_ERROR("Failed to load model: " + bin);
        return false;
    }

    warmup(); // Run 10 inferences for cache
    LOG_INFO("Model loaded successfully. Vulkan: " + std::to_string(vulkan_));
    LOG_INFO("Input size: 640x640, Classes: 80 (COCO pre-trained)");
    return true;
}

void Detector::warmup() {
    cv::Mat dummy(640, 640, CV_8UC3, cv::Scalar(128, 128, 128)); // Gray frame
    for (int i = 0; i < 10; ++i) {
        detect(dummy);
    }
    LOG_INFO("Warmup completed");
}

std::vector<Detection> Detector::detect(const cv::Mat& frame) {
    if (frame.empty()) {
        LOG_WARN("Empty frame for detection");
        return {};
    }

    // Pre-process: resize to 640x640, normalize [0,1], BGR2RGB
    cv::Mat resized;
    cv::resize(frame, resized, cv::Size(640, 640), 0, 0, cv::INTER_LINEAR); // NEON-optimized
    resized.convertTo(resized, CV_32FC3, 1.0 / 255.0);
    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB); // NEON-optimized

    // Convert to ncnn Mat (zero-copy if possible)
    ncnn::Mat in = ncnn::Mat::from_pixels_resize(resized.data, ncnn::Mat::PIXEL_RGB, frame.cols, frame.rows, 640, 640);

    // Normalize (ImageNet means/std)
    const float mean_vals[3] = {123.675f, 116.28f, 103.53f};
    const float norm_vals[3] = {1 / 58.395f, 1 / 57.12f, 1 / 57.375f};
    in.substract_mean_normalize(mean_vals, norm_vals);

    // Inference
    ncnn::Extractor ex = net_.create_extractor();
    ex.input("images", in);
    ncnn::Mat out;
    if (ex.extract("output", out) != 0) {
        LOG_ERROR("Inference failed");
        return {};
    }

    // Post-process YOLOv5s output: [1, 25200, 85] (batch=1, anchors=25200, 85=4box+1obj+80cls)
    std::vector<Detection> dets;
    if (out.h != 25200 || out.w != 85) {
        LOG_ERROR("Unexpected output shape: " + std::to_string(out.h) + "x" + std::to_string(out.w));
        return {};
    }

    float conf_threshold = 0.5f;
    float nms_threshold = 0.45f;
    int stride = 640; // Input size

    // Decode all candidates
    std::vector<Detection> candidates;
    for (int i = 0; i < out.h; ++i) {
        const float* values = out.row(i);

        // Object confidence
        float obj_conf = values[4];
        if (obj_conf <= conf_threshold) continue;

        // Class scores (80 classes)
        float class_score = 0;
        int class_id = -1;
        for (int k = 0; k < 80; ++k) {
            float score = values[5 + k];
            if (score > class_score) {
                class_score = score;
                class_id = k;
            }
        }

        // Total confidence
        float conf = obj_conf * class_score;
        if (conf <= conf_threshold) continue;

        // Decode box: center_x, center_y, w, h -> x1, y1, x2, y2 (sigmoid + anchor scaling)
        float cx = (values[0] * 2.0f - 0.5f + i % 80) / 80.0f * stride; // Simplified for YOLOv5 (no anchors in post)
        float cy = (values[1] * 2.0f - 0.5f + i / 80) / 80.0f * stride;
        float w = (values[2] * 2.0f) * stride;
        float h = (values[3] * 2.0f) * stride;

        float x1 = cx - w * 0.5f;
        float y1 = cy - h * 0.5f;
        float x2 = cx + w * 0.5f;
        float y2 = cy + h * 0.5f;

        // Scale back to original frame size
        float scale_x = (float)frame.cols / 640.0f;
        float scale_y = (float)frame.rows / 640.0f;
        x1 *= scale_x; y1 *= scale_y; x2 *= scale_x; y2 *= scale_y;

        // Clamp to frame bounds
        x1 = std::max(0.0f, std::min(x1, (float)frame.cols - 1));
        y1 = std::max(0.0f, std::min(y1, (float)frame.rows - 1));
        x2 = std::max(0.0f, std::min(x2, (float)frame.cols - 1));
        y2 = std::max(0.0f, std::min(y2, (float)frame.rows - 1));

        // Map pre-trained class: Only detect "person" (class 0) as "enemy"
        if (class_id == 0) {  // COCO class 0 = person -> map to enemy
            candidates.push_back({cv::Rect(static_cast<int>(x1), static_cast<int>(y1),
                                           static_cast<int>(x2 - x1), static_cast<int>(y2 - y1)),
                                  conf, 0 /* enemy classId */});
        }
    }

    // Apply NMS
    dets = nms(candidates, nms_threshold);
    LOG_DEBUG("Detected " + std::to_string(dets.size()) + " enemies (conf > " + std::to_string(conf_threshold) + ")");

    return dets;
}

// Simple NMS implementation (IoU-based)
std::vector<Detection> Detector::nms(const std::vector<Detection>& dets, float iou_threshold) {
    std::vector<Detection> result;
    std::vector<int> indices(dets.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Sort by confidence descending
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {
        return dets[a].conf > dets[b].conf;
    });

    std::vector<bool> suppressed(dets.size(), false);
    for (int i : indices) {
        if (suppressed[i]) continue;
        result.push_back(dets[i]);

        for (int j : indices) {
            if (suppressed[j] || j == i) continue;
            float iou = compute_iou(dets[i].box, dets[j].box);
            if (iou > iou_threshold) {
                suppressed[j] = true;
            }
        }
    }
    return result;
}

float Detector::compute_iou(const cv::Rect& a, const cv::Rect& b) {
    float inter_x1 = std::max(a.x, b.x);
    float inter_y1 = std::max(a.y, b.y);
    float inter_x2 = std::min(a.x + a.width, b.x + b.width);
    float inter_y2 = std::min(a.y + a.height, b.y + b.height);

    if (inter_x2 <= inter_x1 || inter_y2 <= inter_y1) return 0.0f;

    float inter_area = (inter_x2 - inter_x1) * (inter_y2 - inter_y1);
    float union_area = a.area() + b.area() - inter_area;
    return inter_area / union_area;
}

void Detector::start(ScreenCapture* cap) {
    if (!cap) {
        LOG_ERROR("Invalid capture pointer");
        return;
    }
    cap_ = cap;
    running_ = true;  // Add running_ flag to header if not there
    thread_ = std::make_unique<std::thread>([this]() {
        timer_.reset();
        while (running_) {
            cv::Mat frame = cap_->getFrame();
            if (!frame.empty()) {
                results_ = detect(frame);
            }
            long long elapsed = timer_.elapsed();
            if (elapsed > 16) {  // >16ms = <60FPS
                LOG_WARN("Detection lag: " + std::to_string(elapsed) + "us");
            }
            timer_.reset();
            // Sleep if too fast (yield CPU)
            if (elapsed < 8) {  // <8ms, sleep 1ms
                std::this_thread::sleep_for(std::chrono::microseconds(1000));
            }
        }
    });
    LOG_INFO("Detection thread started");
}

// Add to header if needed: bool running_ = false;
void Detector::stop() {
    running_ = false;
    if (thread_) {
        thread_->join();
        LOG_INFO("Detection thread stopped");
    }
}
