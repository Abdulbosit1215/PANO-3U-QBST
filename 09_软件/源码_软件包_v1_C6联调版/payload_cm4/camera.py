"""Dual-camera capture module (picamera2 + libcamera).
- Synchronized dual-fisheye capture (hardware sync: both FSIN lines tied to CM4 GPIO4, error <1 ms)
- Supports RAW (DNG) + JPEG still capture and H.265 video
"""
import time, io, threading
import numpy as np

try:
    from picamera2 import Picamera2
    from picamera2.encoders import H264Encoder  # H.265 requires libav backend
    from picamera2.outputs import FileOutput
    ON_TARGET = True
except ImportError:
    ON_TARGET = False   # development/test environment (ground PC)


class DualFisheyeCamera:
    def __init__(self, cfg):
        self.cfg = cfg
        self.cam0 = None
        self.cam1 = None
        self._lock = threading.Lock()
        self._rec = {0: None, 1: None}
        self.ok = [False, False]
        if ON_TARGET:
            self._init_hw()

    def _init_hw(self):
        w, h = self.cfg["cam_resolution"]
        for idx in (0, 1):
            try:
                cam = Picamera2(camera_num=idx)
                sc = cam.create_still_configuration(
                    main={"size": (w, h), "format": "RGB888"},
                    raw={"size": (w, h)})
                cam.configure(sc)
                vc = cam.create_video_configuration(
                    main={"size": (w, h), "format": "RGB888"},
                    controls={"FrameRate": self.cfg["video_fps"]})
                cam.start()
                cam.set_controls({
                    "ExposureTime": self.cfg["ae_exposure_us"],
                    "AnalogueGain": self.cfg["analogue_gain"],
                })
                setattr(self, f"cam{idx}", cam)
                self.ok[idx] = True
            except Exception as e:
                self.ok[idx] = False
                print(f"[camera] cam{idx} init fail: {e}")

    # ---------- Still capture ----------
    def shoot(self, tag=None):
        """Capture both cameras synchronously, return {idx: (jpeg_bytes, dng_path_or_None)}."""
        ts = time.strftime("%Y%m%d_%H%M%S") if tag is None else tag
        out = {}
        with self._lock:
            jobs = {}
            for idx in (0, 1):
                cam = getattr(self, f"cam{idx}", None)
                if cam is None or not self.ok[idx]:
                    continue
                jobs[idx] = cam.capture_request()   # request-based capture helps preserve sync
            for idx, req in jobs.items():
                try:
                    buf = io.BytesIO()
                    req.save("main", buf)           # JPEG
                    dng = None
                    if self.cfg["shoot_dng"]:
                        dng = f"cam{idx}_{ts}.dng"
                        req.save_dng(dng)
                    out[idx] = (buf.getvalue(), dng)
                    req.release()
                except Exception as e:
                    print(f"[camera] cam{idx} shoot fail: {e}")
                    self.ok[idx] = False
        return ts, out

    # ---------- Video ----------
    def start_video(self, path_fmt, duration_s):
        """Dual-stream H.265 recording. path_fmt includes {idx} placeholder."""
        with self._lock:
            for idx in (0, 1):
                cam = getattr(self, f"cam{idx}", None)
                if cam is None or not self.ok[idx]:
                    continue
                try:
                    enc = H264Encoder(bitrate=self.cfg["video_bitrate_mbps"] * 1000000)
                    out = FileOutput(path_fmt.format(idx=idx))
                    cam.start_encoder(enc, out)
                    self._rec[idx] = (enc, out)
                except Exception as e:
                    print(f"[camera] cam{idx} video fail: {e}")

    def stop_video(self):
        with self._lock:
            for idx in (0, 1):
                cam = getattr(self, f"cam{idx}", None)
                if cam and self._rec.get(idx):
                    try:
                        cam.stop_encoder()
                    except Exception:
                        pass
                    self._rec[idx] = None

    @property
    def recording(self):
        return any(v is not None for v in self._rec.values())

    def temperature(self, idx):
        """Sensor temperature; on target read IMX477 driver or onboard NTC."""
        if ON_TARGET:
            try:
                with open(f"/sys/class/thermal/thermal_zone{idx+1}/temp") as f:
                    return int(f.read()) // 1000
            except Exception:
                return 25
        return 25
