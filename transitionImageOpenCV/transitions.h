#ifndef TRANSITIONS_H
#define TRANSITIONS_H

#include <opencv2/opencv.hpp>
#include <nlohmann/json.hpp>

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <stdexcept>

using json = nlohmann::json;

#ifndef PI
#define PI 3.1415926545
#endif

// -------------------- Global mask blur options --------------------
struct MaskBlurOptions {
    std::string type = "none"; // none | blur | gaussian | median
    int ksize = 0;              // odd > 0
    double sigma = 0.0;         // for gaussian
    bool opacityChange = false; 
    bool enabled() const { return ksize > 0 && type != "none"; }
};

// -------------------- Utility: Easing --------------------
namespace Easing {
    enum class Type { Linear, EaseIn, EaseOut, EaseInOut, EaseInBounce,
                      EaseOutBounce, EaseInElastic, EaseOutElastic,
                      EaseInCirc, EaseOutCirc, EaseInOutCirc,
                      EaseInQuint, EaseOutQuint, EaseInOutQuint
    };

    inline Type parse(const std::string& sIn) {
        std::string s = sIn;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s == "ease-in" || s == "easein") return Type::EaseIn;
        if (s == "ease-out" || s == "easeout") return Type::EaseOut;
        if (s == "ease-in-out" || s == "easeinout") return Type::EaseInOut;
        if (s == "ease-in-bounce" || s == "easeinbounce") return Type::EaseInBounce;
        if (s == "ease-out-bounce" || s == "easeoutbounce") return Type::EaseOutBounce;
        if (s == "ease-in-elastic" || s == "easeinelastic") return Type::EaseInElastic;
        if (s == "ease-out-elastic" || s == "easeoutelastic") return Type::EaseOutElastic;
        if (s == "ease-in-circ" || s == "easeincirc") return Type::EaseInCirc;
        if (s == "ease-out-circ" || s == "easeoutcirc") return Type::EaseOutCirc;
        if (s == "ease-inout-circ" || s == "easeinoutcirc") return Type::EaseInOutCirc;
        if (s == "ease-in-quint" || s == "easeinquint") return Type::EaseInQuint;
        if (s == "ease-out-quint" || s == "easeoutquint") return Type::EaseOutQuint;
        if (s == "ease-inout-quint" || s == "easeinoutquint") return Type::EaseInOutQuint;
        return Type::Linear;
    }

    inline double apply(double t, Type type) {
        t = std::clamp(t, 0.0, 1.0);
        switch(type) {
            case Type::EaseIn:    return t*t*t;
            case Type::EaseOut:   { double u = 1.0 - t; return 1.0 - u*u*u; }
            case Type::EaseInOut: return t<0.5 ? 4*t*t*t : 1.0 - std::pow(-2*t+2,3)/2.0;
            case Type::EaseInBounce : return std::pow( 2, 6 * (t - 1) ) * std::abs( std::sin( t * PI * 3.5 ) );
            case Type::EaseOutBounce : return 1 - std::pow( 2, -6 * t ) * std::abs( std::cos( t * PI * 3.5 ) );
            case Type::EaseInElastic: { double t2 = t * t; return t2 * t2 * std::sin( t * PI * 4.5 ); }
            case Type::EaseOutElastic: { double t2 = (t - 1) * (t - 1);return 1 - t2 * t2 * std::cos( t * PI * 4.5 ); }
            case Type::EaseInCirc: return 1 - std::sqrt( 1 - t );
            case Type::EaseOutCirc  : return std::sqrt( t );
            case Type::EaseInOutCirc: {
                if( t < 0.5 ) { return (1 - std::sqrt( 1 - 2 * t )) * 0.5; }
                else { return (1 + std::sqrt( 2 * t - 1 )) * 0.5; }
            }
            case Type::EaseInQuint: { double t2 = t * t;return t * t2 * t2;}
            case Type::EaseOutQuint: {double t2 = (--t) * t;return 1 + t * t2 * t2;}
            case Type::EaseInOutQuint: {
                double t2;
                if( t < 0.5 ) {t2 = t * t;return 16 * t * t2 * t2;}
                else {t2 = (--t) * t;return 1 + 16 * t * t2 * t2;}
            }
            default: return t;
        }
    }
}

// -------------------- Utility: Blend --------------------
namespace Blend {
    enum class Mode { Normal, Add, Screen };

    inline Mode parse(const std::string& sIn){
        std::string s=sIn;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (s=="add"||s=="additive") return Mode::Add;
        if (s=="screen") return Mode::Screen;
        return Mode::Normal;
    }

