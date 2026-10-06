"""Ground station main control: receive -> decode -> reassemble -> stitch -> display.
Run: python3 ground_station.py [--kiss 127.0.0.1:8001] [--out received/]
"""
import argparse, os, sys, time
sys.path.insert(0, os.path.dirname(__file__))
from receiver import KissReceiver, parse_downlink
from image_assemble import FileAssembler

BEACON_LOG = "beacons.log"

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--kiss", default="127.0.0.1:8001")
    ap.add_argument("--out", default="received")
    ap.add_argument("--replay", help="offline replay: read from raw frame file (for testing)")
    a = ap.parse_args()

    asm = FileAssembler(a.out)
    done_files = []

    def on_pkt(pkt):
        d = parse_downlink(pkt["payload"])
        if not d: return
        if d["type"] == "beacon":
            line = (f"{time.strftime('%H:%M:%S')} BEACON seq={d['seq']} "
                    f"mode={d['mode']} vbat={d['vbat_mv']}mV i={d['ibat_ma']}mA "
                    f"temp={d['temp']} gyro={d['gyro_dps']} "
                    f"pl_mode={d['pl_mode']} free={d['free_mb']}MB files={d['files']}")
            print(line)
            open(BEACON_LOG, "a").write(line + "\n")
        elif d["type"] == "chunk":
            done = asm.feed(d)
            if done:
                print(f"[done] file_id={d['file_id']} -> {done}")
                done_files.append(done)
        prog = asm.progress()
        if prog and d["type"] == "chunk" and d["chunk_no"] % 200 == 0:
            print("[progress]", prog)

    if a.replay:
        for line in open(a.replay, "rb").read().split(b"\xc0"):
            if line:
                on_pkt({"payload": line[17:-2] if len(line) > 19 else line})
        return

    host, port = a.kiss.split(":")
    print(f"[ground] connecting to Direwolf KISS {host}:{port} ...")
    rx = KissReceiver(host, int(port))
    rx.on_packet(on_pkt)
    print("[ground] waiting for beacon... (Ctrl-C to exit)")
    try:
        rx.run()
    except KeyboardInterrupt:
        print("\n[ground] exit. received files:", done_files)

if __name__ == "__main__":
    main()
