#include "NCNNLoader.h"
#include <ncnn/net.h>

NCNNLoader::NCNNLoader(const std::string& param, const std::string& bin) : net_(new ncnn::Net()) {
    // Load as above
}