    inline cv::Mat apply(const cv::Mat& A, const cv::Mat& B, double alpha, Mode mode){
        CV_Assert(A.size()==B.size() && A.type()==B.type());
        alpha = std::clamp(alpha, 0.0, 1.0);
        if (mode==Mode::Normal){
            cv::Mat out; cv::addWeighted(A, 1.0-alpha, B, alpha, 0.0, out); return out;
        }
        cv::Mat Af,Bf; A.convertTo(Af, CV_32F); B.convertTo(Bf, CV_32F);
        cv::Mat blend;
        if (mode==Mode::Add){
            cv::add(Af, Bf, blend);
            cv::min(blend, 255.0, blend);
        } else { // Screen: 1-(1-A)(1-B)
            Af = Af/255.0f; Bf = Bf/255.0f;
            blend = 1.0 - (1.0 - Af).mul(1.0 - Bf);
            blend *= 255.0f;
        }
        cv::Mat outf = Af*(1.0-alpha) + blend*alpha;
        cv::Mat out; outf.convertTo(out, A.type());
        return out;
    }
}

// -------------------- Utility: Mask helpers --------------------
struct MaskUtils {
    static void applyMaskBlur(cv::Mat1b& mask, const MaskBlurOptions& opt) {
        if (!opt.enabled()) return;
        int k = opt.ksize;
        if (k % 2 == 0) k += 1; // must be odd
        std::string t = opt.type; std::transform(t.begin(), t.end(), t.begin(), ::tolower);
        if (t == "blur") {
            cv::blur(mask, mask, cv::Size(k,k));
        } else if (t == "gaussian" || t == "gaussianblur") {
            double sigma = opt.sigma > 0 ? opt.sigma : 0;
            cv::GaussianBlur(mask, mask, cv::Size(k,k), sigma);
        } else if (t == "median" || t == "medianblur") {
            cv::medianBlur(mask, mask, k);
        }
    }

    // Blend two images using a single-channel mask (0..255). White favors imgB, black favors imgA.
    static cv::Mat blendWithMask(const cv::Mat& imgA, const cv::Mat& imgB, cv::Mat1b mask, const MaskBlurOptions& blurOpt = {}) {
        CV_Assert(imgA.size() == imgB.size());
        CV_Assert(imgA.type() == imgB.type());
        CV_Assert(mask.size() == imgA.size());
        applyMaskBlur(mask, blurOpt);
        cv::Mat maskF, invMaskF;
        mask.convertTo(maskF, CV_32F, 1.0/255.0);
        invMaskF = 1.0 - maskF;
        cv::Mat aF, bF;
        imgA.convertTo(aF, CV_32F);
        imgB.convertTo(bF, CV_32F);
        std::vector<cv::Mat> aC, bC, outC;
        cv::split(aF, aC);
        cv::split(bF, bC);
        outC.resize(aC.size());
        for (size_t c=0; c<aC.size(); ++c) {
            outC[c] = aC[c].mul(invMaskF) + bC[c].mul(maskF);
        }
        cv::Mat out;
        cv::merge(outC, out);
        out.convertTo(out, imgA.type());
        return out;
    }

    // Create a linear gradient mask (0..255) across the image with a given angle (degrees).
    static cv::Mat1b createLinearGradientMask(const cv::Size& sz, double angleDeg) {
        cv::Mat1b mask(sz, uchar(0));
        double ang = angleDeg * M_PI / 180.0;
        double dx = std::cos(ang);
        double dy = std::sin(ang);
        std::vector<cv::Point2f> corners = {
            {0,0},
            {(float)(sz.width-1),0},
            {0,(float)(sz.height-1)},
            {(float)(sz.width-1),(float)(sz.height-1)}
        };
        double minProj = 1e18, maxProj = -1e18;
        for (auto& p: corners) {
            double proj = p.x*dx + p.y*dy;
            minProj = std::min(minProj, proj);
            maxProj = std::max(maxProj, proj);
        }
        double range = (maxProj - minProj);
        if (range <= 1e-9) range = 1.0;
        for (int y=0; y<sz.height; ++y) {
            uchar* row = mask.ptr(y);
            for (int x=0; x<sz.width; ++x) {
                double proj = x*dx + y*dy;
                double t = (proj - minProj) / range;
                int v = (int)std::round(255.0 * std::clamp(t, 0.0, 1.0));
                row[x] = (uchar)v;
            }
        }
        return mask;
    }
};

// -------------------- Transition Base --------------------
class TransitionBetween {
public:
    TransitionBetween(double durationSec, double fps, Easing::Type easing, MaskBlurOptions maskBlurOpt = {})
        : duration_(durationSec), fps_(fps), easing_(easing), maskBlur_(std::move(maskBlurOpt)) {
        if (duration_ <= 0) throw std::runtime_error("duration must be > 0");
        if (fps_ <= 0) fps_ = 30.0;
        frameCount_ = std::max(1, (int)std::round(duration_ * fps_));
    }

