#pragma once
#include <memory>
#include <ncnn/net.h>

class NCNNLoader {
public:
    NCNNLoader(const std::string& param, const std::string& bin);
    ncnn::Net* getNet() { return net_.get(); }
private:
    std::unique_ptr<ncnn::Net> net_;
};
