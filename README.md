# videotools
some stuff on video to merge with another or with a image ... 

Fonctionnalités :

✅ Fusion de 2 vidéos (principale + incrustation)
✅ Alignement temporel : START (début) ou END (fin)
✅ Positionnement : top-left, top-right, bottom-left, bottom-right, center, ou coordonnées personnalisées
✅ Chroma key : rendre une couleur transparente (vert par défaut)
✅ Redimensionnement de la vidéo d'incrustation
✅ Tolérance ajustable pour le chroma key

Instructions de compilation :
Prérequis
bash# Sur Ubuntu/Debian
sudo apt-get install cmake build-essential libopencv-dev

# Sur macOS (avec Homebrew)
brew install cmake opencv

# Sur Windows
# Installer OpenCV et configurer les variables d'environnement
Compilation
bash# Créer un dossier build
mkdir build
cd build

# Configurer avec CMake
cmake ..

# Compiler
cmake --build .

# Ou avec make
make
Exemples d'utilisation :
bash# Exemple basique - overlay en haut à gauche
./video_merger -m video_principale.mp4 -o overlay.mp4 -out resultat.avi

# Avec chroma key vert (fond vert transparent)
./video_merger -m main.mp4 -o green_screen.mp4 -out result.avi -c 0,255,0

# Position en bas à droite, alignement à la fin
./video_merger -m main.mp4 -o overlay.mp4 -out result.avi \
  -p bottomright -a end

# Position personnalisée avec coordonnées
./video_merger -m main.mp4 -o overlay.mp4 -out result.avi \
  -p custom -x 100 -y 50

# Avec redimensionnement (50% de la taille)
./video_merger -m main.mp4 -o overlay.mp4 -out result.avi \
  -s 0.5 -p topright

# Chroma key bleu avec tolérance élevée
./video_merger -m main.mp4 -o blue_screen.mp4 -out result.avi \
  -c 0,0,255 -t 60
Structure du projet :
video-merger/
├── CMakeLists.txt
├── main.cpp
└── build/
    └── video_merger (exécutable après compilation)
Le programme affiche la progression en temps réel et sauvegarde le résultat en format AVI (codec MJPEG). Vous pouvez modifier le codec dans le code si nécessaire !RéessayerClaude n'a pas encore la capacité d'exécuter le code qu'il génère.CEst ce que tu peut ajouter aussi comme parametre ou va etre au niveau temporel ,  incrusté la video secondaire , cela peut etre a debut , ou alors la duree de la video main - la duree de la video secondaire , ou alors grace a un nombre de frame ou a un time stampModifierVideo Merger avec Chroma KeyCode ∙ Version 2 enum class Position { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT, CENTER, CUSTOM };
enum class TimeAlign { START, END, FRAME, TIMESTAMP };

struct Config {
    string mainVideo;
    string overlayVideo;
    string outputVideo;
    Position positiParfait ! J'ai ajouté les nouveaux paramètres d'alignement temporel. Voici les nouvelles possibilités :
Nouveaux paramètres temporels :
1. Par frame (-a frame -f <number>)
Commence l'incrustation à une frame spécifique
bash# Commencer l'overlay à la frame 150
./video_merger -m main.mp4 -o overlay.mp4 -a frame -f 150 -out result.avi
2. Par timestamp (-a timestamp -ts <seconds>)
Commence l'incrustation à un moment précis en secondes
bash# Commencer l'overlay à 5.5 secondes
./video_merger -m main.mp4 -o overlay.mp4 -a timestamp -ts 5.5 -out result.avi

# Commencer à 1 minute 30
./video_merger -m main.mp4 -o overlay.mp4 -ts 90 -out result.avi
3. Au début (-a start - par défaut)
L'overlay commence dès le début
bash./video_merger -m main.mp4 -o overlay.mp4 -a start -out result.avi
4. À la fin (-a end)
L'overlay se termine en même temps que la vidéo principale
bash./video_merger -m main.mp4 -o overlay.mp4 -a end -out result.avi
Exemples complets :
bash# Overlay avec fond vert, commence à 10 secondes, en bas à droite
./video_merger -m main.mp4 -o green_screen.mp4 \
  -c 0,255,0 -ts 10 -p bottomright -out result.avi

# Overlay à 50% de taille, commence à la frame 200, position personnalisée
./video_merger -m main.mp4 -o overlay.mp4 \
  -s 0.5 -f 200 -p custom -x 100 -y 50 -out result.avi

# Overlay centré qui se termine avec la vidéo principale
./video_merger -m main.mp4 -o overlay.mp4 \
  -a end -p center -out result.avi

# Combine tout : chroma key + timestamp + position
./video_merger -m main.mp4 -o overlay.mp4 \
  -c 0,255,0 -t 50 -ts 15.5 -p topright -s 0.75 -out result.avi
Le programme affiche maintenant l'alignement choisi et la frame de début calculée pour plus de clarté ! 🎬RéessayerClaude n'a pas encore la capacité d'exécuter le code qu'il génère.CEst ce qu'il est possible d'avoir le son de la premiere video qui sera dans la video finale ?ModifierVideo Merger avec Chroma KeyCode ∙ Version 6     cout << "\nTraitement vidéo terminé! Vidéo sauvegardée: " << cfg.outputVideo << endl;
    
    mainCap.release();
    overlayCap.release();
    writer.release();
    
