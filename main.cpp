#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <algorithm>

using namespace cv;
using namespace std;

enum class Position { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, CENTER, CUSTOM };
enum class TimeAlign { START, END, FRAME, TIMESTAMP };

struct Config {
    string mainVideo;
    string overlayVideo;
    string outputVideo;
    Position position = Position::TOP_LEFT;
    int customX = 0;
    int customY = 0;
    TimeAlign timeAlign = TimeAlign::START;
    int startFrame = 0;
    double startTimestamp = 0.0;
    Vec3b chromaKey = Vec3b(0, 255, 0); // Vert par défaut
    bool useChromaKey = false;
    int chromaTolerance = 40;
    double overlayScale = 1.0;
};

void printUsage(const char* progName) {
    cout << "Usage: " << progName << " [options]\n"
         << "Options:\n"
         << "  -m, --main <file>          Video principale (requise)\n"
         << "  -o, --overlay <file>       Video d'incrustation (requise)\n"
         << "  -out, --output <file>      Video de sortie (défaut: output.avi)\n"
         << "  -p, --position <pos>       Position: topleft|topright|bottomleft|bottomright|center|custom\n"
         << "  -x <pixels>                Position X personnalisée (avec --position custom)\n"
         << "  -y <pixels>                Position Y personnalisée (avec --position custom)\n"
         << "  -a, --align <align>        Alignement temporel: start|end|frame|timestamp (défaut: start)\n"
         << "  -f, --frame <number>       Frame de début pour l'overlay (avec --align frame)\n"
         << "  -ts, --timestamp <sec>     Timestamp de début en secondes (avec --align timestamp)\n"
         << "  -c, --chroma <r,g,b>       Activer chroma key avec couleur RGB (ex: 0,255,0 pour vert)\n"
         << "  -t, --tolerance <val>      Tolérance du chroma key (défaut: 40)\n"
         << "  -s, --scale <float>        Échelle de la vidéo overlay (défaut: 1.0)\n"
         << "  -h, --help                 Afficher cette aide\n"
         << "\n"
         << "Exemples d'alignement temporel:\n"
         << "  start       : Overlay commence au début de la vidéo principale\n"
         << "  end         : Overlay se termine avec la fin de la vidéo principale\n"
         << "  frame       : Overlay commence à une frame spécifique (-f)\n"
         << "  timestamp   : Overlay commence à un timestamp spécifique (-ts)\n";
}

bool parseArgs(int argc, char** argv, Config& cfg) {
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }
        else if ((arg == "-m" || arg == "--main") && i + 1 < argc) {
            cfg.mainVideo = argv[++i];
        }
        else if ((arg == "-o" || arg == "--overlay") && i + 1 < argc) {
            cfg.overlayVideo = argv[++i];
        }
        else if ((arg == "-out" || arg == "--output") && i + 1 < argc) {
            cfg.outputVideo = argv[++i];
        }
        else if ((arg == "-p" || arg == "--position") && i + 1 < argc) {
            string pos = argv[++i];
            transform(pos.begin(), pos.end(), pos.begin(), ::tolower);
            if (pos == "topleft") cfg.position = Position::TOP_LEFT;
            else if (pos == "topright") cfg.position = Position::TOP_RIGHT;
            else if (pos == "bottomleft") cfg.position = Position::BOTTOM_LEFT;
            else if (pos == "bottomright") cfg.position = Position::BOTTOM_RIGHT;
            else if (pos == "center") cfg.position = Position::CENTER;
            else if (pos == "custom") cfg.position = Position::CUSTOM;
        }
        else if (arg == "-x" && i + 1 < argc) {
            cfg.customX = stoi(argv[++i]);
        }
        else if (arg == "-y" && i + 1 < argc) {
            cfg.customY = stoi(argv[++i]);
        }
        else if ((arg == "-a" || arg == "--align") && i + 1 < argc) {
            string align = argv[++i];
            transform(align.begin(), align.end(), align.begin(), ::tolower);
            if (align == "start") cfg.timeAlign = TimeAlign::START;
            else if (align == "end") cfg.timeAlign = TimeAlign::END;
            else if (align == "frame") cfg.timeAlign = TimeAlign::FRAME;
            else if (align == "timestamp") cfg.timeAlign = TimeAlign::TIMESTAMP;
        }
        else if ((arg == "-f" || arg == "--frame") && i + 1 < argc) {
            cfg.startFrame = stoi(argv[++i]);
            cfg.timeAlign = TimeAlign::FRAME;
        }
        else if ((arg == "-ts" || arg == "--timestamp") && i + 1 < argc) {
            cfg.startTimestamp = stod(argv[++i]);
            cfg.timeAlign = TimeAlign::TIMESTAMP;
        }
        else if ((arg == "-c" || arg == "--chroma") && i + 1 < argc) {
            cfg.useChromaKey = true;
            string color = argv[++i];
            size_t pos1 = color.find(',');
            size_t pos2 = color.find(',', pos1 + 1);
            if (pos1 != string::npos && pos2 != string::npos) {
                cfg.chromaKey[2] = stoi(color.substr(0, pos1)); // R
                cfg.chromaKey[1] = stoi(color.substr(pos1 + 1, pos2 - pos1 - 1)); // G
                cfg.chromaKey[0] = stoi(color.substr(pos2 + 1)); // B
            }
        }
        else if ((arg == "-t" || arg == "--tolerance") && i + 1 < argc) {
            cfg.chromaTolerance = stoi(argv[++i]);
        }
        else if ((arg == "-s" || arg == "--scale") && i + 1 < argc) {
            cfg.overlayScale = stod(argv[++i]);
        }
    }
    
    if (cfg.mainVideo.empty() || cfg.overlayVideo.empty()) {
        cerr << "Erreur: Les vidéos principale et d'incrustation sont requises!\n";
        return false;
    }
    
    if (cfg.outputVideo.empty()) {
        cfg.outputVideo = "output.avi";
    }
    
    return true;
}

