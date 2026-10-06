"""Payload software unit tests (runs on ground PC, no camera hardware required).
Run: python3 -m pytest tests/ -v
"""
import os, sys, struct, tempfile
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
import link as L
from storage import StorageManager


class FakeCfg(dict):
    def __getattr__(self, k): return self[k]
    def __setattr__(self, k, v): self[k] = v


# ---------- Link protocol ----------
def test_frame_roundtrip():
    got = []
    dec = L.Decoder(lambda c, p: got.append((c, p)))
    payload = bytes(range(200))            # includes 0x7E/0x7D bytes that require escaping
    frame = L.encode(L.CMD_SHOOT, payload)
    assert frame[0] == 0x7E and frame[-1] == 0x7E
    assert b"\x7e" not in frame[1:-1]      # escaping is effective
    dec.feed(frame)
    assert got == [(L.CMD_SHOOT, payload)]

def test_frame_crc_reject():
    got = []
    dec = L.Decoder(lambda c, p: got.append((c, p)))
    frame = bytearray(L.encode(0x07, b"\x01\x02"))
    frame[3] ^= 0xFF                        # corrupt payload
    dec.feed(bytes(frame))
    assert got == []                        # CRC-invalid frame must be dropped

def test_frame_stream_split():
    """Frames split into arbitrary chunks should still reassemble correctly."""
    got = []
    dec = L.Decoder(lambda c, p: got.append((c, p)))
    frame = L.encode(0x01) + L.encode(0x03, b"\x05\x00\x0a")
    for i in range(0, len(frame), 3):
        dec.feed(frame[i:i + 3])
    assert len(got) == 2

# ---------- Storage ----------
def test_storage_chunk_roundtrip(tmp_path):
    cfg = FakeCfg()
    cfg.data_dir = str(tmp_path); cfg["disk_reserve_mb"] = 0
    sm = StorageManager(cfg)
    data = os.urandom(5000)
    fp = tmp_path / "t.bin"; fp.write_bytes(data)
    fid = sm.register(str(fp), "test")
    info = sm.prep_file(fid)
    assert info["total"] == 25
    blob = b"".join(sm.read_chunk(fid, i) for i in range(25))
    assert blob == data
    import zlib
    assert info["crc32"] == zlib.crc32(data) & 0xFFFFFFFF

def test_storage_index_survives_bad_json(tmp_path):
    cfg = FakeCfg()
    cfg.data_dir = str(tmp_path); cfg["disk_reserve_mb"] = 0
    (tmp_path / "index.json").write_text("{corrupted!!")
    sm = StorageManager(cfg)               # should not crash, auto-rebuild index
    assert sm.index["next_id"] == 1
    assert list(tmp_path.glob("*.bad.*"))  # corrupted file is backed up

# ---------- Stitching ----------
def test_stitch_synthetic(tmp_path):
    """Synthesize two hemisphere gradients and verify stitched output size and continuity."""
    import numpy as np, cv2, json
    from stitch import PanoStitcher
    H, W = 3040, 4056
    f = (H / 2) / np.radians(97.5)
    calib = {"cam0": {"f": f, "cx": W/2, "cy": H/2, "R": [1,0,0,0,1,0,0,0,1]},
             "cam1": {"f": f, "cx": W/2, "cy": H/2, "R": [-1,0,0,0,1,0,0,0,-1]}}
    cf = tmp_path / "cal.json"; json.dump(calib, open(cf, "w"))
    st = PanoStitcher(str(cf), out_w=720, out_h=360)
    img0 = np.full((H, W, 3), 200, np.uint8)
    img1 = np.full((H, W, 3), 100, np.uint8)
    pano = st.stitch(img0, img1)
    assert pano.shape == (360, 720, 3)
    # cam0 covers +Z hemisphere (panorama top), expected value ~200
    assert pano[10, 360].mean() > 180
    # equator overlap band should be between the two values (blended)
    mid = pano[180, 10].mean()
    assert 90 < mid < 210
