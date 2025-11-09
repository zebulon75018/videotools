#include "video_producer.h"

#include <fstream>
#include <iostream>
#include <stdexcept>

using json = nlohmann::json;

VideoProducer::VideoProducer(Params p) : p_(std::move(p)) {}

int VideoProducer::run() {
    cv::Mat img1 = cv::imread(p_.image1Path, cv::IMREAD_COLOR);
    cv::Mat img2 = cv::imread(p_.image2Path, cv::IMREAD_COLOR);
    if (img1.empty() || img2.empty()) {
        std::cerr << "Error: could not read input images." << std::endl;
        return 2;
    }
    if (img2.size() != img1.size()) {
        cv::resize(img2, img2, img1.size(), 0, 0, cv::INTER_LINEAR);
    }

    json cfg;
    try {
        std::ifstream f(p_.jsonPath);
        if (!f) throw std::runtime_error("Cannot open JSON config: " + p_.jsonPath);
        f >> cfg;
    } catch (const std::exception& e) {
        std::cerr << "JSON error: " << e.what() << std::endl;
        return 3;
    }

    int outW = cfg.value("width", img1.cols);
    int outH = cfg.value("height", img1.rows);
    if (outW != img1.cols || outH != img1.rows) {
        cv::resize(img1, img1, cv::Size(outW, outH));
        cv::resize(img2, img2, cv::Size(outW, outH));
    }

    std::unique_ptr<TransitionBetween> transition;
    try {
        transition = createTransitionFromJson(cfg, (p_.fps>0? p_.fps:30.0), cv::Size(outW,outH));
    } catch (const std::exception& e) {
        std::cerr << "Transition error: " << e.what() << std::endl;
        return 4;
    }

    int frames = transition->frameCount();
    double fps = transition->fps();

    std::string codecStr = cfg.value("codec", std::string("MJPG"));
    if (codecStr.size() != 4) codecStr = "MJPG";
    int fourcc = cv::VideoWriter::fourcc(codecStr[0], codecStr[1], codecStr[2], codecStr[3]);

    cv::VideoWriter writer;
    if (!writer.open(p_.outputPath, fourcc, fps, cv::Size(outW, outH))) {
        std::cerr << "Error: could not open output video: " << p_.outputPath << std::endl;
        return 5;
    }

    for (int i=0; i<frames; ++i) {
        cv::Mat frame = transition->renderFrame(i, img1, img2);
        if (frame.empty()) {
            std::cerr << "Warning: empty frame at index " << i << std::endl;
            continue;
        }
        if (frame.size() != cv::Size(outW, outH)) {
            cv::resize(frame, frame, cv::Size(outW, outH));
        }
        writer.write(frame);
    }
    writer.release();
    std::cout << "Wrote " << frames << " frames at " << fps << " fps to '" << p_.outputPath << "' " << std::endl;
    return 0;
}

