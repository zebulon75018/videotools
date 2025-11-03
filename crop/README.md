 # 🎬 QtVideoCut – Simple Video Segment Editor (Qt + OpenCV + FFmpeg)

**QtVideoCut** is a lightweight C++/Qt application that lets you open a video, preview it, and select a segment using a **double range slider**.  
You can then **save or export** the selected part of the video:
- via **OpenCV** (fast re-encoding with progress bar but without sound)
- or via **FFmpeg** (lossless cut, no re-encoding)

---

## ✨ Features

- 🔹 Open and preview videos (via OpenCV)
- 🔹 Interactive **double range slider** to select a segment
- 🔹 Displays the **current frame** as you move the slider
- 🔹 Shows **In / Out timecodes** in the bottom status bar
- 🔹 📋 **Copy timecodes** to clipboard with one click
- 🔹 Save segment using:
  - 🧩 **OpenCV** – *File → Save segment...* (with progress dialog but without sound)
  - ⚡ **FFmpeg** – *File → Export segment (ffmpeg)...* (direct copy, no re-encode)
- 🔹 Optionally open a video from the command line

---

## 🧱 Project Structure
```
qt-video-cut/
├── CMakeLists.txt
├── main.cpp
├── mainwindow.h
├── mainwindow.cpp
├── range_slider.h
└── README.md
```
yaml
Copier le code

> 💡 `range_slider.h` contains a custom Qt `RangeSlider` widget used for the double-handle timeline.

---

## ⚙️ Requirements

- **Qt 6** (or Qt 5)
- **OpenCV 4.x**
- **FFmpeg** (must be in your `PATH` for export)

---

## 🧩 Build Instructions (CMake)

```bash
git clone https://github.com/<your-username>/QtVideoCut.git
cd QtVideoCut
qmake
make -j
```

## Build and run 🚀

# ▶️ Usage
Open a video
Go to File → Open Video...

Or from command line:
```
bash
Copier le code
./QtVideoCut my_video.mp4
```

Select a segment
Use the RangeSlider:

Left handle → start of segment (In)

Right handle → end of segment (Out)

The current timecodes are shown in the status bar:
```
In: 00:00:02.040 | Out: 00:00:05.120
```
Copy timecodes
Click the 📋 Copy timecodes button in the bottom-right corner.
The text In: ... | Out: ... is placed in your clipboard.

Save a segment
File → Save segment...

Uses OpenCV to re-encode and save the selected frames

Displays a progress bar

File → Export segment (ffmpeg)...

Uses FFmpeg to copy the segment without re-encoding

Command executed:

bash
Copier le code
```
ffmpeg -y -ss <start> -i "<input>" -t <duration> -c copy "<output>"
```
## 🖼️ Example UI

(The RangeSlider lets you easily select the start and end of the clip to export.)

## 🧠 Technical Details
Language: C++17

Framework: Qt Widgets

Video reading: cv::VideoCapture

Video writing: cv::VideoWriter or ffmpeg via system()

Mat → QImage conversion: custom cv::cvtColor handling

Status bar:

Shows “In” and “Out” timecodes

Includes a 📋 copy button for quick clipboard access

Cross-platform: Windows / Linux / macOS

## 🧰 Possible Improvements
 Add real-time playback (Play/Pause)

 Preserve audio when exporting with OpenCV

 Move to a QML / QtQuick UI

 Use QProcess to monitor FFmpeg progress

 Add translations / multi-language support

📄 License
This project is released under the MIT License.
You are free to use, modify, and distribute it.

## 👨‍💻 Author

🔗 https://github.com/zebulon75018

💛 If you find this project useful, consider leaving a ⭐ on GitHub!

