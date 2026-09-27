"""Storage manager: file registry, chunk metadata, space reclamation, power-loss protection."""
import os, json, time, zlib, threading

class StorageManager:
    def __init__(self, cfg):
        self.data_dir = cfg["data_dir"]
        self.reserve_mb = cfg["disk_reserve_mb"]
        self.index_path = os.path.join(self.data_dir, "index.json")
        self._lock = threading.Lock()
        os.makedirs(self.data_dir, exist_ok=True)
        self.index = self._load()

    def _load(self):
        if os.path.exists(self.index_path):
            try:
                return json.load(open(self.index_path))
            except Exception:
                # Corrupted index: back it up and rebuild without losing stored files
                os.rename(self.index_path, self.index_path + f".bad.{int(time.time())}")
        return {"next_id": 1, "files": {}}

    def _save(self):
        tmp = self.index_path + ".tmp"
        json.dump(self.index, open(tmp, "w"))
        os.replace(tmp, self.index_path)   # atomic write to reduce power-loss corruption risk

    # ---------- File registration ----------
    def register(self, path, kind, meta=None):
        """kind: pano_jpg / video / dng / preview"""
        with self._lock:
            fid = self.index["next_id"]
            self.index["next_id"] += 1
            self.index["files"][str(fid)] = {
                "path": path, "kind": kind, "ts": int(time.time()),
                "size": os.path.getsize(path), "meta": meta or {},
                "downloaded": False,
            }
            self._save()
            return fid

    def list_files(self, offset=0, limit=16):
        files = sorted(self.index["files"].items(),
                       key=lambda kv: -kv[1]["ts"])
        return [(int(fid), f["size"], f["ts"], f["kind"])
                for fid, f in files[offset:offset + limit]]

    # ---------- Chunked downlink ----------
    CHUNK = 200
    def prep_file(self, fid):
        f = self.index["files"].get(str(fid))
        if not f or not os.path.exists(f["path"]):
            return None
        total = (f["size"] + self.CHUNK - 1) // self.CHUNK
        crc = self._file_crc(f["path"])
        return {"file_id": fid, "total": total,
                "chunk_size": self.CHUNK, "crc32": crc}

    def read_chunk(self, fid, chunk_no):
        f = self.index["files"].get(str(fid))
        if not f:
            return None
        with open(f["path"], "rb") as fp:
            fp.seek(chunk_no * self.CHUNK)
            return fp.read(self.CHUNK)

    def _file_crc(self, path):
        crc = 0
        with open(path, "rb") as fp:
            while blk := fp.read(65536):
                crc = zlib.crc32(blk, crc)
        return crc & 0xFFFFFFFF

    # ---------- Space management ----------
    def free_mb(self):
        st = os.statvfs(self.data_dir)
        return st.f_bavail * st.f_frsize // (1024 * 1024)

    def ensure_space(self, need_mb):
        """Delete oldest downloaded files when space is insufficient."""
        while self.free_mb() < need_mb + self.reserve_mb:
            victims = [(f["ts"], fid) for fid, f in self.index["files"].items()
                       if f["downloaded"]]
            if not victims:
                return False      # nothing to delete; let upper layer report error
            _, fid = min(victims)
            self.delete(int(fid))
        return True

    def delete(self, fid):
        with self._lock:
            f = self.index["files"].pop(str(fid), None)
            if f and os.path.exists(f["path"]):
                try: os.remove(f["path"])
                except OSError: pass
            self._save()

    def mark_downloaded(self, fid):
        if str(fid) in self.index["files"]:
            self.index["files"][str(fid)]["downloaded"] = True
            self._save()