    virtual ~TransitionBetween() = default;

    int frameCount() const { return frameCount_; }
    double fps() const { return fps_; }
    double duration() const { return duration_; }

    cv::Mat renderFrame(int i, const cv::Mat& img1, const cv::Mat& img2) {
        double t = (frameCount_ <= 1) ? 1.0 : (double)i / (double)(frameCount_-1);
        double te = Easing::apply(t, easing_);
        return composite(te, img1, img2);
    }
    static std::list<std::string> getTransitiontring();
    static std::list<std::string> getEasing();


protected:
    const MaskBlurOptions& maskBlur() const { return maskBlur_; }
    virtual cv::Mat composite(double t, const cv::Mat& img1, const cv::Mat& img2) = 0;

    int getOpacityMask(double t) {
               if (maskBlur_.opacityChange)
               {
                   return (int)((double)255*t);
               } else {
                   return 255;
               }
    }

    int frameCount_ = 0;
    double duration_ = 0.0;
    double fps_ = 30.0;
    bool changeOpacityMask_ = true;
    Easing::Type easing_ = Easing::Type::Linear;
    MaskBlurOptions maskBlur_{};
};

// -------------------- Slider Transition (multi-direction) --------------------
class SliderTransition : public TransitionBetween {
public:
    enum class Dir { LeftToRight, RightToLeft, TopToBottom, BottomToTop };
    SliderTransition(double d, double f, Easing::Type e, Dir dir)
        : TransitionBetween(d,f,e), dir_(dir) {}
protected:
    cv::Mat composite(double t, const cv::Mat& img1, const cv::Mat& img2) override {
        CV_Assert(img1.size() == img2.size());
        cv::Mat out = img1.clone();
        int W = out.cols, H = out.rows;
        if (dir_==Dir::RightToLeft) {
            int x = (int)std::round((1.0 - t) * W);
            int x0 = std::max(0, x);
            int width = std::min(W, x + W) - x0;
            if (width > 0) {
                int srcX = std::max(0, -x);
                img2(cv::Rect(srcX, 0, width, H)).copyTo(out(cv::Rect(x0, 0, width, H)));
            }
        } else if (dir_==Dir::LeftToRight) {
            int x = (int)std::round((t - 1.0) * W); // start off-screen left
            int x0 = std::max(0, x);
            int width = std::min(W, x + W) - x0;
            if (width > 0) {
                int srcX = std::max(0, -x);
                img2(cv::Rect(srcX, 0, width, H)).copyTo(out(cv::Rect(x0, 0, width, H)));
            }
        } else if (dir_==Dir::BottomToTop) {
            int y = (int)std::round((1.0 - t) * H);
            int y0 = std::max(0, y);
            int height = std::min(H, y + H) - y0;
            if (height > 0) {
                int srcY = std::max(0, -y);
                img2(cv::Rect(0, srcY, W, height)).copyTo(out(cv::Rect(0, y0, W, height)));
            }
        } else { // TopToBottom
            int y = (int)std::round((t - 1.0) * H); // start off-screen top
            int y0 = std::max(0, y);
            int height = std::min(H, y + H) - y0;
            if (height > 0) {
                int srcY = std::max(0, -y);
                img2(cv::Rect(0, srcY, W, height)).copyTo(out(cv::Rect(0, y0, W, height)));
            }
        }
        return out;
    }
    Dir dir_;
};

// -------------------- Fade Transition --------------------
class FadeTransition : public TransitionBetween {
public:
    using TransitionBetween::TransitionBetween;
protected:
    cv::Mat composite(double t, const cv::Mat& img1, const cv::Mat& img2) override {
        CV_Assert(img1.size() == img2.size());
        return Blend::apply(img1, img2, t, Blend::Mode::Normal);
    }
};

// -------------------- AppearRight (mask-based) --------------------
class AppearRightTransition : public TransitionBetween {
public:
    using TransitionBetween::TransitionBetween;
protected:
    cv::Mat composite(double t, const cv::Mat& img1, const cv::Mat& img2) override {
        CV_Assert(img1.size() == img2.size());
        int W = img1.cols, H = img1.rows;
        int w = (int)std::round(t * W);
        cv::Mat1b mask(H, W, uchar(0));
        int x0 = std::max(0, W - w);
        if (x0 < W) {
            cv::rectangle(mask, cv::Rect(x0, 0, W - x0, H), cv::Scalar(getOpacityMask(255)), cv::FILLED);
        }
        return MaskUtils::blendWithMask(img1, img2, mask, maskBlur());
    }
};

