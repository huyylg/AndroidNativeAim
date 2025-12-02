#pragma once
#include <vector>
#include "ai/Detector.h"
#include "aim/TargetPredictor.h"

class AimController {
public:
    AimController();
    void start(Detector* det);
    cv::Point2f getAimPoint();
    void adjustSensitivity(float factor);

private:
    Detector* det_ = nullptr;
    TargetPredictor predictor_;
    float sensitivity_ = 1.0f;
    // Priority: distance, threat (conf), visibility
    int getPriority(const Detection& d) const;
};
