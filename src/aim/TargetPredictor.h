#pragma once
#include <opencv2/opencv.hpp>
#include <Eigen/Dense> // For Kalman (install via pkg)
#include <vector>

class TargetPredictor {
public:
    TargetPredictor();
    cv::Point2f predict(const std::vector<Detection>& dets, float dt);
    void update(const cv::Point2f& pos, const cv::Point2f& vel);

private:
    Eigen::MatrixXf kalman_; // 4x4 state [x,y,vx,vy]
    // Bezier curve for smooth path
    std::vector<cv::Point2f> bezierPoints_;
};
