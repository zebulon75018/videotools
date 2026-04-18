Voici un README propre et complet en anglais pour ton projet :

---

# 🎬 Video Segment Colorizer (Gradio + AI Background Removal)

A Python application built with Gradio that allows you to:

* Upload a video
* Select a specific frame
* Automatically remove the background using AI
* Recolor either the foreground or the background
* Insert customizable text **between foreground and background layers**
* Generate a processed video segment

---

## 🚀 Features

### 🎥 Video Processing

* Upload any video file
* Navigate frames using a slider
* Extract and preview individual frames

### 🧠 AI Background Removal

* Uses `rembg` (U²-Net / ISNet models)
* No dependency on `huggingface_hub` or `withoutbg`
* Fully local processing

### 🎨 Selective Colorization

* Apply effects to:

  * Background only
  * Foreground only
* Available styles:

  * Grayscale
  * Boost saturation
  * Warm tone
  * Cool tone

### ✍️ Advanced Text Layer (Key Feature)

Text is rendered **between background and foreground**, meaning:

* It can be naturally occluded by the subject
* It behaves like a real composited layer

#### Text options:

* Custom text content
* Position:

  * Presets (center, top-left, etc.)
  * OR manual X/Y coordinates
* Custom `.ttf` font support
* Font size
* Text color (hex, rgb, or named)
* Opacity control
* Outline (stroke) width + color

### 🖱️ Interactive Placement (Optional)

* With `gradio-image-prompter`, you can:

  * Click on the preview
  * Move a point to position the text visually

---

## 📦 Installation

### 1. Create environment (recommended)

```bash
python -m venv venv
source venv/bin/activate  # Linux / Mac
venv\\Scripts\\activate   # Windows
```

### 2. Install dependencies

```bash
pip install gradio opencv-python pillow numpy rembg onnxruntime
```

### (Optional) For interactive point placement:

```bash
pip install gradio-image-prompter
```

---

## ▶️ Run the App

```bash
python gradio_withoutbg_video_app.py
```

Then open the URL shown in your terminal (usually `http://127.0.0.1:7860`).

---

## 🧩 How It Works

1. A frame is extracted from the video
2. AI generates an alpha mask (foreground vs background)
3. The pipeline builds 3 layers:

   * Background (optionally recolored)
   * Text (optional, customizable)
   * Foreground (original or recolored)
4. Layers are composited using alpha blending
5. The result is written into a video segment

---

## ⚙️ Parameters

### Video

* Frame selection
* Segment duration
* Output FPS
* Frame sampling (performance control)

### Text

* Content
* Position (anchor or X/Y)
* Font (`.ttf` supported)
* Size
* Color
* Opacity
* Outline width & color

---

## ⚠️ Notes

* Processing is CPU-intensive (especially per-frame segmentation)
* `rembg` uses ONNX models → performance depends on your hardware
* Large videos or long durations may take time

---

## 🛠️ Possible Improvements

* Audio preservation in output video
* GPU acceleration
* Better tracking (reuse mask across frames)
* Real drag & drop text layer (via custom JS canvas)
* Export transparency (RGBA video)

---

## 📄 License

This project is for experimentation and prototyping purposes.

---

## 🙌 Credits

* Background removal powered by `rembg`
* UI built with Gradio
* Video processing via OpenCV


