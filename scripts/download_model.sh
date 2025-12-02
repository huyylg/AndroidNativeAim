#!/bin/bash

# Tải pre-trained YOLOv5s ONNX từ Ultralytics (hoặc GitHub)
cd $(dirname $0)/../models
if [ ! -f "yolov5s.onnx" ]; then
    echo "Downloading YOLOv5s ONNX..."
    wget https://github.com/ultralytics/assets/releases/download/v8.0.0/yolov5s.onnx
fi

# Cài onnx2ncnn nếu chưa (trong termux_setup.sh đã có NCNN)
if [ ! -f "/data/data/com.termux/files/usr/bin/onnx2ncnn" ]; then
    echo "Building onnx2ncnn tool..."
    cd $HOME/ncnn/tools/onnx
    cmake . -DCMAKE_BUILD_TYPE=Release
    make -j4
    cp onnx2ncnn $PREFIX/bin/
fi

# Convert ONNX -> NCNN
echo "Converting to NCNN..."
onnx2ncnn yolov5s.onnx yolov5s.param yolov5s.bin

# Optimize (FP16 cho nhanh)
ncnnoptimize yolov5s.param yolov5s.bin yolov5s_fp16.param yolov5s_fp16.bin 0

# Copy optimized models
cp yolov5s_fp16.param yolov5s.param
cp yolov5s_fp16.bin yolov5s.bin

echo "Pre-trained model ready in ./models/ (yolov5s.param/bin)"
echo "Edit src/ai/Detector.cpp to map classes (e.g., class 0 = 'person' as enemy)"
