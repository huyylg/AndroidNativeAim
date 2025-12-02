import onnx
from onnxsim import simplify
import ncnn

# Load ONNX
model = onnx.load("yolov5s.onnx")
onnx.checker.check_model(model)
model_simple, check = simplify(model)
onnx.save(model_simple, "yolov5s_simp.onnx")

# Convert
os.system('onnx2ncnn yolov5s_simp.onnx yolov5s.param yolov5s.bin')

# Calibrate for INT8
# Use calibration dataset
print("Model converted. Test on Android.")
