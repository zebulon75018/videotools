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
    string overlayImage;
    string outputVideo;
    Position position = Position::TOP_LEFT;
    int customX = 0;
    int customY = 0;
    TimeAlign timeAlign = TimeAlign::START;
    int startFrame = 0;
    double startTimestamp = 0.0;
    int duration = -1; // -1 = toute la durée de la vidéo
    Vec3b chromaKey = Vec3b(0, 255, 0); // Vert par défaut
    bool useChromaKey = false;
    int chromaTolerance = 40;
    double overlayScale = 1.0;
    double opacity = 1.0; // 0.0 à 1.0
    bool useAlphaChannel = true; // Utiliser le canal alpha du PNG si disponible
};

void printUsage(const char* progName) {
    cout << "Usage: " << progName << " [options]\n"
         << "\nDescription:\n"
         << "  Fusionne une image sur une vidéo avec support de transparence et positionnement.\n"
         << "\nOptions requises:\n"
         << "  -v, --video <file>         Vidéo principale (requise)\n"
         << "  -i, --image <file>         Image à incruster (requise)\n"
         << "\nOptions de sortie:\n"
         << "  -out, --output <file>      Vidéo de sortie (défaut: output.avi)\n"
         << "\nOptions de positionnement:\n"
         << "  -p, --position <pos>       Position: topleft|topright|bottomleft|bottomright|center|custom\n"
         << "                             (défaut: topleft)\n"
         << "  -x <pixels>                Position X personnalisée (avec --position custom)\n"
         << "  -y <pixels>                Position Y personnalisée (avec --position custom)\n"
         << "\nOptions temporelles:\n"
         << "  -a, --align <align>        Alignement: start|end|frame|timestamp (défaut: start)\n"
         << "  -f, --frame <number>       Frame de début (avec --align frame)\n"
         << "  -ts, --timestamp <sec>     Timestamp de début en secondes (avec --align timestamp)\n"
         << "  -d, --duration <frames>    Durée en frames (-1 = reste de la vidéo)\n"
         << "\nOptions visuelles:\n"
         << "  -s, --scale <float>        Échelle de l'image (défaut: 1.0)\n"
         << "  -op, --opacity <float>     Opacité de l'image: 0.0 (transparent) à 1.0 (opaque)\n"
         << "  -c, --chroma <r,g,b>       Activer chroma key avec couleur RGB (ex: 0,255,0)\n"
         << "  -t, --tolerance <val>      Tolérance du chroma key (défaut: 40)\n"
         << "  --no-alpha                 Ignorer le canal alpha du PNG\n"
         << "\nAutres:\n"
         << "  -h, --help                 Afficher cette aide\n"
         << "\nExemples:\n"
         << "  # Logo en haut à droite, toute la vidéo\n"
         << "  " << progName << " -v video.mp4 -i logo.png -p topright -s 0.3\n"
         << "\n  # Watermark centré avec 50% d'opacité\n"
         << "  " << progName << " -v video.mp4 -i watermark.png -p center -op 0.5\n"
         << "\n  # Image avec fond vert transparent, de 5s à 15s\n"
         << "  " << progName << " -v video.mp4 -i image.jpg -c 0,255,0 -ts 5 -d 300\n"
         << "\n  # Image à position spécifique, apparaît à la frame 100\n"
         << "  " << progName << " -v video.mp4 -i overlay.png -p custom -x 50 -y 100 -f 100\n";
}

