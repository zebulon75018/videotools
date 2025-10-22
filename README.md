# 🎬 Video Tools

This project contains tools for manipulating videos, specifically for merging videos together, or with images.

There're three program : one to blend two video , and one to blend an image to a video , and draw subtitle with word speaking.
![merge](https://github.com/zebulon75018/videotools/blob/main/img/bigbunny.png?raw=true) 
+ 
![merge](https://github.com/zebulon75018/videotools/blob/main/img/giphy2.gif?raw=true)  
 = 
![merge](https://github.com/zebulon75018/videotools/blob/main/img/mergeimg2video.png?raw=true)


Subtitle with soeanking  word
![](https://github.com/zebulon75018/videotools/blob/main/img/subtitle.gif?raw=true?raw=true)



## ✨ Features

*   ✅ **Merge 2 videos** (main + overlay)
*   ✅ **Merge a video with an image** (with PNG alpha channel support)
*   ✅ **Temporal Alignment** : `START` (beginning), `END` (end), `FRAME` (specific frame), `TIMESTAMP` (specific time in seconds)
*   ✅ **Overlay Positioning** : `top-left`, `top-right`, `bottom-left`, `bottom-right`, `center`, or custom coordinates (`custom`)
*   ✅ **Chroma Key** : make a specific color transparent (green by default)
*   ✅ **Resizing** of the overlay video or image
*   ✅ **Adjustable tolerance** for chroma key
*   ✅ **Opacity Control** for image overlays (`mergeimagetovideo`)
*   ✅ **Automatic audio integration** from the main video (via `ffmpeg`)
*   ✅ **Draw subtitle** from the main video with a srt and a .json specific file.
  

## 🛠️ Compilation Instructions

### Prerequisites

Make sure you have the following dependencies installed:

**On Ubuntu/Debian:**

```bash
sudo apt-get install cmake build-essential libopencv-dev ffmpeg
```

**On macOS (with Homebrew):**

```bash
brew install cmake opencv ffmpeg
```

**On Windows:**

1.  Install [OpenCV](https://opencv.org/releases/) and configure environment variables.
2.  Download [ffmpeg](https://ffmpeg.org/download.html) and add it to your `PATH`.
3.  Install [CMake](https://cmake.org/download/).
4.  Install a C++ compiler (e.g., MinGW or Visual Studio).

### How to hav specific json file for word speaking hightlight

```
git clone https://github.com/SYSTRAN/faster-whisper
or pip faster-whisper
```
```
python  makejsonsubtitle.py exampleaudio.mp3 sample.json
```


### Compilation

Get the 2 include files: 

https://github.com/saurabhshri/simple-yet-powerful-srt-subtitle-parser-cpp ( srtparser.h )

https://github.com/nlohmann/json (single header: json.hpp)


1.  **Create a `build` folder and navigate into it:**

    ```bash
    mkdir build
    cd build
    ```

2.  **Configure with CMake:**

    ```bash
    cmake ..
    ```

3.  **Compile:**

    ```bash
    cmake --build .
    # Or using make if you have it
    # make
    ```

    After compilation, you will find two executables in the `build/` folder: `video_merger` and `mergeimagetovideo`.


### Integrated Help

You can always get help on command-line options:

```bash
./video_merger --help

Usage: ./video_merger [options]
Options:
  -m, --main <file>          Video principale (requise)
  -o, --overlay <file>       Video d'incrustation (requise)
  -out, --output <file>      Video de sortie (défaut: output.avi)
  -p, --position <pos>       Position: topleft|topright|bottomleft|bottomright|center|custom
  -x <pixels>                Position X personnalisée (avec --position custom)
  -y <pixels>                Position Y personnalisée (avec --position custom)
  -a, --align <align>        Alignement temporel: start|end|frame|timestamp (défaut: start)
  -f, --frame <number>       Frame de début pour l'overlay (avec --align frame)
  -ts, --timestamp <sec>     Timestamp de début en secondes (avec --align timestamp)
  -c, --chroma <r,g,b>       Activer chroma key avec couleur RGB (ex: 0,255,0 pour vert)
  -t, --tolerance <val>      Tolérance du chroma key (défaut: 40)
  -s, --scale <float>        Échelle de la vidéo overlay (défaut: 1.0)
  -h, --help                 Afficher cette aide

Exemples d'alignement temporel:
  start       : Overlay commence au début de la vidéo principale
  end         : Overlay se termine avec la fin de la vidéo principale
  frame       : Overlay commence à une frame spécifique (-f)
  timestamp   : Overlay commence à un timestamp spécifique (-ts)

```


```bash
./mergeimagetovideo --help

Usage: ./mergeimagetovideo [options]

Description:
  Fusionne une image sur une vidéo avec support de transparence et positionnement.

Options requises:
  -v, --video <file>         Vidéo principale (requise)
  -i, --image <file>         Image à incruster (requise)

Options de sortie:
  -out, --output <file>      Vidéo de sortie (défaut: output.avi)

Options de positionnement:
  -p, --position <pos>       Position: topleft|topright|bottomleft|bottomright|center|custom
                             (défaut: topleft)
  -x <pixels>                Position X personnalisée (avec --position custom)
  -y <pixels>                Position Y personnalisée (avec --position custom)

Options temporelles:
  -a, --align <align>        Alignement: start|end|frame|timestamp (défaut: start)
  -f, --frame <number>       Frame de début (avec --align frame)
  -ts, --timestamp <sec>     Timestamp de début en secondes (avec --align timestamp)
  -d, --duration <frames>    Durée en frames (-1 = reste de la vidéo)

Options visuelles:
  -s, --scale <float>        Échelle de l'image (défaut: 1.0)
  -op, --opacity <float>     Opacité de l'image: 0.0 (transparent) à 1.0 (opaque)
  -c, --chroma <r,g,b>       Activer chroma key avec couleur RGB (ex: 0,255,0)
  -t, --tolerance <val>      Tolérance du chroma key (défaut: 40)
  --no-alpha                 Ignorer le canal alpha du PNG

Autres:
  -h, --help                 Afficher cette aide

```

```bash
./mergeimagetovideo --help

Usage: ./mergeimagetovideo [options]

Description:
  Fusionne une image sur une vidéo avec support de transparence et positionnement.

Options requises:
  -v, --video <file>         Vidéo principale (requise)
  -i, --image <file>         Image à incruster (requise)

Options de sortie:
  -out, --output <file>      Vidéo de sortie (défaut: output.avi)

Options de positionnement:
  -p, --position <pos>       Position: topleft|topright|bottomleft|bottomright|center|custom
                             (défaut: topleft)
  -x <pixels>                Position X personnalisée (avec --position custom)
  -y <pixels>                Position Y personnalisée (avec --position custom)

Options temporelles:
  -a, --align <align>        Alignement: start|end|frame|timestamp (défaut: start)
  -f, --frame <number>       Frame de début (avec --align frame)
  -ts, --timestamp <sec>     Timestamp de début en secondes (avec --align timestamp)
  -d, --duration <frames>    Durée en frames (-1 = reste de la vidéo)

Options visuelles:
  -s, --scale <float>        Échelle de l'image (défaut: 1.0)
  -op, --opacity <float>     Opacité de l'image: 0.0 (transparent) à 1.0 (opaque)
  -c, --chroma <r,g,b>       Activer chroma key avec couleur RGB (ex: 0,255,0)
  -t, --tolerance <val>      Tolérance du chroma key (défaut: 40)
  --no-alpha                 Ignorer le canal alpha du PNG

Autres:
  -h, --help                 Afficher cette aide

```

```bash
sage:
  ./videoSubRenderer <input_video> <output_video> <subtitles.(srt|json)>
     [--font-face N] [--font-scale F] [--thickness T]
     [--color B,G,R] [--position top|bottom] [--center 0|1]
     [--margin-x PX] [--margin-y PX] [--line-gap PX]
     [--keep-html 0|1]
     [--outline 0|1] [--outline-thickness T] [--outline-color B,G,R]
     [--bg 0|1] [--bg-color B,G,R] [--bg-alpha A] [--bg-pad-x PX] [--bg-pad-y PX]
     [--safe-pct P]
     [--max-width-pct P]
     [--karaoke 0|1] [--hl-scale F] [--hl-thickness T] [--hl-color B,G,R]

Notes:
  - .srt via srtparser.h ; .json = tableau d'objets {start,end,text,words:[{word,start,end},...]}
  - color/outline-color/bg-color en B,G,R (OpenCV). bg-alpha dans [0..1].
  - safe-pct applique des marges minimales en % (title safe).
  - max-width-pct: largeur max du bloc sous-titres après marges/safe.
```
<img width="777" height="234" alt="image" src="https://github.com/user-attachments/assets/b6a0d1a9-7d15-4bb9-8344-66506803b68e" />

![](https://github.com/zebulon75018/videotools/blob/main/img/subtitle2.png?raw=true)




## 🚀 Usage Examples

### 1. `video_merger` (Video + Video / Image Merging)

This tool is designed to merge one video onto another video, or an image onto a video.

```bash
# Basic example - top-left video overlay
./video_merger -m main_video.mp4 -o overlay.mp4 -out result.avi

# With green chroma key (transparent green background)
./video_merger -m main.mp4 -o green_screen.mp4 -out result.avi -c 0,255,0

# Bottom-right position, end alignment
./video_merger -m main.mp4 -o overlay.mp4 -out result.avi \
  -p bottomright -a end

# Custom position with coordinates
./video_merger -m main.mp4 -o overlay.mp4 -out result.avi \
  -p custom -x 100 -y 50

# With resizing (50% of the original size)
./video_merger -m main.mp4 -o overlay.mp4 -out result.avi \
  -s 0.5 -p topright

# Blue chroma key with high tolerance
./video_merger -m main.mp4 -o blue_screen.mp4 -out result.avi \
  -c 0,0,255 -t 60

# New temporal parameters for overlay:

# 1. By frame (-a frame -f <number>)
# Starts the overlay at a specific frame
./video_merger -m main.mp4 -o overlay.mp4 -a frame -f 150 -out result.avi

# 2. By timestamp (-a timestamp -ts <seconds>)
# Starts the overlay at a precise time in seconds
./video_merger -m main.mp4 -o overlay.mp4 -a timestamp -ts 5.5 -out result.avi

# Start at 1 minute 30 seconds
./video_merger -m main.mp4 -o overlay.mp4 -ts 90 -out result.avi

# 3. At the beginning (-a start - default)
# The overlay starts from the beginning
./video_merger -m main.mp4 -o overlay.mp4 -a start -out result.avi

# 4. At the end (-a end)
# The overlay ends at the same time as the main video
./video_merger -m main.mp4 -o overlay.mp4 -a end -out result.avi

# Complete examples:

# Overlay with green background, starts at 10 seconds, bottom-right
./video_merger -m main.mp4 -o green_screen.mp4 \
  -c 0,255,0 -ts 10 -p bottomright -out result.avi

# Overlay at 50% size, starts at frame 200, custom position
./video_merger -m main.mp4 -o overlay.mp4 \
  -s 0.5 -f 200 -p custom -x 100 -y 50 -out result.avi

# Centered overlay that ends with the main video
./video_merger -m main.mp4 -o overlay.mp4 \
  -a end -p center -out result.avi

# Combine everything: chroma key + timestamp + position
./video_merger -m main.mp4 -o overlay.mp4 \
  -c 0,255,0 -t 50 -ts 15.5 -p topright -s 0.75 -out result.avi

# Audio from the main video is automatically included!
# If ffmpeg is installed: Audio is integrated automatically
# If ffmpeg is not installed: The program will display a command to run manually
```

### 2. `mergeimagetovideo` (Video + Image Merging only)

This application is specifically optimized for merging an image (with or without PNG transparency) onto a video, with precise opacity and duration controls.

```bash
# Logo / Watermark

# PNG with native transparency
./mergeimagetovideo -v video.mp4 -i logo_transparent.png -p topright

# Ignore alpha channel if needed
./mergeimagetovideo -v video.mp4 -i image.png --no-alpha

# Semi-transparent watermark (50%)
./mergeimagetovideo -v video.mp4 -i watermark.png -op 0.5 -p center

# Very subtle logo (20%)
./mergeimagetovideo -v video.mp4 -i logo.png -op 0.2 -p bottomright

# Top-right logo, 30% size
./mergeimagetovideo -v video.mp4 -i logo.png -p topright -s 0.3

# Centered watermark with 30% transparency
./mergeimagetovideo -v video.mp4 -i watermark.png -p center -op 0.3

# Bottom-left logo for the entire video
./mergeimagetovideo -v video.mp4 -i brand.png -p bottomleft -s 0.2


# Temporary Image

# Image from 5s to 15s (300 frames at 30fps)
./mergeimagetovideo -v video.mp4 -i overlay.jpg -ts 5 -d 300 -p center

# Image at frame 100, duration 200 frames
./mergeimagetovideo -v video.mp4 -i image.png -f 100 -d 200

# Image that appears at the end (last 5 seconds)
./mergeimagetovideo -v video.mp4 -i end_screen.jpg -a end -d 150


# Chroma Key

# Image with transparent green background
./mergeimagetovideo -v video.mp4 -i image_green.jpg \
  -c 0,255,0 -t 40 -p center

# Image with blue background, semi-transparent
./mergeimagetovideo -v video.mp4 -i blue_bg.png \
  -c 0,0,255 -op 0.7 -p topright


# Advanced Combinations

# Transparent PNG logo, custom position, 80% opacity
./mergeimagetovideo -v video.mp4 -i logo.png \
  -p custom -x 50 -y 50 -op 0.8 -s 0.5

# Watermark with chroma key + opacity
./mergeimagetovideo -v video.mp4 -i watermark.jpg \
  -c 255,255,255 -t 30 -op 0.4 -p bottomright

# Image that appears gradually (simulated by opacity)
./mergeimagetovideo -v video.mp4 -i overlay.png \
  -ts 10 -d 90 -p center -op 0.6
```

### µ3. `videoSubRenderer` (Video + subtitle (srt) or json )

Fond semi-opaque + outline + safe area 10% :
```
./video_sub in.mp4 out.mp4 subs.srt \
  --bg 1 --bg-color 0,0,0 --bg-alpha 0.45 --bg-pad-x 28 --bg-pad-y 16 \
  --outline 1 --outline-thickness 4 --outline-color 0,0,0 \
  --safe-pct 10 \
  --font-scale 1.1 --thickness 2 --color 255,255,255 --position bottom --center 1
```

Fond seul (barre en haut) :

```
./video_sub in.mp4 out.mp4 subs.srt \
  --position top --center 0 --margin-x 60 --bg 1 --bg-alpha 0.35 --bg-color 0,0,0
```

```
./video_sub in.mp4 out.mp4 subs.srt \
  --bg 1 --bg-color 0,0,0 --bg-alpha 0.4 --bg-pad-x 28 --bg-pad-y 16 \
  --outline 1 --outline-thickness 4 --outline-color 0,0,0 \
  --safe-pct 10 --max-width-pct 85
```



## 📂 Project Structure

```
video-merger/
├── CMakeLists.txt
├── main.cpp                    # Source code for 'video_merger' (video + video/image)
├── mergeimagetovideo.cpp       # Source code for 'mergeimagetovideo' (video + image only)
└── build/
    ├── video_merger            # Executable after compilation
    └── mergeimagetovideo       # Executable after compilation
```

The program displays real-time progress and saves the result in AVI format (MJPEG codec). You can change the codec in the code if needed!

---
