"""Dual-fisheye panorama stitching (shared core for onboard quick preview and ground high-quality stitching).

Model: equidistant fisheye projection  r = f * theta
Calibration file calibration.json format:
{
  "cam0": {"f": px_focal, "cx": .., "cy": .., "k1..k4": distortion terms,
           "R": 3x3 rotation (relative to panorama frame)},
  "cam1": {...},
  "pano": {"w": 5760, "h": 2880}
}

Pipeline: for each panorama pixel (lon, lat) -> direction vector v -> transform by camera extrinsic R
into camera frame -> FOV check -> fisheye projection + distortion -> sample pixel -> weighted blending in overlap region
"""
import json
import numpy as np
import cv2


def _sph2dir(lon, lat):
    cl = np.cos(lat)
    return np.stack([cl * np.cos(lon), cl * np.sin(lon), np.sin(lat)], axis=-1)


class PanoStitcher:
    def __init__(self, calib_path, out_w=5760, out_h=2880):
        c = json.load(open(calib_path))
        self.cams = [c["cam0"], c["cam1"]]
        self.out_w, self.out_h = out_w, out_h
        self._build_maps()

    def _build_maps(self):
        """Precompute two remap tables + blend weights (once at startup, then table lookup)."""
        w, h = self.out_w, self.out_h
        lon = (np.arange(w) / w - 0.5) * 2 * np.pi          # [-pi, pi]
        lat = (0.5 - np.arange(h) / h) * np.pi              # [pi/2, -pi/2]
        LON, LAT = np.meshgrid(lon, lat)
        V = _sph2dir(LON, LAT)                               # (h,w,3)
        self.maps = []
        self.weights = []
        for cam in self.cams:
            R = np.asarray(cam["R"]).reshape(3, 3)
            Vc = V @ R.T                                     # direction in camera frame
            z = Vc[..., 2]
            theta = np.arccos(np.clip(z, -1, 1))             # off-axis angle
            phi = np.arctan2(Vc[..., 1], Vc[..., 0])
            # Equidistant projection + 4th-order distortion correction
            f, cx, cy = cam["f"], cam["cx"], cam["cy"]
            k1, k2, k3, k4 = (cam.get(f"k{i}", 0.0) for i in range(1, 5))
            th = theta * (1 + k1 * theta**2 + k2 * theta**4
                          + k3 * theta**6 + k4 * theta**8)
            r = f * th
            mx = (cx + r * np.cos(phi)).astype(np.float32)
            my = (cy + r * np.sin(phi)).astype(np.float32)
            # Out-of-FOV mask (FOV 195°, keep 2° margin)
            valid = theta < np.radians(97.5 - 2.0)
            mx[~valid] = -1; my[~valid] = -1
            self.maps.append((mx, my))
            # Blend weight: smaller off-axis angle gets higher weight; smooth overlap transition
            wgt = np.clip((np.radians(97.5) - theta) / np.radians(15), 0, 1)
            wgt[~valid] = 0
            self.weights.append(wgt.astype(np.float32))

    def stitch(self, img0, img1):
        """Stitch two fisheye frames into an equirectangular panorama."""
        acc = np.zeros((self.out_h, self.out_w, 3), np.float32)
        wsum = np.zeros((self.out_h, self.out_w), np.float32)
        for img, (mx, my), wgt in zip((img0, img1), self.maps, self.weights):
            warped = cv2.remap(img, mx, my, cv2.INTER_LINEAR,
                               borderMode=cv2.BORDER_CONSTANT)
            acc += warped.astype(np.float32) * wgt[..., None]
            wsum += wgt
        wsum[wsum < 1e-6] = 1.0
        return (acc / wsum[..., None]).astype(np.uint8)

    def quick_preview(self, img0, img1, width=1024):
        """Quick preview panorama (for downlink): stitch after downscaling."""
        pano = self.stitch(img0, img1)
        return cv2.resize(pano, (width, width // 2))


def stitch_pair(jpg0_bytes, jpg1_bytes, calib_path, out_w=5760, out_h=2880):
    """Convenience function: generate panorama directly from JPEG bytes."""
    i0 = cv2.imdecode(np.frombuffer(jpg0_bytes, np.uint8), cv2.IMREAD_COLOR)
    i1 = cv2.imdecode(np.frombuffer(jpg1_bytes, np.uint8), cv2.IMREAD_COLOR)
    st = PanoStitcher(calib_path, out_w, out_h)
    return st.stitch(i0, i1)
