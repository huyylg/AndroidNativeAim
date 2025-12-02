#!/bin/bash

# Check Android version
ANDROID_VER=$(getprop ro.build.version.release | cut -d. -f1)
if [ $ANDROID_VER -lt 8 ]; then
    echo "Android 8+ required"
    exit 1
fi

# Update Termux
pkg update -y && pkg upgrade -y

# Install deps
pkg install -y clang cmake ninja make git wget unzip termux-api python

# Build OpenCV (NEON optimized)
cd $HOME
git clone https://github.com/opencv/opencv.git -b 4.8.0
cd opencv
mkdir build && cd build
cmake -DCMAKE_TOOLCHAIN_FILE=$HOME/android-ndk-r25c/build/cmake/android.toolchain.cmake \
      -DANDROID_ABI=arm64-v8a -DANDROID_PLATFORM=android-26 \
      -DWITH_NEON=ON -DBUILD_SHARED_LIBS=OFF -DCMAKE_BUILD_TYPE=Release ..
ninja
mkdir -p $HOME/opencv-mobile-4.8.0-android/sdk/native/jni
# Copy libs (simplified; adjust for full build)

# Build NCNN with Vulkan
cd $HOME
git clone https://github.com/Tencent/ncnn.git
cd ncnn
git submodule update --init
mkdir build-android-vulkan && cd build-android-vulkan
sh ../tools/gnu-android.sh arm64-v8a android-26
cmake -DCMAKE_TOOLCHAIN_FILE=../tools/gnu-android.toolchain.cmake \
      -DNCNN_VULKAN=ON -DCMAKE_BUILD_TYPE=Release ..
make -j4
make install
mkdir -p $HOME/ncnn/build-android-vulkan/install/lib/cmake/ncnn

# Download pre-trained model
cd $HOME/AndroidNativeAim/scripts
bash download_model.sh
echo "Pre-trained model downloaded and converted."

# Install Termux:API
termux-api-setup  # Run manually if needed

# Env vars
echo 'export ANDROID_NDK_HOME=$HOME/android-ndk-r25c' >> ~/.bashrc
echo 'export PATH=$PATH:$HOME/ncnn/build-android-vulkan/tools/android/ndk' >> ~/.bashrc
source ~/.bashrc

# Detect Vulkan
if command -v vulkaninfo &> /dev/null; then
    echo "Vulkan supported"
else
    echo "Fallback to CPU"
fi

# Test
echo "Setup complete. Run cmake --version to verify."