// -------------------- Wipe Transition (directional) --------------------
class WipeTransition : public TransitionBetween {
public:
    enum class Dir { LeftToRight, RightToLeft, TopToBottom, BottomToTop };
    WipeTransition(double d, double f, Easing::Type e, Dir dir, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), dir_(dir) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        CV_Assert(a.size() == b.size());
        int W=a.cols, H=a.rows;
        cv::Mat1b m(H,W, uchar(0));
        switch(dir_) {
            case Dir::LeftToRight: {
                int w = (int)std::round(t*W);
                if (w>0) cv::rectangle(m, cv::Rect(0,0,std::min(w,W),H), getOpacityMask(t), cv::FILLED);
            } break;
            case Dir::RightToLeft: {
                int w = (int)std::round(t*W);
                int x0 = std::max(0, W - w);
                if (x0<W) cv::rectangle(m, cv::Rect(x0,0,W-x0,H), getOpacityMask(t), cv::FILLED);
            } break;
            case Dir::TopToBottom: {
                int h = (int)std::round(t*H);
                if (h>0) cv::rectangle(m, cv::Rect(0,0,W,std::min(h,H)), getOpacityMask(t), cv::FILLED);
            } break;
            case Dir::BottomToTop: {
                int h = (int)std::round(t*H);
                int y0 = std::max(0, H - h);
                if (y0<H) cv::rectangle(m, cv::Rect(0,y0,W,H-y0), getOpacityMask(t), cv::FILLED);
            } break;
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    Dir dir_;
};

// -------------------- Barndoor Transition --------------------
class BarndoorTransition : public TransitionBetween {
public:
    enum class Orientation { Horizontal, Vertical };
    BarndoorTransition(double d, double f, Easing::Type e, Orientation o, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), orient_(o) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows;
        cv::Mat1b m(H,W, uchar(0));
        if (orient_==Orientation::Horizontal) {
            int half = (int)std::round(0.5 * t * W);
            int cx = W/2;
            int x0 = std::max(0, cx - half);
            int x1 = std::min(W, cx + half);
            if (x1>x0) cv::rectangle(m, cv::Rect(x0,0,x1-x0,H), getOpacityMask(255), cv::FILLED);
        } else {
            int half = (int)std::round(0.5 * t * H);
            int cy = H/2;
            int y0 = std::max(0, cy - half);
            int y1 = std::min(H, cy + half);
            if (y1>y0) cv::rectangle(m, cv::Rect(0,y0,W,y1-y0), getOpacityMask(255), cv::FILLED);
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    Orientation orient_;
};

// -------------------- Radial (circular) Wipe --------------------
class RadialTransition : public TransitionBetween {
public:
    RadialTransition(double d, double f, Easing::Type e, cv::Point center, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), center_(center) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows;
        double maxR = std::max({
            std::hypot((double)center_.x, (double)center_.y),
            std::hypot((double)W-center_.x, (double)center_.y),
            std::hypot((double)center_.x, (double)H-center_.y),
            std::hypot((double)W-center_.x,(double)H-center_.y)
        });
        double r = t * maxR;
        cv::Mat1b m(H,W, uchar(0));
        cv::circle(m, center_, (int)std::ceil(r), getOpacityMask(255), cv::FILLED);
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    cv::Point center_;
};

// -------------------- Pie (sector) Wipe --------------------
class PieWipeTransition : public TransitionBetween {
public:
    PieWipeTransition(double d, double f, Easing::Type e, cv::Point center, double startDeg, bool ccw, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), center_(center), startDeg_(startDeg), ccw_(ccw) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows;
        double maxR = std::max({
            std::hypot((double)center_.x, (double)center_.y),
            std::hypot((double)W-center_.x, (double)center_.y),
            std::hypot((double)center_.x, (double)H-center_.y),
            std::hypot((double)W-center_.x,(double)H-center_.y)
        });
        double sweep = 360.0 * t * (ccw_? 1.0 : -1.0);
        int steps = std::max(2, (int)(std::abs(sweep)/2.0) + 1);
        std::vector<cv::Point> poly; poly.reserve(steps+2);
        poly.emplace_back(center_);
        for (int k=0; k<=steps; ++k){
            double frac = (double)k / (double)steps;
            double ang = (startDeg_ + sweep*frac) * M_PI / 180.0;
            int x = (int)std::round(center_.x + maxR*std::cos(ang));
            int y = (int)std::round(center_.y + maxR*std::sin(ang));
            poly.emplace_back(x,y);
        }
        cv::Mat1b m(H,W, uchar(0));
        const cv::Point* pts = poly.data(); int npts = (int)poly.size();
        cv::fillPoly(m, &pts, &npts, 1, cv::Scalar(getOpacityMask(t)));
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    cv::Point center_; double startDeg_; bool ccw_;
};

// -------------------- Pie + Radius + Sweep (advanced) --------------------
class PieAdvancedTransition : public TransitionBetween {
public:
    PieAdvancedTransition(double d, double f, Easing::Type e, cv::Point center, double startDeg, bool ccw,
                          double r0_frac, double r1_frac, double sweep_deg, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), center_(center), startDeg_(startDeg), ccw_(ccw),
          r0_(r0_frac), r1_(r1_frac), sweep_(sweep_deg) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows;
        double maxR = std::max({
            std::hypot((double)center_.x, (double)center_.y),
            std::hypot((double)W-center_.x, (double)center_.y),
            std::hypot((double)center_.x, (double)H-center_.y),
            std::hypot((double)W-center_.x,(double)H-center_.y)
        });
        double r = (r0_ + (r1_-r0_)*t) * maxR;
        double sweepNow = sweep_ * t * (ccw_? 1.0 : -1.0);
        int steps = std::max(2, (int)(std::abs(sweepNow)/2.0)+1);
        std::vector<cv::Point> poly; poly.emplace_back(center_);
        for(int k=0;k<=steps;++k){
            double frac=(double)k/steps;
            double ang=(startDeg_ + sweepNow*frac)*M_PI/180.0;
            int x=(int)std::round(center_.x + r*std::cos(ang));
            int y=(int)std::round(center_.y + r*std::sin(ang));
            poly.emplace_back(x,y);
        }
        cv::Mat1b m(H,W, uchar(0));
        const cv::Point* pts=poly.data(); int n=(int)poly.size();
        cv::fillPoly(m, &pts, &n, 1, cv::Scalar(getOpacityMask(t)));
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    cv::Point center_; double startDeg_; bool ccw_; double r0_, r1_, sweep_;
};

// -------------------- Zoom Transition --------------------
class ZoomTransition : public TransitionBetween {
public:
    ZoomTransition(double d, double f, Easing::Type e, std::string mode="in")
        : TransitionBetween(d,f,e), mode_(std::move(mode)) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows; cv::Point2f c(W/2.0f, H/2.0f);
        double s = (mode_=="out") ? (1.5 - 0.5*t) : (0.5 + 0.5*t);
        cv::Mat M = cv::getRotationMatrix2D(c, 0.0, s);
        cv::Mat bZoom; cv::warpAffine(b, bZoom, M, b.size(), cv::INTER_LINEAR, cv::BORDER_REPLICATE);
        double alpha = t;
        return Blend::apply(a, bZoom, alpha, Blend::Mode::Normal);
    }
    std::string mode_;
};

// -------------------- Interleaving Stripes (left/right) --------------------
class InterleaveRectanglesTransition : public TransitionBetween {
public:
    InterleaveRectanglesTransition(double d, double f, Easing::Type e, int bands, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), bands_(std::max(2,bands)) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows; cv::Mat1b m(H,W, uchar(0));
        int bandW = std::max(1, W / bands_);
        for (int i=0;i<bands_;++i){
            int xStart = i*bandW; int xEnd = (i==bands_-1? W : xStart+bandW);
            bool fromLeft = (i%2==0);
            int width = (int)std::round((xEnd - xStart) * t);
            if (fromLeft) {
                cv::Rect r(xStart, 0, std::min(width, xEnd-xStart), H);
                if (r.width>0) cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
            } else {
                int x0 = xEnd - width; x0 = std::max(x0, xStart);
                cv::Rect r(x0, 0, xEnd - x0, H);
                if (r.width>0) cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
            }
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    int bands_;
};

// -------------------- Blur Circles --------------------
class BlurTransition : public TransitionBetween {
public:
    BlurTransition(double d, double f, Easing::Type e, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        double beta = 1.0 - t; // Complémentaire pour la première image

        // Calculer l'intensité du flou (taille du noyau, doit être impair)
        int blurKernelSize1 = static_cast<int>(t * 15) * 8 + 1; // Flou croissant pour img1
        int blurKernelSize2 = static_cast<int>(beta * 15) * 8 + 1; // Flou décroissant pour img2

        cv::Mat blurredImg1, blurredImg2;
        // Appliquer le flou gaussien
        cv::GaussianBlur(a, blurredImg1, cv::Size(blurKernelSize1, blurKernelSize1), 0);
        cv::GaussianBlur(b, blurredImg2, cv::Size(blurKernelSize2, blurKernelSize2), 0);

        // Mélanger les deux images floues
        cv::Mat frame;
        cv::addWeighted(blurredImg1, beta, blurredImg2, t, 0.0, frame);
        return frame;

    }
};

// -------------------- Random Growing Circles --------------------
class RandomCirclesTransition : public TransitionBetween {
public:
    RandomCirclesTransition(double d, double f, Easing::Type e, int count, unsigned seed, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), count_(std::max(1,count)), seed_(seed) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows; cv::Mat1b m(H,W, uchar(0));
        std::mt19937 rng(seed_);
        std::uniform_int_distribution<int> dx(0,W-1), dy(0,H-1);
        double maxR = std::hypot(W, H);
        double r = 0.05*maxR + 0.95*maxR*t;
        for (int i=0;i<count_;++i){
            cv::Point c(dx(rng), dy(rng));
            int ri = (int)std::round(r * 0.25);

            //geomertyDraw(c, ri, m) ;
            if (ri>0) cv::circle(m, c, ri, getOpacityMask(t), cv::FILLED);
        }
        double scale = std::pow(t, 1.5);
        if (scale<1.0){
            int k = std::max(0, (int)std::round((1.0-scale) * 6));
            if (k>0){
                cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*k+1,2*k+1));
                cv::erode(m,m,kernel);
            }
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    void geomertyDraw(cv::Point c, int ri,cv::Mat1b& m) {
            if (ri>0) cv::circle(m, c, ri, 255, cv::FILLED);
    }
    int count_; unsigned seed_;
};

// -------------------- Random Growing Circles --------------------
class RandomSquaresTransition : public TransitionBetween {
public:
    RandomSquaresTransition(double d, double f, Easing::Type e, int count, unsigned seed, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), count_(std::max(1,count)), seed_(seed) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows; cv::Mat1b m(H,W, uchar(0));
        std::mt19937 rng(seed_);
        std::uniform_int_distribution<int> dx(0,W-1), dy(0,H-1);
        double maxR = std::hypot(W, H);
        double r = 0.05*maxR + 0.95*maxR*t;
        for (int i=0;i<count_;++i){
            cv::Point c(dx(rng), dy(rng));
            int ri = (int)std::round(r * 0.30);

            if (ri>0) cv::rectangle(m,cv::Point(c.x-ri/2,c.y-(ri/2)), cv::Point(c.x+ri/2,c.y+ri/2), getOpacityMask(t), cv::FILLED);
        }
        double scale = std::pow(t, 1.5);
        if (scale<1.0){
            int k = std::max(0, (int)std::round((1.0-scale) * 6));
            if (k>0){
                cv::Mat kernel = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(2*k+1,2*k+1));
                cv::erode(m,m,kernel);
            }
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    void geomertyDraw(cv::Point c, int ri,cv::Mat1b& m) {
            if (ri>0) cv::circle(m, c, ri, 255, cv::FILLED);
    }
    int count_; unsigned seed_;
};

/*
class RandomSquareTransition : public RandomCirclesTransition {
public:
    RandomSquareTransition(double d, double f, Easing::Type e, int count, unsigned seed, MaskBlurOptions mb = {})
        : RandomCirclesTransition(d,f,e,mb) {}

protected:
    void geomertyDraw(cv::Point c, int ri,cv::Mat1b& m) {
            if (ri>0) cv::rectangle(m, c, cv::Point(c.x+ri,c.y+ri), 255, cv::FILLED);
    }
};
*/

// -------------------- Checkerboard (damier) Transition --------------------
class CheckerboardTransition : public TransitionBetween {
public:
    CheckerboardTransition(double d, double f, Easing::Type e, int rows, int cols, bool stepwise, unsigned seed, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), rows_(std::max(2,rows)), cols_(std::max(2,cols)), stepwise_(stepwise), seed_(seed) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows;
        cv::Mat1b m(H,W, uchar(0));
        int cw = W / cols_, ch = H / rows_;
        std::mt19937 rng(seed_);
        for (int r=0;r<rows_;++r){
            for (int c=0;c<cols_;++c){
                int x0 = c*cw; int y0 = r*ch; int x1 = (c==cols_-1? W : x0+cw); int y1 = (r==rows_-1? H : y0+ch);
                int parity = (r + c) & 1;
                double tt = t;
                if (stepwise_) {
                    if ((tt < 0.5 && parity==0) or (tt >= 0.5 and parity==1)){
                        double local = (tt<0.5)? (tt/0.5) : ((tt-0.5)/0.5);
                        if (local > 0.001) cv::rectangle(m, cv::Rect(x0,y0,x1-x0,y1-y0), 255, cv::FILLED);
                    }
                } else {
                    double local = std::clamp(tt - 0.25*parity, 0.0, 1.0);
                    if (local>0.0) cv::rectangle(m, cv::Rect(x0,y0,x1-x0,y1-y0), 255, cv::FILLED);
                }
            }
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    int rows_, cols_; bool stepwise_; unsigned seed_;
};

// -------------------- Moving Bars with variable speeds --------------------
class MovingBarsTransition : public TransitionBetween {
public:
    // axis: vertical -> bars are vertical, progress left/right; horizontal -> bars are horizontal, progress up/down
    MovingBarsTransition(double d, double f, Easing::Type e, std::string axis, int count, std::string direction,
                         double speedMin, double speedMax, unsigned seed, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), axis_(std::move(axis)), count_(std::max(1,count)), direction_(std::move(direction)),
          speedMin_(std::max(1.0, speedMin)), speedMax_(std::max(speedMin, speedMax)), seed_(seed) {
            initSpeeds();
        }
protected:
    void initSpeeds(){
        std::mt19937 rng(seed_);
        std::uniform_real_distribution<double> dist(speedMin_, speedMax_);
        speeds_.assign(count_, 1.0);
        for (int i=0;i<count_;++i) speeds_[i] = dist(rng);
    }
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows;
        cv::Mat1b m(H,W, uchar(0));

