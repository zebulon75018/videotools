import os
import tempfile
from dataclasses import dataclass
from typing import Optional, Tuple

import cv2
import gradio as gr
import numpy as np
from PIL import Image, ImageColor, ImageDraw, ImageFont

try:
    from withoutbg import WithoutBG
except ImportError:
    WithoutBG = None


APP_TITLE = "withoutBG Video Segment Colorizer"


@dataclass
class AppState:
    video_path: Optional[str] = None
    fps: float = 25.0
    frame_count: int = 0
    width: int = 0
    height: int = 0
    model_ready: bool = False


_STATE = AppState()
_MODEL = None


def ensure_model(mode: str, api_key: str = ""):
    """Lazy model initialization.

    Supported modes:
    - opensource: local Focus model
    - api: withoutBG Pro API
    """
    global _MODEL, _STATE

    if WithoutBG is None:
        raise RuntimeError(
            "Le package 'withoutbg' n'est pas installé. Lancez: pip install withoutbg"
        )

    if _MODEL is not None:
        return _MODEL

    if mode == "api":
        if not api_key.strip():
            raise RuntimeError("Un api_key withoutBG est requis en mode API.")
        _MODEL = WithoutBG.api(api_key=api_key.strip())
    else:
        _MODEL = WithoutBG.opensource()

    _STATE.model_ready = True
    return _MODEL


def reset_model():
    global _MODEL, _STATE
    _MODEL = None
    _STATE.model_ready = False


def get_video_info(video_path: str) -> Tuple[float, int, int, int]:
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise RuntimeError("Impossible d'ouvrir la vidéo.")

    fps = cap.get(cv2.CAP_PROP_FPS) or 25.0
    frame_count = int(cap.get(cv2.CAP_PROP_FRAME_COUNT) or 0)
    width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH) or 0)
    height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT) or 0)
    cap.release()

    if frame_count <= 0:
        raise RuntimeError("Impossible de lire le nombre de frames de la vidéo.")

    return fps, frame_count, width, height


def extract_frame(video_path: str, frame_index: int) -> np.ndarray:
    cap = cv2.VideoCapture(video_path)
    if not cap.isOpened():
        raise RuntimeError("Impossible d'ouvrir la vidéo.")

    cap.set(cv2.CAP_PROP_POS_FRAMES, int(frame_index))
    ok, frame_bgr = cap.read()
    cap.release()
    if not ok or frame_bgr is None:
        raise RuntimeError(f"Impossible de lire la frame {frame_index}.")

    return cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)


def video_uploaded(video_file):
    if video_file is None:
        return (
            gr.update(maximum=1, value=0, interactive=False),
            None,
            "Charge une vidéo pour commencer.",
            gr.update(value=0),
            gr.update(value="0.00 s"),
        )

    video_path = video_file
    fps, frame_count, width, height = get_video_info(video_path)

    _STATE.video_path = video_path
    _STATE.fps = fps
    _STATE.frame_count = frame_count
    _STATE.width = width
    _STATE.height = height

    first_frame = extract_frame(video_path, 0)
    duration = frame_count / fps if fps > 0 else 0.0

    info = (
        f"Vidéo chargée: {frame_count} frames | {fps:.2f} fps | "
        f"{width}x{height} | durée: {duration:.2f} s"
    )

    return (
        gr.update(maximum=max(frame_count - 1, 1), value=0, interactive=True),
        first_frame,
        info,
        gr.update(value=0),
        gr.update(value="0.00 s"),
    )


def on_slider_change(frame_index: int):
    if not _STATE.video_path:
        return None, gr.update(value="Charge d'abord une vidéo."), gr.update(value=0)

    frame = extract_frame(_STATE.video_path, frame_index)
    seconds = frame_index / _STATE.fps if _STATE.fps else 0.0
    return frame, gr.update(value=f"{seconds:.2f} s"), gr.update(value=frame_index)


def pil_rgba_from_rgb(frame_rgb: np.ndarray) -> Image.Image:
    return Image.fromarray(frame_rgb).convert("RGB")


def rgba_to_numpy(image_rgba: Image.Image) -> np.ndarray:
    return np.array(image_rgba.convert("RGBA"))


def make_mask_with_withoutbg(frame_rgb: np.ndarray, model_mode: str, api_key: str):
    model = ensure_model(model_mode, api_key)
    pil_img = pil_rgba_from_rgb(frame_rgb)
    result_rgba = model.remove_background(pil_img)
    rgba = rgba_to_numpy(result_rgba)
    alpha = rgba[:, :, 3]
    return rgba, alpha


