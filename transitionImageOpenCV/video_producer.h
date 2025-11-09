#ifndef VIDEO_PRODUCER_H
#define VIDEO_PRODUCER_H

#include <string>
#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include "transitions.h"

class VideoProducer {
public:
    struct Params {
        std::string image1Path;
        std::string image2Path;
        std::string jsonPath;
        std::string outputPath;
        double fps = 30.0; // CLI default; may be overridden by JSON
    };

    explicit VideoProducer(Params p);
    int run();

private:
    Params p_;
};

#endif // VIDEO_PRODUCER_H

