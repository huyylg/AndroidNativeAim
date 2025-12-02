#pragma once
#include <chrono>
#include <thread>
#include "aim/AimController.h"

class InputInjector {
public:
    InputInjector();
    void start(AimController* aim);
    void injectTap(float x, float y);
    void injectSwipe(float x1, float y1, float x2, float y2, int duration);

private:
    AimController* aim_ = nullptr;
    // Use clock_nanosleep for precision
};
