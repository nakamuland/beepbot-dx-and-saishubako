#!/usr/bin/env python3
"""Basic offline checks for the built K132 app images and launcher package."""
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
DIST = ROOT / "dist"
SLOT_SIZE = 0x330000
FILES = {
    "beepBot DX OTA app": DIST / "beepbotdx-k132-ota1.bin",
    "Saishubako OTA app": DIST / "saishubako-k132-ota1.bin",
    "K132 launcher OTA0 app": DIST / "k132-app-launcher-ota0.bin",
}


def main() -> None:
    payloads = {}
    failed = False
    for label, path in FILES.items():
        if not path.is_file():
            print(f"FAIL: missing {label}: {path}")
            failed = True
            continue
        data = path.read_bytes()
        payloads[label] = data
        if not data or data[0] != 0xE9:
            print(f"FAIL: {label} is not an ESP app image (magic {data[:1].hex()})")
            failed = True
        elif len(data) < 3:
            print(f"FAIL: {label} is too short to contain an ESP image header")
            failed = True
        elif data[2] != 2:
            print(f"FAIL: {label} flash mode is {data[2]}, expected DIO (2)")
            failed = True
        elif len(data) > SLOT_SIZE:
            print(f"FAIL: {label} is {len(data)} bytes; slot is {SLOT_SIZE} bytes")
            failed = True
        else:
            print(f"OK:   {label}: {len(data):,} bytes / {SLOT_SIZE:,}-byte app slot")
    launcher = payloads.get("K132 launcher OTA0 app")
    if launcher:
        for label in ("beepBot DX OTA app", "Saishubako OTA app"):
            image = payloads.get(label)
            if image and image not in launcher:
                print(f"FAIL: launcher does not contain exact {label} bytes")
                failed = True
            elif image:
                print(f"OK:   launcher contains exact {label} image")
    if failed:
        sys.exit(1)
    print("All offline artifact checks passed. This does not replace hardware testing.")


if __name__ == "__main__":
    main()