        bool vertical = (axis_.size()? std::tolower(axis_[0])=='v' : true);
        bool dirLeft = (direction_.size()? std::tolower(direction_[0])=='l' : false);
        bool dirRight = (direction_.size()? std::tolower(direction_[0])=='r' : false);
        bool dirTop = (direction_.size()? std::tolower(direction_[0])=='t' : false);
        bool dirBottom = (direction_.size()? std::tolower(direction_[0])=='b' : false);
        std::cout << dirBottom;
        if (vertical) {
            int bandW = std::max(1, W / count_);
            for (int i=0;i<count_;++i){
                bool parity = (i %2) == 0;
                int x0 = i*bandW;
                double tt = std::clamp(t * speeds_[i], 0.0, 1.0);
                cv::Rect r;
                if (dirBottom) {
                    r = cv::Rect(x0, 0, (int)(W/ count_), (int)(tt*H)); 
                    if (r.width>0) {
                        cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
                    }
                } else { // top
                    r=  cv::Rect(x0, 0, (int)(W/ count_),std::min(H, H-(int)(tt*H)));
                    if (r.width>0) {
                        cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
                    }
                }
            }
        } else {
            int bandH = std::max(1, H / count_);
            for (int i=0;i<count_;++i){
                int y0 = i*bandH; int y1 = (i==count_-1? H : y0+bandH);
                double tt = std::clamp(t * speeds_[i], 0.0, 1.0);
                if (dirRight) {
                    cv::Rect r(0, y0, (int)(tt*H), (int)(H/ count_));
                    if (r.height>0) {
                        cv::rectangle(m, r,getOpacityMask(t) , cv::FILLED);
                    }
                } else { // left
                    cv::Rect r(0, y0, (int)std::min(W, W-(int)(tt*W)),(int)(H/ count_) );
                    if (r.height>0) {
                        cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
                    }
                }
            }
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    std::string axis_; int count_; std::string direction_;
    double speedMin_, speedMax_; unsigned seed_; std::vector<double> speeds_;
};

// -------------------- Blinds (with wave/phase) --------------------
class BlindsTransition : public TransitionBetween {
public:
    BlindsTransition(double d, double f, Easing::Type e, std::string axis, int count, std::string direction,
                     double waveAmp = 0.0, double wavePhase = 0.0, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb), axis_(std::move(axis)), count_(std::max(1,count)), direction_(std::move(direction)),
          waveAmp_(std::clamp(waveAmp,0.0,0.49)), wavePhase_(wavePhase) {}
protected:
    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows; cv::Mat1b m(H,W, uchar(0));
        bool vertical = (axis_.size()? std::tolower(axis_[0])=='v' : true);
        auto timeWithWave = [&](int i){
            if (waveAmp_<=0.0) return t;
            double phase = wavePhase_ + 2.0*M_PI * (double)i / std::max(1,count_);
            double offset = waveAmp_ * std::sin(phase);
            return std::clamp(t + offset, 0.0, 1.0);
        };
        if (vertical) {
            int bandW = std::max(1, W / count_);
            bool fromLeft = (direction_.size()? std::tolower(direction_[0])=='l' : true);
            for (int i=0;i<count_;++i){
                int x0 = i*bandW; int x1 = (i==count_-1? W : x0+bandW);
                int w = (int)std::round((x1-x0)*timeWithWave(i));
                if (fromLeft) {
                    cv::Rect r(x0, 0, std::min(w,x1-x0), H); if (r.width>0) cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
                } else {
                    int xs = x1 - w; xs = std::max(xs, x0);
                    cv::Rect r(xs, 0, x1-xs, H); if (r.width>0) cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
                }
            }
        } else {
            int bandH = std::max(1, H / count_);
            bool fromTop = (direction_.size()? std::tolower(direction_[0])=='t' : true);
            for (int i=0;i<count_;++i){
                int y0 = i*bandH; int y1 = (i==count_-1? H : y0+bandH);
                int h = (int)std::round((y1-y0)*timeWithWave(i));
                if (fromTop) {
                    cv::Rect r(0, y0, W, std::min(h,y1-y0)); if (r.height>0) cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
                } else {
                    int ys = y1 - h; ys = std::max(ys, y0);
                    cv::Rect r(0, ys, W, y1-ys); if (r.height>0) cv::rectangle(m, r, getOpacityMask(t), cv::FILLED);
                }
            }
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }
    std::string axis_; int count_; std::string direction_; double waveAmp_; double wavePhase_;
};

// -------------------- Animated Checkerboard Transition --------------------
class AnimatedCheckerboardTransition : public TransitionBetween {
public:
    enum class Order { Row, Col, Diag, InvDiag, Random };

