"""PANO-3U payload configuration (CM4).
All tunable parameters are centralized here; onboard param_table.json can override defaults.
"""
import json, os

DEFAULTS = {
    # Camera
    "cam_resolution": [4056, 3040],
    "cam0_name": "cam0",          # camera 0 (CSI0, 4-lane)
    "cam1_name": "cam1",          # camera 1 (CSI1, 2-lane)
    "ae_exposure_us": 2000,       # default exposure (supports freeze at <=2°/s spin)
    "analogue_gain": 2.0,
    "awb_mode": "auto",
    # Video
    "video_fps": 30,
    "video_bitrate_mbps": 25,     # per-stream H.265
    "video_max_duration_s": 900,
    # Still capture
    "shoot_dng": True,            # store RAW (DNG) + JPEG
    "jpeg_quality": 92,
    # Stitching (onboard quick preview)
    "pano_width": 5760,
    "pano_height": 2880,
    "preview_width": 1024,        # quick preview downlink resolution
    "calib_file": "calibration.json",
    # Storage
    "data_dir": "/mnt/sdcard",
    "disk_reserve_mb": 2048,      # reserved free space
    # Link
    "uart_port": "/dev/ttyAMA1",
    "uart_baud": 115200,
    "heartbeat_timeout_s": 10,    # no PING within timeout -> wait for shutdown command
    # Thermal control
    "temp_min_c": -10,
    "temp_max_c": 55,
}

class Config:
    def __init__(self, path="/opt/pano3u/param_table.json"):
        self.cfg = dict(DEFAULTS)
        if os.path.exists(path):
            try:
                self.cfg.update(json.load(open(path)))
            except Exception:
                pass  # use defaults if parameter table is corrupted (non-fatal)
        self.path = path
    def __getitem__(self, k): return self.cfg[k]
    def set(self, k, v):
        self.cfg[k] = v
        try:
            json.dump(self.cfg, open(self.path, "w"))
        except Exception:
            pass

cfg = Config()