Point calculatePosition(Position pos, int mainW, int mainH, int overlayW, int overlayH, int customX, int customY) {
    switch (pos) {
        case Position::TOP_LEFT:
            return Point(0, 0);
        case Position::TOP_RIGHT:
            return Point(mainW - overlayW, 0);
        case Position::BOTTOM_LEFT:
            return Point(0, mainH - overlayH);
        case Position::BOTTOM_RIGHT:
            return Point(mainW - overlayW, mainH - overlayH);
        case Position::CENTER:
            return Point((mainW - overlayW) / 2, (mainH - overlayH) / 2);
        case Position::CUSTOM:
            return Point(customX, customY);
    }
    return Point(0, 0);
}

Mat applyChromaKey(const Mat& overlay, const Vec3b& chromaKey, int tolerance) {
    Mat result = overlay.clone();
    Mat alpha(overlay.rows, overlay.cols, CV_8UC1);
    
    for (int y = 0; y < overlay.rows; y++) {
        for (int x = 0; x < overlay.cols; x++) {
            Vec3b pixel = overlay.at<Vec3b>(y, x);
            
            // Calculer la distance de couleur
            int diff = abs(pixel[0] - chromaKey[0]) + 
                      abs(pixel[1] - chromaKey[1]) + 
                      abs(pixel[2] - chromaKey[2]);
            
            if (diff < tolerance * 3) {
                alpha.at<uchar>(y, x) = 0; // Transparent
            } else {
                alpha.at<uchar>(y, x) = 255; // Opaque
            }
        }
    }
    
    return alpha;
}

void overlayImage(Mat& background, const Mat& foreground, const Mat& mask, Point position) {
    for (int y = 0; y < foreground.rows; y++) {
        for (int x = 0; x < foreground.cols; x++) {
            int bgX = position.x + x;
            int bgY = position.y + y;
            
            if (bgX >= 0 && bgX < background.cols && bgY >= 0 && bgY < background.rows) {
                if (mask.at<uchar>(y, x) > 0) {
                    background.at<Vec3b>(bgY, bgX) = foreground.at<Vec3b>(y, x);
                }
            }
        }
    }
}

