#include "FrameBuffer.h"
#include <linux/fb.h>
#include <sys/ioctl.h>

FrameBuffer::FrameBuffer(const char* dev) : fd_(open(dev, O_RDONLY)) {
    if (fd_ < 0) throw std::runtime_error("FB open failed");
    struct fb_var_screeninfo vinfo;
    ioctl(fd_, FBIOGET_VSCREENINFO, &vinfo);
    // Setup mmap
}

cv::Mat FrameBuffer::capture() {
    // Mmap and copy to Mat (zero-copy view)
    return cv::Mat(height_, width_, CV_8UC3, mapped_);
}