bool parseArgs(int argc, char** argv, Config& cfg) {
    if (argc < 2) {
        return false;
    }
    
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return false;
        }
        else if ((arg == "-v" || arg == "--video") && i + 1 < argc) {
            cfg.mainVideo = argv[++i];
        }
        else if ((arg == "-i" || arg == "--image") && i + 1 < argc) {
            cfg.overlayImage = argv[++i];
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
            else {
                cerr << "Position invalide: " << pos << endl;
                return false;
            }
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
            else {
                cerr << "Alignement invalide: " << align << endl;
                return false;
            }
        }
        else if ((arg == "-f" || arg == "--frame") && i + 1 < argc) {
            cfg.startFrame = stoi(argv[++i]);
            cfg.timeAlign = TimeAlign::FRAME;
        }
        else if ((arg == "-ts" || arg == "--timestamp") && i + 1 < argc) {
            cfg.startTimestamp = stod(argv[++i]);
            cfg.timeAlign = TimeAlign::TIMESTAMP;
        }
        else if ((arg == "-d" || arg == "--duration") && i + 1 < argc) {
            cfg.duration = stoi(argv[++i]);
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
            } else {
                cerr << "Format de couleur invalide. Utilisez: r,g,b\n";
                return false;
            }
        }
        else if ((arg == "-t" || arg == "--tolerance") && i + 1 < argc) {
            cfg.chromaTolerance = stoi(argv[++i]);
        }
        else if ((arg == "-s" || arg == "--scale") && i + 1 < argc) {
            cfg.overlayScale = stod(argv[++i]);
            if (cfg.overlayScale <= 0) {
                cerr << "L'échelle doit être positive\n";
                return false;
            }
        }
        else if ((arg == "-op" || arg == "--opacity") && i + 1 < argc) {
            cfg.opacity = stod(argv[++i]);
            if (cfg.opacity < 0.0 || cfg.opacity > 1.0) {
                cerr << "L'opacité doit être entre 0.0 et 1.0\n";
                return false;
            }
        }
        else if (arg == "--no-alpha") {
            cfg.useAlphaChannel = false;
        }
    }
    
    if (cfg.mainVideo.empty() || cfg.overlayImage.empty()) {
        cerr << "Erreur: La vidéo et l'image sont requises!\n\n";
        printUsage(argv[0]);
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

Mat createMaskFromChromaKey(const Mat& image, const Vec3b& chromaKey, int tolerance) {
    Mat mask(image.rows, image.cols, CV_8UC1);
    
    for (int y = 0; y < image.rows; y++) {
        for (int x = 0; x < image.cols; x++) {
            Vec3b pixel = image.at<Vec3b>(y, x);
            
            // Calculer la distance de couleur
            int diff = abs(pixel[0] - chromaKey[0]) + 
                      abs(pixel[1] - chromaKey[1]) + 
                      abs(pixel[2] - chromaKey[2]);
            
            if (diff < tolerance * 3) {
                mask.at<uchar>(y, x) = 0; // Transparent
            } else {
                mask.at<uchar>(y, x) = 255; // Opaque
            }
        }
    }
    
    return mask;
}

Mat extractAlphaChannel(const Mat& image) {
    if (image.channels() == 4) {
        Mat alpha;
        extractChannel(image, alpha, 3);
        return alpha;
    }
    // Si pas de canal alpha, retourner un masque opaque
    return Mat::ones(image.rows, image.cols, CV_8UC1) * 255;
}

void overlayImageWithMask(Mat& background, const Mat& foreground, const Mat& mask, Point position, double opacity) {
    for (int y = 0; y < foreground.rows; y++) {
        for (int x = 0; x < foreground.cols; x++) {
            int bgX = position.x + x;
            int bgY = position.y + y;
            
            if (bgX >= 0 && bgX < background.cols && bgY >= 0 && bgY < background.rows) {
                double alpha = (mask.at<uchar>(y, x) / 255.0) * opacity;
                
                if (alpha > 0) {
                    Vec3b fgPixel = foreground.at<Vec3b>(y, x);
                    Vec3b& bgPixel = background.at<Vec3b>(bgY, bgX);
                    
                    // Blending avec alpha
                    bgPixel[0] = static_cast<uchar>(fgPixel[0] * alpha + bgPixel[0] * (1.0 - alpha));
                    bgPixel[1] = static_cast<uchar>(fgPixel[1] * alpha + bgPixel[1] * (1.0 - alpha));
                    bgPixel[2] = static_cast<uchar>(fgPixel[2] * alpha + bgPixel[2] * (1.0 - alpha));
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
    
    cout << "=== Merge Image to Video ===\n\n";
    
    // Ouvrir la vidéo
    VideoCapture cap(cfg.mainVideo);
    
    if (!cap.isOpened()) {
        cerr << "Erreur: Impossible d'ouvrir la vidéo: " << cfg.mainVideo << endl;
        return 1;
    }
    
    // Récupérer les propriétés de la vidéo
    int videoW = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
    int videoH = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));
    double fps = cap.get(CAP_PROP_FPS);
    int frameCount = static_cast<int>(cap.get(CAP_PROP_FRAME_COUNT));
    
    cout << "Vidéo: " << videoW << "x" << videoH << " @ " << fps << " fps, " 
         << frameCount << " frames\n";
    
    // Charger l'image
    Mat originalImage = imread(cfg.overlayImage, IMREAD_UNCHANGED);
    
    if (originalImage.empty()) {
        cerr << "Erreur: Impossible de charger l'image: " << cfg.overlayImage << endl;
        return 1;
    }
    
    cout << "Image: " << originalImage.cols << "x" << originalImage.rows 
         << ", " << originalImage.channels() << " canaux\n";
    
    // Convertir en BGR si nécessaire et extraire le canal alpha
    Mat imageRGB, imageMask;
    
    if (originalImage.channels() == 4) {
        if (cfg.useAlphaChannel) {
            imageMask = extractAlphaChannel(originalImage);
            cout << "Utilisation du canal alpha de l'image\n";
        } else {
            imageMask = Mat::ones(originalImage.rows, originalImage.cols, CV_8UC1) * 255;
        }
        cvtColor(originalImage, imageRGB, COLOR_BGRA2BGR);
    } else {
        imageRGB = originalImage.clone();
        imageMask = Mat::ones(originalImage.rows, originalImage.cols, CV_8UC1) * 255;
    }
    
    // Redimensionner l'image si nécessaire
    int overlayW = static_cast<int>(imageRGB.cols * cfg.overlayScale);
    int overlayH = static_cast<int>(imageRGB.rows * cfg.overlayScale);
    
    if (cfg.overlayScale != 1.0) {
        resize(imageRGB, imageRGB, Size(overlayW, overlayH));
        resize(imageMask, imageMask, Size(overlayW, overlayH));
    }
    
    cout << "Taille finale de l'image: " << overlayW << "x" << overlayH << "\n";
    
    // Appliquer le chroma key si demandé
    if (cfg.useChromaKey) {
        Mat chromaMask = createMaskFromChromaKey(imageRGB, cfg.chromaKey, cfg.chromaTolerance);
        // Combiner avec le masque existant
        bitwise_and(imageMask, chromaMask, imageMask);
        cout << "Chroma key activé: RGB(" << (int)cfg.chromaKey[2] << "," 
             << (int)cfg.chromaKey[1] << "," << (int)cfg.chromaKey[0] << ")\n";
    }
    
    // Calculer la position
    Point overlayPos = calculatePosition(cfg.position, videoW, videoH, overlayW, overlayH, 
                                        cfg.customX, cfg.customY);
    cout << "Position: (" << overlayPos.x << ", " << overlayPos.y << ")\n";
    
    // Calculer le timing
    int startFrame = 0;
    int endFrame = frameCount;
    
    switch (cfg.timeAlign) {
        case TimeAlign::START:
            startFrame = 0;
            if (cfg.duration > 0) {
                endFrame = min(startFrame + cfg.duration, frameCount);
            }
            cout << "Timing: Début (frames " << startFrame << " à " << endFrame << ")\n";
            break;
            
        case TimeAlign::END:
            if (cfg.duration > 0) {
                startFrame = max(0, frameCount - cfg.duration);
            }
            endFrame = frameCount;
            cout << "Timing: Fin (frames " << startFrame << " à " << endFrame << ")\n";
            break;
            
        case TimeAlign::FRAME:
            startFrame = cfg.startFrame;
            if (startFrame < 0) startFrame = 0;
            if (startFrame > frameCount) startFrame = frameCount;
            
            if (cfg.duration > 0) {
                endFrame = min(startFrame + cfg.duration, frameCount);
            }
            cout << "Timing: Frame " << startFrame << " à " << endFrame << "\n";
            break;
            
        case TimeAlign::TIMESTAMP:
            startFrame = static_cast<int>(cfg.startTimestamp * fps);
            if (startFrame < 0) startFrame = 0;
            if (startFrame > frameCount) startFrame = frameCount;
            
            if (cfg.duration > 0) {
                endFrame = min(startFrame + cfg.duration, frameCount);
            }
            cout << "Timing: " << cfg.startTimestamp << "s (frames " 
                 << startFrame << " à " << endFrame << ")\n";
            break;
    }
    
    if (cfg.opacity < 1.0) {
        cout << "Opacité: " << (cfg.opacity * 100) << "%\n";
    }
    
    // Créer le writer
    VideoWriter writer(cfg.outputVideo, 
                      VideoWriter::fourcc('M','J','P','G'),
                      fps,
                      Size(videoW, videoH));
    
    if (!writer.isOpened()) {
        cerr << "Erreur: Impossible de créer la vidéo de sortie\n";
        return 1;
    }
    
    cout << "\nTraitement en cours...\n";
    
    Mat frame;
    int frameNum = 0;
    
    while (cap.read(frame)) {
        // Appliquer l'overlay si on est dans la fenêtre temporelle
        if (frameNum >= startFrame && frameNum < endFrame) {
            overlayImageWithMask(frame, imageRGB, imageMask, overlayPos, cfg.opacity);
        }
        
        writer.write(frame);
        
        frameNum++;
        if (frameNum % 30 == 0) {
            cout << "Frame " << frameNum << "/" << frameCount 
                 << " (" << (frameNum * 100 / frameCount) << "%)\r" << flush;
        }
    }
    
    cout << "\nTraitement vidéo terminé!\n";
    
    cap.release();
    writer.release();
    
    // Intégrer l'audio avec ffmpeg
    cout << "\nIntégration de l'audio...\n";
    
    string tempVideo = cfg.outputVideo + "_temp.avi";
    string cmdRename = "mv \"" + cfg.outputVideo + "\" \"" + tempVideo + "\"";
    system(cmdRename.c_str());
    
    string cmdFFmpeg = "ffmpeg -i \"" + tempVideo + "\" -i \"" + cfg.mainVideo + 
                       "\" -c:v copy -map 0:v:0 -map 1:a:0? -shortest -y \"" + 
                       cfg.outputVideo + "\" 2>/dev/null";
    
    int result = system(cmdFFmpeg.c_str());
    
    if (result == 0) {
        cout << "✓ Audio intégré avec succès!\n";
        string cmdCleanup = "rm \"" + tempVideo + "\"";
        system(cmdCleanup.c_str());
        cout << "\n✓ Vidéo finale sauvegardée: " << cfg.outputVideo << endl;
    } else {
        cout << "⚠ Impossible d'intégrer l'audio (ffmpeg non disponible)\n";
        cout << "  Vidéo sans audio: " << tempVideo << endl;
        cout << "\n  Commande manuelle:\n";
        cout << "  ffmpeg -i \"" << tempVideo << "\" -i \"" << cfg.mainVideo 
             << "\" -c:v copy -map 0:v:0 -map 1:a:0 -shortest \"" 
             << cfg.outputVideo << "\"\n";
    }
    
    return 0;
}
