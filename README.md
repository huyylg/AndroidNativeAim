# AndroidNativeAim

Auto aim assistant for mobile FPS games on Android via Termux.

## Setup
1. Run `./termux_setup.sh`
2. Run `./termux_build.sh`
3. Place models in ./models/
4. Run `./scripts/start.sh`

## Training
Run python scripts in ./training/ on a PC with GPU.

## Performance
- FPS: >=60
- Latency: <50ms
- Targets: ARM64, C++17, OpenCV+NCNN
