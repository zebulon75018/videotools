#include <iostream>
#include "video_producer.h"

static void printUsage(const char* argv0,char *argv1) {
    std::cout << "Usage: " << argv0 << " <image1> <image2> <config.json> <output_video> [fps] " << std::endl;
    if (std::string(argv1) == "-h" || std::string(argv1) == "--help") 
    {
       std::cout << "Transitions : " << std::endl;
       for(std::string t : TransitionBetween::getTransitiontring()) 
       {
           std::cout << "\t" << t << std::endl;
       }
       std::cout << "Easing : " << std::endl;
       for(std::string t : TransitionBetween::getEasing()) 
       {
           std::cout << "\t" << t << std::endl;
       }
    } else {
        std::cout << " -h or --help for full help " << std::endl;
    }
    std::cout << "Example JSON: { \"duration\": 5, \"type\": \"slideright\", \"easing\": \"ease-in-out\" } " << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 5)
    {
        if ( argc == 2 ) {
             printUsage(argv[0],argv[1]);
            } else {
             printUsage(argv[0],(char *)"");
            }
        return 1;
    }

    VideoProducer::Params p;
    p.image1Path = argv[1];
    p.image2Path = argv[2];
    p.jsonPath   = argv[3];
    p.outputPath = argv[4];
    if (argc >= 6) {
        try { p.fps = std::stod(argv[5]); } catch (...) { p.fps = 30.0; }
    }

    try {
        VideoProducer producer(p);
        return producer.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal: " << e.what() << std::endl;
        return 10;
    }
}