def colorize_region(region_rgb: np.ndarray, mode: str = "grayscale") -> np.ndarray:
    if mode == "grayscale":
        gray = cv2.cvtColor(region_rgb, cv2.COLOR_RGB2GRAY)
        return np.stack([gray, gray, gray], axis=-1)

    if mode == "boost_saturation":
        hsv = cv2.cvtColor(region_rgb, cv2.COLOR_RGB2HSV).astype(np.float32)
        hsv[:, :, 1] = np.clip(hsv[:, :, 1] * 1.8, 0, 255)
        hsv[:, :, 2] = np.clip(hsv[:, :, 2] * 1.05, 0, 255)
        return cv2.cvtColor(hsv.astype(np.uint8), cv2.COLOR_HSV2RGB)

    if mode == "warm":
        out = region_rgb.astype(np.float32).copy()
        out[:, :, 0] *= 1.10
        out[:, :, 1] *= 1.02
        out[:, :, 2] *= 0.88
        return np.clip(out, 0, 255).astype(np.uint8)

    if mode == "cool":
        out = region_rgb.astype(np.float32).copy()
        out[:, :, 0] *= 0.90
        out[:, :, 1] *= 1.00
        out[:, :, 2] *= 1.15
        return np.clip(out, 0, 255).astype(np.uint8)


    if mode == "neon":
        out = region_rgb.astype(np.float32).copy()
        out[:, :, 0] *= 57.0/ 255.0
        out[:, :, 1] *= 1.00
        out[:, :, 2] *= 20.0/ 255.0
        return np.clip(out, 0, 255).astype(np.uint8)

    if mode == "neon2":
        out = region_rgb.astype(np.float32).copy()
        out[:, :, 0] *= 20.0/ 255.0
        out[:, :, 1] *= 1.00
        out[:, :, 2] *= 57.0/ 255.0
        return np.clip(out, 0, 255).astype(np.uint8)

    if mode == "neon3":
        out = region_rgb.astype(np.float32).copy()
        out[:, :, 0] *= 20.0/ 255.0
        out[:, :, 1] *= 57.0/ 255.0
        out[:, :, 2] *= 1.00
        return np.clip(out, 0, 255).astype(np.uint8)

    return region_rgb


def safe_parse_color(color_text: str):
    try:
        return ImageColor.getrgb(color_text.strip() or "white")
    except Exception:
        return (255, 255, 255)


def load_font(font_name: str, font_size: int):
    font_candidates = {
        "default": [],
        "dejavu_sans": ["DejaVuSans.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"],
        "dejavu_serif": ["DejaVuSerif.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSerif.ttf"],
        "dejavu_mono": ["DejaVuSansMono.ttf", "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"],
        "arial": ["Arial.ttf", "arial.ttf"],
    }

    if font_name == "default":
        return ImageFont.load_default()

    for candidate in font_candidates.get(font_name, []):
        try:
            return ImageFont.truetype(candidate, font_size)
        except Exception:
            continue

    return ImageFont.load_default()


