#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <memory>

// ---- SRT parser (header-only) ----
// https://github.com/saurabhshri/simple-yet-powerful-srt-subtitle-parser-cpp
#include "srtparser.h"

// ---- JSON (header-only: nlohmann/json) ----
// https://github.com/nlohmann/json (single header: json.hpp)
#include "json.hpp"
using json = nlohmann::json;

// ========================= Options / CLI =========================
struct Options {
    std::string inVideo;
    std::string outVideo;
    std::string subPath; // .srt ou .json (auto-détection via extension)

    // style texte "normal"
    int fontFace = cv::FONT_HERSHEY_SIMPLEX;
    double fontScale = 1.0;
    int thickness = 2;
    cv::Scalar color = cv::Scalar(255,255,255); // BGR

    // positionnement
    std::string position = "bottom"; // "top" | "bottom"
    bool center = true;              // centrage horizontal
    int marginX = 30;                // px
    int marginY = 30;                // px
    int lineGap = 8;                 // px
    bool keepHTML = false;

    // Outline
    bool outline = false;
    int outlineThickness = 3;
    cv::Scalar outlineColor = cv::Scalar(0,0,0);

    // Fond semi-opaque
    bool bg = false;
    cv::Scalar bgColor = cv::Scalar(0,0,0);
    double bgAlpha = 0.5; // 0..1
    int bgPadX = 20;
    int bgPadY = 12;

    // Safe area (marge mini en % de largeur/hauteur)
    double safePct = 0.0;

    // Word-wrap
    double maxWidthPct = 90.0; // % de la largeur utilisable

    // Karaoké (.json uniquement)
    bool karaoke = true;
    double hlScale = 1.3;                 // taille mot actif
    int hlThickness = 3;                  // épaisseur mot actif
    cv::Scalar hlColor = cv::Scalar(0,255,255); // BGR
};

static bool parseBGR(const std::string& s, cv::Scalar& out) {
    int b,g,r; char c1, c2;
    std::istringstream iss(s);
    if ((iss >> b >> c1 >> g >> c2 >> r) && c1==',' && c2==',') { out=cv::Scalar(b,g,r); return true; }
    return false;
}

static bool endsWithNoCase(const std::string& s, const std::string& suf) {
    if (s.size() < suf.size()) return false;
    return std::equal(suf.rbegin(), suf.rend(), s.rbegin(),
                      [](char a, char b){ return std::tolower(a)==std::tolower(b); });
}

static void printUsage(const char* prog) {
    std::cout <<
"Usage:\n"
"  " << prog << " <input_video> <output_video> <subtitles.(srt|json)>\n"
"     [--font-face N] [--font-scale F] [--thickness T]\n"
"     [--color B,G,R] [--position top|bottom] [--center 0|1]\n"
"     [--margin-x PX] [--margin-y PX] [--line-gap PX]\n"
"     [--keep-html 0|1]\n"
"     [--outline 0|1] [--outline-thickness T] [--outline-color B,G,R]\n"
"     [--bg 0|1] [--bg-color B,G,R] [--bg-alpha A] [--bg-pad-x PX] [--bg-pad-y PX]\n"
"     [--safe-pct P]\n"
"     [--max-width-pct P]\n"
"     [--karaoke 0|1] [--hl-scale F] [--hl-thickness T] [--hl-color B,G,R]\n"
"\nNotes:\n"
"  - .srt via srtparser.h ; .json = tableau d'objets {start,end,text,words:[{word,start,end},...]}\n"
"  - color/outline-color/bg-color en B,G,R (OpenCV). bg-alpha dans [0..1].\n"
"  - safe-pct applique des marges minimales en % (title safe).\n"
"  - max-width-pct: largeur max du bloc sous-titres après marges/safe.\n"
<< std::endl;
}

