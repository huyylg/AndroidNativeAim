import torch
from ultralytics import YOLO

model = YOLO('path/to/trained.pt')
model.export(format='onnx', imgsz=640, simplify=True)

# ONNX to NCNN
os.system('onnx2ncnn input.onnx yolov5s.param yolov5s.bin')
# Quantize
os.system('ncnnoptimize yolov5s.param yolov5s.bin yolov5s_opt.param yolov5s_opt.bin 0')  # INT8