def compute_text_position(frame_width: int, frame_height: int, text_w: int, text_h: int, anchor: str, margin_x: int, margin_y: int):
    positions = {
        "top_left": (margin_x, margin_y),
        "top_center": ((frame_width - text_w) // 2, margin_y),
        "top_right": (frame_width - text_w - margin_x, margin_y),
        "center_left": (margin_x, (frame_height - text_h) // 2),
        "center": ((frame_width - text_w) // 2, (frame_height - text_h) // 2),
        "center_right": (frame_width - text_w - margin_x, (frame_height - text_h) // 2),
        "bottom_left": (margin_x, frame_height - text_h - margin_y),
        "bottom_center": ((frame_width - text_w) // 2, frame_height - text_h - margin_y),
        "bottom_right": (frame_width - text_w - margin_x, frame_height - text_h - margin_y),
    }
    x, y = positions.get(anchor, positions["center"])
    return max(0, x), max(0, y)


def draw_interstitial_text(
    frame_rgb: np.ndarray,
    text_value: str,
    text_anchor: str,
    text_color: str,
    font_size: int,
    font_name: str,
    margin_x: int,
    margin_y: int,
    text_opacity: float = 1.0,
    outline_width: int = 0,
    outline_color: str = "black",

) -> np.ndarray:
    if not text_value or not text_value.strip():
        return frame_rgb.copy()

    base = Image.fromarray(frame_rgb).convert("RGBA")
    overlay = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    font = load_font(font_name, max(8, int(font_size)))
    color_rgb = safe_parse_color(text_color)
    outline_rgb = safe_parse_color(outline_color)
    alpha_value = int(max(0.0, min(1.0, float(text_opacity))) * 255)


    bbox = draw.multiline_textbbox((0, 0), text_value, font=font, spacing=4)
    text_w = max(1, bbox[2] - bbox[0])
    text_h = max(1, bbox[3] - bbox[1])
    x, y = compute_text_position(base.width, base.height, text_w, text_h, text_anchor, margin_x, margin_y)

    draw.multiline_text(
        (x, y),
        text_value,
        font=font,
        fill=(*color_rgb, alpha_value),
        spacing=4,
        stroke_width=max(0, int(outline_width)),
        stroke_fill=(*outline_rgb, alpha_value),
    )
    merged = Image.alpha_composite(base, overlay).convert("RGB")
    return np.array(merged)


def compose_effect(
    frame_rgb: np.ndarray,
    alpha: np.ndarray,
    target: str,
    color_mode: str,
    text_value: str = "",
    text_anchor: str = "center",
    text_color: str = "white",
    font_size: int = 48,
    font_name: str = "dejavu_sans",
    margin_x: int = 30,
    margin_y: int = 30,
    text_opacity: float = 1.0,
    outline_width: int = 0,
    outline_color: str = "black",
  
) -> np.ndarray:
    alpha_f = (alpha.astype(np.float32) / 255.0)[..., None]
    fg_original = frame_rgb.astype(np.float32)
    recolored = colorize_region(frame_rgb, mode=color_mode).astype(np.float32)

    if target == "background":
        middle_layer = recolored.astype(np.uint8)
        middle_layer = draw_interstitial_text(
            middle_layer,
            text_value=text_value,
            text_anchor=text_anchor,
            text_color=text_color,
            font_size=font_size,
            font_name=font_name,
            margin_x=margin_x,
            margin_y=margin_y,
            text_opacity=text_opacity,
            outline_width=outline_width,
            outline_color=outline_color,
         
        ).astype(np.float32)
        composed = fg_original * alpha_f + middle_layer * (1.0 - alpha_f)
    else:
        middle_layer = frame_rgb.copy()
        middle_layer = draw_interstitial_text(
            middle_layer,
            text_value=text_value,
            text_anchor=text_anchor,
            text_color=text_color,
            font_size=font_size,
            font_name=font_name,
            margin_x=margin_x,
            margin_y=margin_y,
            text_opacity=text_opacity,
            outline_width=outline_width,
            outline_color=outline_color,
     
        ).astype(np.float32)
        composed = recolored * alpha_f + middle_layer * (1.0 - alpha_f)

    return np.clip(composed, 0, 255).astype(np.uint8)


def preview_processed_frame(
    frame_index: int,
    model_mode: str,
    api_key: str,
    target: str,
    color_mode: str,
    text_value: str,
    text_anchor: str,
    text_color: str,
    font_size: int,
    font_name: str,
    margin_x: int,
    margin_y: int,
    text_opacity: float,
    outline_width: int,
    outline_color: str,
   
):
    if not _STATE.video_path:
        raise gr.Error("Charge une vidéo d'abord.")

    frame_rgb = extract_frame(_STATE.video_path, frame_index)
    rgba, alpha = make_mask_with_withoutbg(frame_rgb, model_mode, api_key)
    preview = compose_effect(
        frame_rgb,
        alpha,
        target,
        color_mode,
        text_value=text_value,
        text_anchor=text_anchor,
        text_color=text_color,
        font_size=font_size,
        font_name=font_name,
        margin_x=margin_x,
        margin_y=margin_y,
        text_opacity=text_opacity,
        outline_width=outline_width,
        outline_color=outline_color,
      
    )

    mask_preview = np.stack([alpha, alpha, alpha], axis=-1)
    cutout_preview = rgba[:, :, :3]

    return preview, mask_preview, cutout_preview


def process_video_segment(
    model_mode: str,
    api_key: str,
    start_frame: int,
    duration_seconds: float,
    target: str,
    color_mode: str,
    text_value: str,
    text_anchor: str,
    text_color: str,
    font_size: int,
    font_name: str,
    margin_x: int,
    margin_y: int,
    text_opacity: float,
    outline_width: int,
    outline_color: str,
  
    sample_every_n: int,
    output_fps: float,
    progress=gr.Progress(track_tqdm=False),
):
    if not _STATE.video_path:
        raise gr.Error("Charge une vidéo d'abord.")

    if duration_seconds <= 0:
        raise gr.Error("La durée doit être > 0.")

    if sample_every_n < 1:
        sample_every_n = 1

    cap = cv2.VideoCapture(_STATE.video_path)
    if not cap.isOpened():
        raise gr.Error("Impossible d'ouvrir la vidéo source.")

    source_fps = _STATE.fps or 25.0
    output_fps = output_fps if output_fps > 0 else source_fps

    total_frames = max(1, int(round(duration_seconds * source_fps)))
    end_frame = min(_STATE.frame_count, start_frame + total_frames)
    frame_indices = list(range(start_frame, end_frame, sample_every_n))
    if not frame_indices:
        cap.release()
        raise gr.Error("Aucune frame à traiter pour cette plage.")

    model = ensure_model(model_mode, api_key)

    tmp_dir = tempfile.mkdtemp(prefix="withoutbg_video_")
    out_path = os.path.join(tmp_dir, "processed_segment.mp4")

    fourcc = cv2.VideoWriter_fourcc(*"mp4v")
    writer = cv2.VideoWriter(out_path, fourcc, output_fps, (_STATE.width, _STATE.height))
    if not writer.isOpened():
        cap.release()
        raise gr.Error("Impossible d'initialiser l'écriture du MP4.")

    try:
        for idx, frame_id in enumerate(frame_indices):
            progress((idx, len(frame_indices)), desc=f"Traitement frame {frame_id}")
            cap.set(cv2.CAP_PROP_POS_FRAMES, int(frame_id))
            ok, frame_bgr = cap.read()
            if not ok or frame_bgr is None:
                continue

            frame_rgb = cv2.cvtColor(frame_bgr, cv2.COLOR_BGR2RGB)
            pil_img = pil_rgba_from_rgb(frame_rgb)
            rgba = model.remove_background(pil_img)
            rgba_np = rgba_to_numpy(rgba)
            alpha = rgba_np[:, :, 3]
            processed_rgb = compose_effect(
                frame_rgb,
                alpha,
                target,
                color_mode,
                text_value=text_value,
                text_anchor=text_anchor,
                text_color=text_color,
                font_size=font_size,
                font_name=font_name,
                margin_x=margin_x,
                margin_y=margin_y,
                text_opacity=text_opacity,
                outline_width=outline_width,
                outline_color=outline_color,              
            )
            writer.write(cv2.cvtColor(processed_rgb, cv2.COLOR_RGB2BGR))
    finally:
        cap.release()
        writer.release()

    actual_duration = len(frame_indices) / output_fps if output_fps > 0 else 0.0
    summary = (
        f"Séquence générée: {len(frame_indices)} frames | début frame {start_frame} | "
        f"durée approx: {actual_duration:.2f} s | cible recolorisée: {target}"
    )
    return out_path, summary


def build_ui():
    with gr.Blocks(title=APP_TITLE) as demo:
        gr.Markdown(
            """
# withoutBG Video Segment Colorizer

Charge une vidéo, choisis une frame de départ, prévisualise le masque généré par `withoutbg`,
puis exporte une séquence où **le background** ou **le foreground** est recolorisé.
            """
        )

        with gr.Row():
            with gr.Column(scale=1):
                video_input = gr.Video(label="Vidéo source")
                load_info = gr.Textbox(label="Infos vidéo", interactive=False)

                model_mode = gr.Radio(
                    choices=["opensource", "api"],
                    value="opensource",
                    label="Mode withoutBG",
                    info="opensource = local, api = withoutBG Pro",
                )
                api_key = gr.Textbox(
                    label="API key withoutBG (si mode api)",
                    type="password",
                    placeholder="sk_...",
                )

                target = gr.Radio(
                    choices=["background", "foreground"],
                    value="background",
                    label="Zone à recoloriser",
                )
                color_mode = gr.Dropdown(
                    choices=["neutral","grayscale","neon","neon2","neon3","boost_saturation", "warm", "cool"],
                    value="grayscale",
                    label="Style de colorisation",
                )

                gr.Markdown("### Texte entre background et foreground")
                text_value = gr.Textbox(
                    label="Texte à dessiner",
                    placeholder="Laisse vide pour ne rien dessiner",
                    lines=3,
                )
                text_anchor = gr.Dropdown(
                    choices=[
                        "top_left", "top_center", "top_right",
                        "center_left", "center", "center_right",
                        "bottom_left", "bottom_center", "bottom_right",
                    ],
                    value="center",
                    label="Position du texte",
                )
                text_color = gr.Textbox(
                    label="Couleur du texte",
                    value="white",
                    placeholder="Ex: white, #FFCC00, rgb(255,0,0)",
                )
                text_opacity = gr.Slider(
                    minimum=0.0,
                    maximum=1.0,
                    value=1.0,
                    step=0.01,
                    label="Opacité du texte",
                )
                font_size = gr.Slider(
                    minimum=8,
                    maximum=200,
                    value=48,
                    step=1,
                    label="Taille de police",
                )
                font_name = gr.Dropdown(
                    choices=["dejavu_sans", "dejavu_serif", "dejavu_mono", "arial", "default"],
                    value="dejavu_sans",
                    label="Police",
                )
                
                with gr.Row():
                    outline_width = gr.Slider(minimum=0, maximum=20, value=2, step=1, label="Bordure / outline")
                    outline_color = gr.Textbox(label="Couleur bordure", value="black")
                with gr.Row():
                    margin_x = gr.Slider(minimum=0, maximum=300, value=30, step=1, label="Marge X")
                    margin_y = gr.Slider(minimum=0, maximum=300, value=30, step=1, label="Marge Y")

                    duration_seconds = gr.Slider(
                    minimum=0.5,
                    maximum=15,
                    value=3.0,
                    step=0.5,
                    label="Durée de la séquence à générer (s)",
                )
                sample_every_n = gr.Slider(
                    minimum=1,
                    maximum=5,
                    value=1,
                    step=1,
                    label="Traiter 1 frame sur N",
                    info="Augmente N pour réduire le temps de calcul.",
                )
                output_fps = gr.Number(label="FPS vidéo de sortie", value=25.0, precision=2)

                with gr.Row():
                    preview_btn = gr.Button("Prévisualiser la frame traitée", variant="secondary")
                    #render_btn = gr.Button("Générer la séquence vidéo", variant="primary")

                status = gr.Textbox(label="Statut", interactive=False)

            with gr.Column(scale=1):
                frame_preview = gr.Image(label="Frame sélectionnée", type="numpy")
                frame_slider = gr.Slider(
                    minimum=0,
                    maximum=1,
                    value=0,
                    step=1,
                    label="Frame de départ",
                    interactive=False,
                )
                timestamp = gr.Textbox(label="Position temporelle", value="0.00 s", interactive=False)
                selected_frame = gr.Number(label="Frame sélectionnée", value=0, precision=0)


                processed_preview = gr.Image(label="Preview composité", type="numpy")
                mask_preview = gr.Image(label="Masque alpha", type="numpy")
                cutout_preview = gr.Image(label="Découpe foreground (RGB)", type="numpy")
                output_video = gr.Video(label="Séquence exportée")

        video_input.change(
            fn=video_uploaded,
            inputs=[video_input],
            outputs=[frame_slider, frame_preview, load_info, selected_frame, timestamp],
        )

        frame_slider.change(
            fn=on_slider_change,
            inputs=[frame_slider],
            outputs=[frame_preview, timestamp, selected_frame],
        )

        preview_btn.click(
            fn=preview_processed_frame,
            inputs=[
                frame_slider,
                model_mode,
                api_key,
                target,
                color_mode,
                text_value,
                text_anchor,
                text_color,
                font_size,
                font_name,
                margin_x,
                margin_y,
                text_opacity,
                outline_width,
                outline_color,
            
            ],
            outputs=[processed_preview, mask_preview, cutout_preview],
        )

        """
        render_btn.click(
            fn=process_video_segment,
            inputs=[
                model_mode,
                api_key,
                frame_slider,
                duration_seconds,
                target,
                color_mode,
                text_value,
                text_anchor,
                text_color,
                font_size,
                font_name,
                margin_x,
                margin_y,
                text_opacity,
                outline_width,
                outline_color,
                sample_every_n,
                output_fps,
            ],
            outputs=[output_video, status],
        )
        """

    return demo


def main():
    demo = build_ui()
    demo.launch()


if __name__ == "__main__":
    main()