static Options parseArgs(int argc, char** argv) {
    Options o;
    if (argc < 4) { printUsage(argv[0]); std::exit(1); }
    o.inVideo = argv[1];
    o.outVideo = argv[2];
    o.subPath = argv[3];

    for (int i=4;i<argc;++i) {
        std::string a = argv[i];
        auto need = [&](bool cond, const std::string& name){ if(!cond){ std::cerr<<"Argument manquant après "<<name<<"\n"; std::exit(2);} };
        auto getD = [&](double& dst){ need(i+1<argc,a); dst = std::stod(argv[++i]); };
        auto getI = [&](int& dst){ need(i+1<argc,a); dst = std::stoi(argv[++i]); };
        auto getS = [&](std::string& dst){ need(i+1<argc,a); dst = argv[++i]; };

        if (a=="--font-face") getI(o.fontFace);
        else if (a=="--font-scale") getD(o.fontScale);
        else if (a=="--thickness") getI(o.thickness);
        else if (a=="--color") { need(i+1<argc,a); if(!parseBGR(argv[++i], o.color)){ std::cerr<<"Couleur invalide\n"; std::exit(2);} }
        else if (a=="--position") { getS(o.position); std::transform(o.position.begin(),o.position.end(),o.position.begin(),::tolower); }
        else if (a=="--center") { int v; getI(v); o.center=(v!=0); }
        else if (a=="--margin-x") getI(o.marginX);
        else if (a=="--margin-y") getI(o.marginY);
        else if (a=="--line-gap") getI(o.lineGap);
        else if (a=="--keep-html") { int v; getI(v); o.keepHTML=(v!=0); }

        else if (a=="--outline") { int v; getI(v); o.outline=(v!=0); }
        else if (a=="--outline-thickness") getI(o.outlineThickness);
        else if (a=="--outline-color") { need(i+1<argc,a); if(!parseBGR(argv[++i], o.outlineColor)){ std::cerr<<"Couleur invalide\n"; std::exit(2);} }

        else if (a=="--bg") { int v; getI(v); o.bg=(v!=0); }
        else if (a=="--bg-color") { need(i+1<argc,a); if(!parseBGR(argv[++i], o.bgColor)){ std::cerr<<"Couleur invalide\n"; std::exit(2);} }
        else if (a=="--bg-alpha") getD(o.bgAlpha);
        else if (a=="--bg-pad-x") getI(o.bgPadX);
        else if (a=="--bg-pad-y") getI(o.bgPadY);

        else if (a=="--safe-pct") getD(o.safePct);
        else if (a=="--max-width-pct") getD(o.maxWidthPct);

        else if (a=="--karaoke") { int v; getI(v); o.karaoke=(v!=0); }
        else if (a=="--hl-scale") getD(o.hlScale);
        else if (a=="--hl-thickness") getI(o.hlThickness);
        else if (a=="--hl-color") { need(i+1<argc,a); if(!parseBGR(argv[++i], o.hlColor)){ std::cerr<<"Couleur invalide\n"; std::exit(2);} }

        else { std::cerr<<"Option inconnue: "<<a<<"\n"; printUsage(argv[0]); std::exit(2); }
    }
    o.bgAlpha = std::max(0.0, std::min(1.0, o.bgAlpha));
    o.safePct = std::max(0.0, o.safePct);
    o.maxWidthPct = std::max(10.0, std::min(100.0, o.maxWidthPct));
    o.hlScale = std::max(1.0, o.hlScale);
    if (o.hlThickness <= 0) o.hlThickness = o.thickness + 1;
    return o;
}

