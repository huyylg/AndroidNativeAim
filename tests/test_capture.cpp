#include <gtest/gtest.h>
#include "capture/ScreenCapture.h"

TEST(CaptureTest, Init) {
    ScreenCapture cap;
    EXPECT_TRUE(cap.init(720,1280,60));
}