    AnimatedCheckerboardTransition(double d, double f, Easing::Type e, int rows, int cols, Order order,
                                   unsigned seed, MaskBlurOptions mb = {})
        : TransitionBetween(d,f,e,mb),
          rows_(std::max(2,rows)),
          cols_(std::max(2,cols)),
          grid(rows, std::vector<int>(cols)),
          order_(order),
          seed_(seed) {
        initPermutation();
    }
protected:
    void initPermutation(){
        int W = rows_;
        int H = cols_;

        if (order_==Order::Random){
            int value = 0;
            for (int j = 0; j < W; j++) {
                for (int i = 0; i < H; i++) {
                    grid[i][j] = value++;
                }
            }
            std::mt19937 rng(seed_);
            for ( int i=0 ; i < W ; i++)
            {
                std::shuffle(grid[i].begin(), grid[i].end(), rng);
            }
            std::shuffle(grid.begin(), grid.end(), rng);
        }
        if (order_==Order::Col){
            int value = 0;
            for (int j = 0; j < W; j++) {
                for (int i = 0; i < H; i++) {
                    grid[i][j] = value++;
                }
            }
        }
        if (order_==Order::Row){
            int value = 0;
            for (int i = 0; i < H; i++) {
                for (int j = 0; j < W; j++) {
                    grid[i][j] = value++;
                }
            }
        }
        if (order_==Order::Diag) {
            int value = 0;
            for (int sum = 0; sum <= (H + W - 2); sum++) {
                for (int i = 0; i < H; i++) {
                    int j = sum - i;
                    if (j >= 0 && j < W) {
                        grid[i][j] = value++;
                    }
                }
            }
        }
        if (order_==Order::InvDiag) {
            int value = 0;
            for (int sum = 0; sum <= (H + W - 2); sum++) {
                for (int i = 0; i < H; i++) {
                    int j = sum - i;
                    if (j >= 0 && j < W) {
                        grid[(H-1)-i][(W-1)-j] = value++;
                    }
                }
            }
        }
    }

