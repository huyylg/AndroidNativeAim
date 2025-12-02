#include "AimController.h"
#include "utils/Logger.h"

AimController::AimController() {}

void AimController::start(Detector* det) {
    det_ = det;
}

cv::Point2f AimController::getAimPoint() {
    auto dets = det_->detect(cv::Mat()); // Get latest
    if (dets.empty()) return {0,0};
    // Sort by priority
    std::sort(dets.begin(), dets.end(), [&](const Detection& a, const Detection& b) {
        return getPriority(a) > getPriority(b);
    });
    cv::Point2f pred = predictor_.predict(dets, 1/60.0f);
    // Smoothing: lerp with previous
    static cv::Point2f last = {0,0};
    pred = last * 0.7 + pred * 0.3;
    last = pred;
    // Recoil comp: adjust up if firing (dummy)
    pred.y -= 5;
    return pred * sensitivity_;
}

int AimController::getPriority(const Detection& d) const {
    float dist = sqrt(d.box.x*d.box.x + d.box.y*d.box.y);
    float threat = d.conf;
    return (1/dist + threat) * 100; // Simplified
}

void AimController::adjustSensitivity(float factor) {
    sensitivity_ *= factor;
    LOG_DEBUG("Sensitivity: " + std::to_string(sensitivity_));
}
