#include <benchmark/benchmark.h>
#include "ai/Detector.h"
// Run benchmarks
static void BM_Detect(benchmark::State& state) {
    Detector det("models/yolov5s.param", "models/yolov5s.bin");
    det.load();
    cv::Mat frame(640,640,CV_8UC3);
    for (auto _ : state) {
        det.detect(frame);
    }
}
BENCHMARK(BM_Detect);
