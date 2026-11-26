#!/usr/bin/env python3
"""Generate a tiny assembler source that embeds both app-only OTA images."""
from pathlib import Path
import argparse
import hashlib


def symbol_block(symbol: str, binary: Path) -> list[str]:
    path = binary.resolve().as_posix().replace('"', '\\"')
    return [
        # Change the assembly source whenever the embedded binary changes.
        f"# {symbol} SHA256 {hashlib.sha256(binary.read_bytes()).hexdigest()}",
        ".balign 16",
        f".global {symbol}_start",
        f"{symbol}_start:",
        f'.incbin "{path}"',
        f".global {symbol}_end",
        f"{symbol}_end:",
    ]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--beepbot", required=True, type=Path)
    parser.add_argument("--saishubako", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    args = parser.parse_args()
    for binary in (args.beepbot, args.saishubako):
        if not binary.is_file() or binary.stat().st_size == 0:
            raise SystemExit(f"Missing or empty app image: {binary}")
    lines = ['.section .rodata.embedded_apps,"a"']
    lines += symbol_block("beepbotdx_ota", args.beepbot)
    lines += symbol_block("saishubako_ota", args.saishubako)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Embedded-source generated: {args.output}")


if __name__ == "__main__":
    main()