int main(int argc, char** argv) {
    Config cfg;
    
    if (!parseArgs(argc, argv, cfg)) {
        return 1;
    }
    
    // Ouvrir les vidéos
    VideoCapture mainCap(cfg.mainVideo);
    VideoCapture overlayCap(cfg.overlayVideo);
    
    if (!mainCap.isOpened()) {
        cerr << "Erreur: Impossible d'ouvrir la vidéo principale: " << cfg.mainVideo << endl;
        return 1;
    }
    
    if (!overlayCap.isOpened()) {
        cerr << "Erreur: Impossible d'ouvrir la vidéo d'incrustation: " << cfg.overlayVideo << endl;
        return 1;
    }
    
    // Récupérer les propriétés
    int mainW = static_cast<int>(mainCap.get(CAP_PROP_FRAME_WIDTH));
    int mainH = static_cast<int>(mainCap.get(CAP_PROP_FRAME_HEIGHT));
    double mainFps = mainCap.get(CAP_PROP_FPS);
    int mainFrameCount = static_cast<int>(mainCap.get(CAP_PROP_FRAME_COUNT));
    
    int overlayW = static_cast<int>(overlayCap.get(CAP_PROP_FRAME_WIDTH) * cfg.overlayScale);
    int overlayH = static_cast<int>(overlayCap.get(CAP_PROP_FRAME_HEIGHT) * cfg.overlayScale);
    int overlayFrameCount = static_cast<int>(overlayCap.get(CAP_PROP_FRAME_COUNT));
    
    cout << "Vidéo principale: " << mainW << "x" << mainH << " @ " << mainFps << " fps, " 
         << mainFrameCount << " frames\n";
    cout << "Vidéo overlay: " << overlayW << "x" << overlayH << ", " 
         << overlayFrameCount << " frames\n";
    
    // Calculer la position
    Point overlayPos = calculatePosition(cfg.position, mainW, mainH, overlayW, overlayH, 
                                        cfg.customX, cfg.customY);
    cout << "Position d'incrustation: (" << overlayPos.x << ", " << overlayPos.y << ")\n";
    
    // Calculer le décalage temporel
    int overlayStartFrame = 0;
    
    switch (cfg.timeAlign) {
        case TimeAlign::START:
            overlayStartFrame = 0;
            cout << "Alignement: Début de la vidéo\n";
            break;
            
        case TimeAlign::END:
            if (overlayFrameCount < mainFrameCount) {
                overlayStartFrame = mainFrameCount - overlayFrameCount;
            }
            cout << "Alignement: Fin de la vidéo (frame " << overlayStartFrame << ")\n";
            break;
            
        case TimeAlign::FRAME:
            overlayStartFrame = cfg.startFrame;
            if (overlayStartFrame < 0) overlayStartFrame = 0;
            if (overlayStartFrame > mainFrameCount) overlayStartFrame = mainFrameCount;
            cout << "Alignement: Frame spécifique " << overlayStartFrame << "\n";
            break;
            
        case TimeAlign::TIMESTAMP:
            overlayStartFrame = static_cast<int>(cfg.startTimestamp * mainFps);
            if (overlayStartFrame < 0) overlayStartFrame = 0;
            if (overlayStartFrame > mainFrameCount) overlayStartFrame = mainFrameCount;
            cout << "Alignement: Timestamp " << cfg.startTimestamp << "s (frame " 
                 << overlayStartFrame << ")\n";
            break;
    }
    
    // Créer le writer
    VideoWriter writer(cfg.outputVideo, 
                      VideoWriter::fourcc('M','J','P','G'),
                      mainFps,
                      Size(mainW, mainH));
    
    if (!writer.isOpened()) {
        cerr << "Erreur: Impossible de créer la vidéo de sortie\n";
        return 1;
    }
    
    cout << "Traitement en cours...\n";
    
    Mat mainFrame, overlayFrame, overlayResized;
    int frameNum = 0;
    
    while (mainCap.read(mainFrame)) {
        Mat outputFrame = mainFrame.clone();
        
        // Vérifier si on doit afficher l'overlay sur cette frame
        int overlayFrameNum = frameNum - overlayStartFrame;
        
        if (overlayFrameNum >= 0 && overlayFrameNum < overlayFrameCount) {
            // Positionner l'overlay au bon frame
            overlayCap.set(CAP_PROP_POS_FRAMES, overlayFrameNum);
            
            if (overlayCap.read(overlayFrame)) {
                // Redimensionner l'overlay si nécessaire
                if (cfg.overlayScale != 1.0) {
                    resize(overlayFrame, overlayResized, Size(overlayW, overlayH));
                } else {
                    overlayResized = overlayFrame;
                }
                
                if (cfg.useChromaKey) {
                    // Appliquer le chroma key
                    Mat mask = applyChromaKey(overlayResized, cfg.chromaKey, cfg.chromaTolerance);
                    overlayImage(outputFrame, overlayResized, mask, overlayPos);
                } else {
                    // Incrustation simple (sans transparence)
                    Mat mask = Mat::ones(overlayResized.rows, overlayResized.cols, CV_8UC1) * 255;
                    overlayImage(outputFrame, overlayResized, mask, overlayPos);
                }
            }
        }
        
        writer.write(outputFrame);
        
        frameNum++;
        if (frameNum % 30 == 0) {
            cout << "Frame " << frameNum << "/" << mainFrameCount << "\r" << flush;
        }
    }
    
    cout << "\nTraitement terminé! Vidéo sauvegardée: " << cfg.outputVideo << endl;
    
    mainCap.release();
    overlayCap.release();
    writer.release();
    
    return 0;
}