    // Extraire et réintégrer l'audio avec ffmpeg
    cout << "\nIntégration deParfait ! J'ai ajouté l'intégration automatique de l'audio de la vidéo principale. Le programme utilise maintenant ffmpeg pour réintégrer l'audio après le traitement vidéo.
Comment ça fonctionne :

OpenCV traite la vidéo (fusion des images)
ffmpeg ajoute l'audio automatiquement depuis la vidéo principale

Prérequis supplémentaire :
bash# Sur Ubuntu/Debian
sudo apt-get install ffmpeg

# Sur macOS
brew install ffmpeg

# Sur Windows
# Télécharger ffmpeg depuis https://ffmpeg.org/ et l'ajouter au PATH
Compilation (mise à jour) :
bash# Dépendances complètes
sudo apt-get install cmake build-essential libopencv-dev ffmpeg

# Compilation (identique)
mkdir build && cd build
cmake ..
make
Utilisation (inchangée) :
bash# L'audio est maintenant automatiquement inclus !
./video_merger -m main.mp4 -o overlay.mp4 -out result.avi

# Avec tous les paramètres
./video_merger -m main.mp4 -o overlay.mp4 \
  -c 0,255,0 -ts 10 -p bottomright -out result.avi
Que se passe-t-il :

✅ Si ffmpeg est installé : Audio intégré automatiquement
⚠️ Si ffmpeg n'est pas installé : Le programme affiche une commande à exécuter manuellement

Le programme conserve la vidéo temporaire et affiche les instructions si ffmpeg n'est pas disponible, vous permettant d'intégrer l'audio plus tard ! 🔊RéessayerClaude n'a pas encore la capacité d'exécuter le code qu'il génère.


Parfait ! J'ai créé une application dédiée mergeimagetovideo spécialement optimisée pour fusionner une image sur une vidéo. 🎨
Nouvelles fonctionnalités spécifiques :
1. Support complet du canal alpha PNG
bash# PNG avec transparence native
./mergeimagetovideo -v video.mp4 -i logo_transparent.png -p topright

# Ignorer le canal alpha si nécessaire
./mergeimagetovideo -v video.mp4 -i image.png --no-alpha
2. Contrôle de l'opacité (-op, --opacity)
bash# Watermark semi-transparent (50%)
./mergeimagetovideo -v video.mp4 -i watermark.png -op 0.5 -p center

# Logo très discret (20%)
./mergeimagetovideo -v video.mp4 -i logo.png -op 0.2 -p bottomright
3. Interface simplifiée et intuitive
Les paramètres sont optimisés pour le cas d'usage image/vidéo
Structure du projet mise à jour :
video-merger/
├── CMakeLists.txt
├── main.cpp                    # video_merger (vidéo + vidéo/image)
├── mergeimagetovideo.cpp       # mergeimagetovideo (vidéo + image uniquement)
└── build/
    ├── video_merger
    └── mergeimagetovideo
Compilation :
bashmkdir build && cd build
cmake ..
make

# Résultat : 2 exécutables
# - video_merger : fusion vidéo/vidéo ou vidéo/image
# - mergeimagetovideo : fusion vidéo/image uniquement (optimisé)
Exemples d'utilisation de mergeimagetovideo :
Logo / Watermark
bash# Logo en haut à droite, 30% de la taille
./mergeimagetovideo -v video.mp4 -i logo.png -p topright -s 0.3

# Watermark centré avec transparence 30%
./mergeimagetovideo -v video.mp4 -i watermark.png -p center -op 0.3

# Logo en bas à gauche toute la vidéo
./mergeimagetovideo -v video.mp4 -i brand.png -p bottomleft -s 0.2
Image temporaire
bash# Image de 5s à 15s (300 frames à 30fps)
./mergeimagetovideo -v video.mp4 -i overlay.jpg -ts 5 -d 300 -p center

# Image à la frame 100, durée 200 frames
./mergeimagetovideo -v video.mp4 -i image.png -f 100 -d 200

# Image qui apparaît à la fin (dernières 5 secondes)
./mergeimagetovideo -v video.mp4 -i end_screen.jpg -a end -d 150
Chroma key
bash# Image avec fond vert transparent
./mergeimagetovideo -v video.mp4 -i image_green.jpg \
  -c 0,255,0 -t 40 -p center

# Image avec fond bleu, semi-transparente
./mergeimagetovideo -v video.mp4 -i blue_bg.png \
  -c 0,0,255 -op 0.7 -p topright
Combinaisons avancées
bash# Logo PNG transparent, position personnalisée, opacité 80%
./mergeimagetovideo -v video.mp4 -i logo.png \
  -p custom -x 50 -y 50 -op 0.8 -s 0.5

# Watermark avec chroma key + opacité
./mergeimagetovideo -v video.mp4 -i watermark.jpg \
  -c 255,255,255 -t 30 -op 0.4 -p bottomright

# Image qui apparaît progressivement (simulé par opacité)
./mergeimagetovideo -v video.mp4 -i overlay.png \
  -ts 10 -d 90 -p center -op 0.6
Avantages de mergeimagetovideo vs video_merger :
Fonctionnalitémergeimagetovideovideo_mergerSupport PNG alpha✅ Optimisé✅ BasiqueContrôle opacité✅ 0.0 à 1.0❌Performance⚡ Plus rapide (pas de décodage vidéo overlay)StandardInterface🎯 Dédiée images🔀 GénériqueBlending alpha✅ Avancé✅ Simple
Aide intégrée :
bash./mergeimagetovideo --help
# Affiche une aide complète avec exemples
Les deux applications sont maintenant disponibles avec des cas d'usage complémentaires ! 🚀RéessayerClaude n'a pas encore la capacité d'exécuter le code qu'il génère.