    inline int groupIndex(int r, int c) const {
        switch(order_){
            case Order::Row: return r;
            case Order::Col: return c;
            case Order::Diag: return  (r + c);
            case Order::InvDiag: return ( rows_ + cols_ ) - (r + c) -1;
            case Order::Random: {
                int idx = r*cols_ + c;
                (void)idx;
                return 1; // original code returned constant 1 here
            }
        }
        return r;
    }

    inline int groupCount() const {
        switch(order_){
            case Order::Row: return rows_;
            case Order::Col: return cols_;
            case Order::Diag: return rows_ + cols_ - 1;
            case Order::InvDiag: return rows_ + cols_ - 1;
            case Order::Random: return rows_*cols_;
        }
        return rows_;
    }

    cv::Mat composite(double t, const cv::Mat& a, const cv::Mat& b) override {
        int W=a.cols, H=a.rows;
        cv::Mat1b m(H,W, uchar(0));

        int index = (int)(t * (double)cols_ * (double) rows_);

        int cw = std::max(1, W / cols_);
        int ch = std::max(1, H / rows_);
        int G = std::max(1, groupCount());
        double slice = 1.0 / G;

        for (int r=0;r<rows_;++r){
            for (int c=0;c<cols_;++c){
                int gi = groupIndex(r,c);
                (void)gi;
                double start = gi * slice;
                double local = (t - start) / slice;
                (void)local;

                if ( grid[r][c] <= index ) {
                    int x0 = c*cw;
                    int y0 = r*ch;
                    int x1 = (c==cols_-1? W : x0+cw);
                    int y1 = (r==rows_-1? H : y0+ch);
                    cv::Rect rct(x0, y0, cw , y1-y0);
                    cv::rectangle(m, rct, getOpacityMask(t), cv::FILLED);
                }
            }
        }
        return MaskUtils::blendWithMask(a,b,m, maskBlur());
    }

    int rows_, cols_;
    Order order_;
    unsigned seed_;
    std::vector<std::vector<int>> grid;
};

// -------------------- Factory --------------------
std::unique_ptr<TransitionBetween> createTransitionFromJson(const json& cfg, double defaultFps, const cv::Size& size);

#endif // TRANSITIONS_H

