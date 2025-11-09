#include "transitions.h"

std::unique_ptr<TransitionBetween> createTransitionFromJson(const json& cfg, double defaultFps, const cv::Size& size) {
    double duration = cfg.value("duration", 3.0);
    double fps = cfg.value("fps", defaultFps>0?defaultFps:30.0);
    std::string type = cfg.value("type", std::string("fade"));
    std::string easingStr = cfg.value("easing", std::string("linear"));
    Easing::Type easing = Easing::parse(easingStr);

    // Optional mask blur
    MaskBlurOptions mblur{};
    if (cfg.contains("mask_blur")) {
        auto mb = cfg["mask_blur"];
        mblur.type  = mb.value("type", std::string("none"));
        mblur.ksize = mb.value("ksize", 0);
        mblur.sigma = mb.value("sigma", 0.0);
        mblur.opacityChange = mb.value("opacitychange", false);
    }

    std::string tLower = type; std::transform(tLower.begin(), tLower.end(), tLower.begin(), ::tolower);

    if (tLower == "slider") {
        std::string dir = cfg.value("direction", std::string("right-to-left"));
        std::string d = dir; std::transform(d.begin(), d.end(), d.begin(), ::tolower);
        SliderTransition::Dir sd = SliderTransition::Dir::RightToLeft;
        if (d=="left-to-right"||d=="ltr"||d=="left") sd = SliderTransition::Dir::LeftToRight;
        else if (d=="top-to-bottom"||d=="ttb"||d=="top") sd = SliderTransition::Dir::TopToBottom;
        else if (d=="bottom-to-top"||d=="btt"||d=="bottom") sd = SliderTransition::Dir::BottomToTop;
        return std::make_unique<SliderTransition>(duration, fps, easing, sd);
    } else if (tLower == "slideright") {
        // Backward compatibility
        return std::make_unique<SliderTransition>(duration, fps, easing, SliderTransition::Dir::RightToLeft);
    } else if (tLower == "fade") {
        return std::make_unique<FadeTransition>(duration, fps, easing, mblur);
    } else if (tLower == "appearright") {
        return std::make_unique<AppearRightTransition>(duration, fps, easing, mblur);
    } else if (tLower == "wipe") {
        std::string dir = cfg.value("direction", std::string("left-to-right"));
        std::string d = dir; std::transform(d.begin(), d.end(), d.begin(), ::tolower);
        WipeTransition::Dir wd = WipeTransition::Dir::LeftToRight;
        if (d=="right-to-left"||d=="rtl"||d=="right") wd = WipeTransition::Dir::RightToLeft;
        else if (d=="top-to-bottom"||d=="ttb"||d=="top") wd = WipeTransition::Dir::TopToBottom;
        else if (d=="bottom-to-top"||d=="btt"||d=="bottom") wd = WipeTransition::Dir::BottomToTop;
        return std::make_unique<WipeTransition>(duration, fps, easing, wd, mblur);
    } else if (tLower == "barndoor") {
        std::string o = cfg.value("orientation", std::string("horizontal"));
        BarndoorTransition::Orientation bo = (std::tolower(o[0])=='v') ? BarndoorTransition::Orientation::Vertical : BarndoorTransition::Orientation::Horizontal;
        return std::make_unique<BarndoorTransition>(duration, fps, easing, bo, mblur);
    } else if (tLower == "radial") {
        int cx = cfg.value("center_x", size.width/2);
        int cy = cfg.value("center_y", size.height/2);
        return std::make_unique<RadialTransition>(duration, fps, easing, cv::Point(cx,cy), mblur);
    } else if (tLower == "pie") {
        int cx = cfg.value("center_x", size.width/2);
        int cy = cfg.value("center_y", size.height/2);
        double start = cfg.value("start_angle", -90.0);
        std::string dir = cfg.value("direction", std::string("ccw"));
        std::string dl = dir; std::transform(dl.begin(), dl.end(), dl.begin(), ::tolower);
        bool ccw = (dl.find("ccw")!=std::string::npos || dl=="ccw");
        return std::make_unique<PieWipeTransition>(duration, fps, easing, cv::Point(cx,cy), start, ccw, mblur);
    } else if (tLower == "pieadvanced" || tLower == "piesweep") {
        int cx = cfg.value("center_x", size.width/2);
        int cy = cfg.value("center_y", size.height/2);
        double start = cfg.value("start_angle", -90.0);
        std::string dir = cfg.value("direction", std::string("ccw"));
        std::string dl = dir; std::transform(dl.begin(), dl.end(), dl.begin(), ::tolower);
        bool ccw = (dl.find("ccw")!=std::string::npos || dl=="ccw");
        double r0 = cfg.value("r0_frac", 0.0);
        double r1 = cfg.value("r1_frac", 1.0);
        double sweep = cfg.value("sweep_deg", 360.0);
        return std::make_unique<PieAdvancedTransition>(duration, fps, easing, cv::Point(cx,cy), start, ccw, r0, r1, sweep, mblur);
    } else if (tLower == "zoom") {
        std::string mode = cfg.value("mode", std::string("in"));
        return std::make_unique<ZoomTransition>(duration, fps, easing, mode);
    } else if (tLower == "blur") {
        return std::make_unique<BlurTransition>(duration, fps, easing,mblur);
    } else if (tLower == "checkerboard" || tLower == "damier") {
        int squares = cfg.value("squares", 8);
        int rows = cfg.value("rows", squares);
        int cols = cfg.value("cols", squares);
        bool stepwise = cfg.value("stepwise", true);
        unsigned seed = cfg.value("seed", 1234);
        return std::make_unique<CheckerboardTransition>(duration, fps, easing, rows, cols, stepwise, seed, mblur);
    } else if (tLower == "movingbars" || tLower == "bars") {
        std::string axis = cfg.value("axis", std::string("horizontal"));
        int count = cfg.value("count", 16);
        std::string direction = cfg.value("direction", std::string("bottom"));
        double speedMin = cfg.value("speed_min", 0.5);
        double speedMax = cfg.value("speed_max", 1.5);
        unsigned seed = cfg.value("seed", 42);
        return std::make_unique<MovingBarsTransition>(duration, fps, easing, axis, count, direction, speedMin, speedMax, seed, mblur);
    } else if (tLower == "interleave") {
        int bands = cfg.value("bands", 10);
        return std::make_unique<InterleaveRectanglesTransition>(duration, fps, easing, bands, mblur);
    } else if (tLower == "randomcircles") {
        int count = cfg.value("count", 20);
        unsigned seed = cfg.value("seed", 12345);
        return std::make_unique<RandomCirclesTransition>(duration, fps, easing, count, seed, mblur);
    } else if (tLower == "randomsquares") {
        int count = cfg.value("count", 20);
        unsigned seed = cfg.value("seed", 12345);
        return std::make_unique<RandomSquaresTransition>(duration, fps, easing, count, seed, mblur);
    } else if (tLower == "blinds") {
        std::string axis = cfg.value("axis", std::string("vertical"));
        int count = cfg.value("count", 16);
        std::string direction = cfg.value("direction", std::string("left"));
        double waveAmp = cfg.value("wave_amplitude", 0.0);
        double wavePhase = cfg.value("wave_phase", 0.0);
        return std::make_unique<BlindsTransition>(duration, fps, easing, axis, count, direction, waveAmp, wavePhase, mblur);
    } else if (tLower == "checkerboardanimated" || tLower == "checkerboard_anim" || tLower == "checkerboard-animated") {
        int squares = cfg.value("squares", 10);
        int rows = cfg.value("rows", squares);
        int cols = cfg.value("cols", squares);
        std::string ord = cfg.value("order", std::string("row"));
        unsigned seed = cfg.value("seed", 1234);
        AnimatedCheckerboardTransition::Order o = AnimatedCheckerboardTransition::Order::Row;
        std::string ol = ord; std::transform(ol.begin(), ol.end(), ol.begin(), ::tolower);
        if (ol=="col"||ol=="column"||ol=="columns") o = AnimatedCheckerboardTransition::Order::Col;
        else if (ol=="diag"||ol=="diagonal") o = AnimatedCheckerboardTransition::Order::Diag;
        else if (ol=="invdiag"||ol=="invdiagonal") o = AnimatedCheckerboardTransition::Order::InvDiag;
        else if (ol=="row") o = AnimatedCheckerboardTransition::Order::Row;
        else if (ol=="random") o = AnimatedCheckerboardTransition::Order::Random;
        return std::make_unique<AnimatedCheckerboardTransition>(duration, fps, easing, rows, cols, o, seed, mblur);
    }

    throw std::runtime_error("Unknown transition type: " + type);
}


std::list<std::string> TransitionBetween::getTransitiontring() {
    return  {
    "slider",
    "slideright",
    "fade",
    "appearright",
    "wipe",
    "barndoor",
    "radial",
    "pie",
    "pieadvanced","piesweep",
    "zoom",
    "checkerboard","damier",
    "movingbars","bars",
    "interleave",
    "randomcircles",
    "randomsquares",
    "blinds",
    "blur",
    "checkerboardanimated","checkerboard_anim","checkerboard-animated"
    };
}

std::list<std::string> TransitionBetween::getEasing() {
  return {
  "ease-in" ,
  "ease-out",
  "ease-in-out" ,
  "ease-in-bounce" ,
  "ease-out-bounce" ,
  "ease-in-elastic" ,
  "ease-out-elastic" ,
  "ease-in-circ" ,
  "ease-out-circ" ,
  "ease-inout-circ" ,
  "ease-in-quint" ,
  "ease-out-quint" ,
  "ease-inout-quint" 
  };
}