// ========================= Helpers wrap SRT =========================
static std::vector<std::string> splitParagraphs(const std::string& text) {
    std::vector<std::string> paras;
    std::string cur;
    for (char c : text) {
        if (c=='\r') continue;
        if (c=='\n') { paras.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    paras.push_back(cur);
    return paras;
}

static void hardWrapWord(const std::string& word, int maxW, int fontFace, double fontScale, int thickness,
                         std::vector<std::string>& outLines)
{
    if (word.empty()) return;
    std::string cur;
    for (char ch : word) {
        std::string test = cur + ch;
        int bl=0; auto sz = cv::getTextSize(test, fontFace, fontScale, thickness, &bl);
        if (sz.width > maxW && !cur.empty()) { outLines.push_back(cur); cur.clear(); }
        cur.push_back(ch);
    }
    if (!cur.empty()) outLines.push_back(cur);
}

static std::vector<std::string> wrapText(const std::string& para,
                                         int maxW, int fontFace, double fontScale, int thickness)
{
    std::vector<std::string> lines;
    if (para.empty()) { lines.emplace_back(""); return lines; }
    std::vector<std::string> words;
    { std::istringstream iss(para); std::string w; while (iss >> w) words.push_back(w); }
    std::string cur;
    for (size_t i=0;i<words.size();++i) {
        const std::string& w = words[i];
        std::string candidate = cur.empty()? w : (cur + " " + w);
        int bl=0; auto sz = cv::getTextSize(candidate, fontFace, fontScale, thickness, &bl);
        if (sz.width <= maxW) {
            cur = candidate;
        } else {
            if (!cur.empty()) { lines.push_back(cur); cur.clear(); }
            int blw=0; auto wsz = cv::getTextSize(w, fontFace, fontScale, thickness, &blw);
            if (wsz.width > maxW) {
                std::vector<std::string> chunks;
                hardWrapWord(w, maxW, fontFace, fontScale, thickness, chunks);
                for (size_t k=0;k+1<chunks.size();++k) lines.push_back(chunks[k]);
                if (!chunks.empty()) cur = chunks.back();
            } else {
                cur = w;
            }
        }
    }
    if (!cur.empty()) lines.push_back(cur);
    return lines;
}

// ========================= Modèle JSON karaoké =========================
struct JWord { std::string token; double start{0}, end{0}; };
struct JSeg  { double start{0}, end{0}; std::string text; std::vector<JWord> words; };

static bool loadJsonSubs(const std::string& path, std::vector<JSeg>& out) {
    std::ifstream f(path);
    if (!f.is_open()) return false;
    json j; f >> j;
    if (!j.is_array()) return false;
    out.clear(); out.reserve(j.size());
    for (auto& it : j) {
        JSeg s;
        s.start = it.value("start", 0.0);
        s.end   = it.value("end", 0.0);
        s.text  = it.value("text", "");
        if (it.contains("words") && it["words"].is_array()) {
            for (auto& w : it["words"]) {
                JWord jw;
                jw.token = w.value("word", "");
                jw.start = w.value("start", 0.0);
                jw.end   = w.value("end", 0.0);
                s.words.push_back(std::move(jw));
            }
        }
        out.push_back(std::move(s));
    }
    std::sort(out.begin(), out.end(), [](const JSeg& a, const JSeg& b){ return a.start < b.start; });
    return true;
}

// wrap au niveau des tokens JSON (les tokens contiennent souvent l'espace de tête)
static std::vector<std::vector<int>> wrapJsonWords(const std::vector<JWord>& words,
                                                   int maxW, int fontFace, double fontScale, int thickness)
{
    std::vector<std::vector<int>> lines;
    std::vector<int> curIdx;
    std::string curText;
    int bl=0;
    for (int i=0;i<(int)words.size();++i) {
        const std::string& tok = words[i].token;
        std::string test = curText + tok;
        auto sz = cv::getTextSize(test, fontFace, fontScale, thickness, &bl);
        if (!curIdx.empty() && sz.width > maxW) {
            lines.push_back(curIdx);
            curIdx.clear();
            curText.clear();
        }
        curIdx.push_back(i);
        curText += tok;
    }
    if (!curIdx.empty()) lines.push_back(curIdx);
    if (lines.empty()) lines.push_back({});
    return lines;
}

int main(int argc, char** argv)
{
    Options opt = parseArgs(argc, argv);

    // ---------- ouverture vidéo ----------
    cv::VideoCapture cap(opt.inVideo);
    if (!cap.isOpened()) { std::cerr<<"Impossible d'ouvrir la vidéo: "<<opt.inVideo<<"\n"; return 1; }

    const int width  = (int)cap.get(cv::CAP_PROP_FRAME_WIDTH);
    const int height = (int)cap.get(cv::CAP_PROP_FRAME_HEIGHT);
    double fps = cap.get(cv::CAP_PROP_FPS);
    if (fps <= 0.0) { fps = 25.0; std::cerr<<"Avertissement: FPS non disponible, utilisation de 25 fps.\n"; }

    cv::VideoWriter writer(opt.outVideo, cv::VideoWriter::fourcc('m','p','4','v'), fps, cv::Size(width, height));
    if (!writer.isOpened()) { std::cerr<<"Impossible de créer la vidéo de sortie: "<<opt.outVideo<<"\n"; return 1; }

    // ---------- charge sous-titres ----------
    const bool isJSON = endsWithNoCase(opt.subPath, ".json");
    std::vector<JSeg> jsegs;
    std::unique_ptr<SubtitleParserFactory> factory;
    std::unique_ptr<SubtitleParser> parser;
    std::vector<SubtitleItem*> srtSubs;

    if (isJSON) {
        if (!loadJsonSubs(opt.subPath, jsegs)) {
            std::cerr<<"Échec lecture JSON: "<<opt.subPath<<"\n";
            return 1;
        }
    } else {
        factory.reset(new SubtitleParserFactory(opt.subPath));
        parser.reset(factory->getParser());
        srtSubs = parser->getSubtitles();
        std::sort(srtSubs.begin(), srtSubs.end(), [](const SubtitleItem* a, const SubtitleItem* b){
            return a->getStartTime() < b->getStartTime();
        });
    }

    size_t idx = 0;
    cv::Mat frame;
    long long frameIndex = 0;

    while (cap.read(frame)) {
        long long t_ms = static_cast<long long>((frameIndex * 1000.0) / fps);
        double t_s = t_ms / 1000.0;

        // marges effectives (safe area)
        int effMarginX = opt.marginX, effMarginY = opt.marginY;
        if (opt.safePct > 0.0) {
            int safeX = (int)std::round(width  * (opt.safePct / 100.0));
            int safeY = (int)std::round(height * (opt.safePct / 100.0));
            effMarginX = std::max(effMarginX, safeX);
            effMarginY = std::max(effMarginY, safeY);
        }
        int usableWidth = width - 2*effMarginX;
        int maxLineWidth = (int)std::round(usableWidth * (opt.maxWidthPct / 100.0));
        maxLineWidth = std::max(10, std::min(usableWidth, maxLineWidth));

        if (isJSON) {
            // -------- JSON (karaoké) --------
            if (!jsegs.empty()) {
                while (idx + 1 < jsegs.size() && jsegs[idx].end < t_s) idx++;
                int active = -1;
                int lo = std::max(0, (int)idx - 1);
                int hi = std::min((int)jsegs.size() - 1, (int)idx + 1);
                for (int k = lo; k <= hi; ++k) {
                    if (t_s >= jsegs[k].start && t_s <= jsegs[k].end) { active = k; break; }
                }

                if (active >= 0) {
                    const auto& seg = jsegs[active];

                    // Si pas de mots -> rendu bloc classique sur seg.text
                    if (seg.words.empty()) {
                        // wrap du texte
                        std::vector<std::string> lines;
                        for (const auto& p : splitParagraphs(seg.text)) {
                            auto wlines = wrapText(p, maxLineWidth, opt.fontFace, opt.fontScale, opt.thickness);
                            if (!lines.empty()) lines.emplace_back("");
                            lines.insert(lines.end(), wlines.begin(), wlines.end());
                        }
                        while (!lines.empty() && lines.front().empty()) lines.erase(lines.begin());
                        while (!lines.empty() && lines.back().empty())  lines.pop_back();
                        if (lines.empty()) lines.emplace_back("");

                        int totalH=0, maxW=0, bl=0;
                        std::vector<cv::Size> sizes;
                        for (auto& L : lines) {
                            auto sz = cv::getTextSize(L, opt.fontFace, opt.fontScale, opt.thickness, &bl);
                            sizes.push_back(sz);
                            maxW = std::max(maxW, sz.width);
                            totalH += sz.height + bl;
                        }
                        if (!lines.empty()) totalH += opt.lineGap * ((int)lines.size() - 1);

                        int blockX = opt.center ? (width - maxW) / 2 : effMarginX;
                        int blockY = (opt.position=="top") ? effMarginY : std::max(0, height - effMarginY - totalH);

                        if (opt.bg) {
                            cv::Mat overlay = frame.clone();
                            cv::Rect rect(std::max(0, blockX - opt.bgPadX), std::max(0, blockY - opt.bgPadY),
                                          std::min(width  - std::max(0, blockX - opt.bgPadX),  maxW + 2*opt.bgPadX),
                                          std::min(height - std::max(0, blockY - opt.bgPadY), totalH + 2*opt.bgPadY));
                            cv::rectangle(overlay, rect, opt.bgColor, cv::FILLED, cv::LINE_AA);
                            cv::addWeighted(overlay, opt.bgAlpha, frame, 1.0 - opt.bgAlpha, 0.0, frame);
                        }

                        int yCursor = blockY;
                        for (size_t i=0;i<lines.size();++i) {
                            int blL=0; auto sz = cv::getTextSize(lines[i], opt.fontFace, opt.fontScale, opt.thickness, &blL);
                            int x = opt.center ? (width - sz.width)/2 : blockX;
                            int y = yCursor + sz.height + blL;
                            if (opt.outline)
                                cv::putText(frame, lines[i], {x,y}, opt.fontFace, opt.fontScale, opt.outlineColor,
                                            std::max(opt.outlineThickness, opt.thickness+1), cv::LINE_AA);
                            cv::putText(frame, lines[i], {x,y}, opt.fontFace, opt.fontScale, opt.color, opt.thickness, cv::LINE_AA);
                            yCursor += sz.height + blL + opt.lineGap;
                        }
                    } else {
                        // wrap par tokens
                        auto linesIdx = wrapJsonWords(seg.words, maxLineWidth, opt.fontFace, opt.fontScale, opt.thickness);

                        // Largeurs réelles (au temps t) + hauteurs par ligne
                        std::vector<int> lineW_real(linesIdx.size(), 0);
                        std::vector<int> lineH(linesIdx.size(), 0);
                        std::vector<int> lineBase(linesIdx.size(), 0);
                        for (size_t li=0; li<linesIdx.size(); ++li) {
                            int wsum = 0, maxH=0, maxBase=0;
                            for (int wi : linesIdx[li]) {
                                bool activeWord = opt.karaoke && (t_s >= seg.words[wi].start && t_s <= seg.words[wi].end);
                                double sc = activeWord ? opt.hlScale : opt.fontScale;
                                int bl=0;
                                auto sz = cv::getTextSize(seg.words[wi].token, opt.fontFace, sc, opt.thickness, &bl);
                                wsum += sz.width;
                                maxH = std::max(maxH, sz.height);
                                maxBase = std::max(maxBase, bl);
                            }
                            if (maxH==0) { int bl=0; auto sz = cv::getTextSize(" ", opt.fontFace, opt.fontScale, opt.thickness, &bl); maxH=sz.height; maxBase=bl; }
                            lineW_real[li] = wsum;
                            lineH[li] = maxH;
                            lineBase[li] = maxBase;
                        }
                        int maxW = 0, totalH = 0;
                        for (size_t li=0; li<linesIdx.size(); ++li) {
                            maxW = std::max(maxW, lineW_real[li]);
                            totalH += lineH[li] + lineBase[li];
                        }
                        if (!linesIdx.empty()) totalH += opt.lineGap * ((int)linesIdx.size() - 1);

                        int blockX = opt.center ? (width - maxW) / 2 : effMarginX;
                        int blockY = (opt.position=="top") ? effMarginY : std::max(0, height - effMarginY - totalH);

                        if (opt.bg) {
                            cv::Mat overlay = frame.clone();
                            cv::Rect rect(std::max(0, blockX - opt.bgPadX), std::max(0, blockY - opt.bgPadY),
                                          std::min(width  - std::max(0, blockX - opt.bgPadX),  maxW + 2*opt.bgPadX),
                                          std::min(height - std::max(0, blockY - opt.bgPadY), totalH + 2*opt.bgPadY));
                            cv::rectangle(overlay, rect, opt.bgColor, cv::FILLED, cv::LINE_AA);
                            cv::addWeighted(overlay, opt.bgAlpha, frame, 1.0 - opt.bgAlpha, 0.0, frame);
                        }

                        // dessin mot à mot (avance = largeur RÉELLE au frame)
                        int yCursor = blockY;
                        for (size_t li=0; li<linesIdx.size(); ++li) {
                            int xStart = opt.center ? (width - lineW_real[li]) / 2 : blockX;
                            int yBase  = yCursor + lineH[li] + lineBase[li];

                            int xCur = xStart;
                            for (int wi : linesIdx[li]) {
                                const std::string& tok = seg.words[wi].token;
                                bool activeWord = opt.karaoke && (t_s >= seg.words[wi].start && t_s <= seg.words[wi].end);

                                double sc  = activeWord ? opt.hlScale     : opt.fontScale;
                                int thick  = activeWord ? opt.hlThickness : opt.thickness;
                                cv::Scalar col = activeWord ? opt.hlColor  : opt.color;

                                int bl=0;
                                auto sz = cv::getTextSize(tok, opt.fontFace, sc, opt.thickness, &bl); // largeur réelle

                                if (opt.outline) {
                                    cv::putText(frame, tok, {xCur, yBase}, opt.fontFace, sc, opt.outlineColor,
                                                std::max(opt.outlineThickness, thick+1), cv::LINE_AA);
                                }
                                cv::putText(frame, tok, {xCur, yBase}, opt.fontFace, sc, col, thick, cv::LINE_AA);

                                xCur += sz.width;
                            }
                            yCursor += lineH[li] + lineBase[li] + opt.lineGap;
                        }
                    }
                }
            }
        } else {
            // -------- SRT classique --------
            if (!srtSubs.empty()) {
                while (idx + 1 < srtSubs.size() && srtSubs[idx]->getEndTime() < t_ms) idx++;
                SubtitleItem* active = nullptr;
                size_t lo = (idx > 0 ? idx - 1 : 0);
                size_t hi_excl = std::min(idx + 2, srtSubs.size());
                for (size_t k=lo; k<hi_excl; ++k) {
                    if (t_ms >= srtSubs[k]->getStartTime() && t_ms <= srtSubs[k]->getEndTime()) { active = srtSubs[k]; break; }
                }
                if (active) {
                    std::string raw = opt.keepHTML ? active->getDialogue(true, true, true)
                                                   : active->getDialogue();

                    std::vector<std::string> lines;
                    for (const auto& p : splitParagraphs(raw)) {
                        auto wlines = wrapText(p, maxLineWidth, opt.fontFace, opt.fontScale, opt.thickness);
                        if (!lines.empty()) lines.emplace_back("");
                        lines.insert(lines.end(), wlines.begin(), wlines.end());
                    }
                    while (!lines.empty() && lines.front().empty()) lines.erase(lines.begin());
                    while (!lines.empty() && lines.back().empty())  lines.pop_back();
                    if (lines.empty()) lines.emplace_back("");

                    int totalH=0, maxW=0, bl=0;
                    for (auto& L : lines) {
                        auto sz = cv::getTextSize(L, opt.fontFace, opt.fontScale, opt.thickness, &bl);
                        maxW = std::max(maxW, sz.width);
                        totalH += sz.height + bl;
                    }
                    if (!lines.empty()) totalH += opt.lineGap * ((int)lines.size()-1);

                    int blockX = opt.center ? (width - maxW) / 2 : effMarginX;
                    int blockY = (opt.position=="top") ? effMarginY : std::max(0, height - effMarginY - totalH);

                    if (opt.bg) {
                        cv::Mat overlay = frame.clone();
                        cv::Rect rect(std::max(0, blockX - opt.bgPadX), std::max(0, blockY - opt.bgPadY),
                                      std::min(width  - std::max(0, blockX - opt.bgPadX),  maxW + 2*opt.bgPadX),
                                      std::min(height - std::max(0, blockY - opt.bgPadY), totalH + 2*opt.bgPadY));
                        cv::rectangle(overlay, rect, opt.bgColor, cv::FILLED, cv::LINE_AA);
                        cv::addWeighted(overlay, opt.bgAlpha, frame, 1.0 - opt.bgAlpha, 0.0, frame);
                    }

                    int yCursor = blockY;
                    for (size_t i=0;i<lines.size();++i) {
                        int blL=0; auto sz = cv::getTextSize(lines[i], opt.fontFace, opt.fontScale, opt.thickness, &blL);
                        int x = opt.center ? (width - sz.width)/2 : blockX;
                        int y = yCursor + sz.height + blL;
                        if (opt.outline)
                            cv::putText(frame, lines[i], {x,y}, opt.fontFace, opt.fontScale, opt.outlineColor,
                                        std::max(opt.outlineThickness, opt.thickness+1), cv::LINE_AA);
                        cv::putText(frame, lines[i], {x,y}, opt.fontFace, opt.fontScale, opt.color, opt.thickness, cv::LINE_AA);
                        yCursor += sz.height + blL + opt.lineGap;
                    }
                }
            }
        }

        writer.write(frame);
        frameIndex++;
    }

    std::cout << "Terminé : " << opt.outVideo << std::endl;
    return 0;
}

