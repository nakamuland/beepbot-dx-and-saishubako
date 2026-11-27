#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
DIST="$ROOT/dist"

if [[ -n "${PIO:-}" ]]; then
  PIO_CMD="$PIO"
elif command -v pio >/dev/null 2>&1; then
  PIO_CMD="$(command -v pio)"
else
  echo "PlatformIO CLI (pio) is required." >&2
  exit 2
fi
PYTHON_CMD="${PYTHON:-python3}"
mkdir -p "$DIST"

printf '\n[1/4] Build beepBot DX (K132 no-PSRAM, OTA app)\n'
(cd "$ROOT/beepbotdx" && "$PIO_CMD" run -e m5stack-cardputer)
cp "$ROOT/beepbotdx/.pio/build/m5stack-cardputer/firmware.bin" "$DIST/beepbotdx-k132-ota1.bin"

printf '\n[2/4] Build Saishubako (K132 no-PSRAM, OTA app)\n'
(cd "$ROOT/saishubako" && "$PIO_CMD" run -e m5stack-cardputer)
cp "$ROOT/saishubako/.pio/build/m5stack-cardputer/firmware.bin" "$DIST/saishubako-k132-ota1.bin"

printf '\n[3/4] Embed both OTA images into the launcher source\n'
"$PYTHON_CMD" "$ROOT/tools/embed_app_images.py" \
  --beepbot "$DIST/beepbotdx-k132-ota1.bin" \
  --saishubako "$DIST/saishubako-k132-ota1.bin" \
  --output "$ROOT/launcher/src/embedded_apps.S"

# PlatformIO does not track the .incbin payload as an object dependency.
(cd "$ROOT/launcher" && "$PIO_CMD" run -e launcher -t clean)

printf '\n[4/4] Build persistent launcher (K132 no-PSRAM, OTA0 app)\n'
(cd "$ROOT/launcher" && "$PIO_CMD" run -e launcher)
cp "$ROOT/launcher/.pio/build/launcher/firmware.bin" "$DIST/k132-app-launcher-ota0.bin"
(cd "$ROOT/unitcams3" && "$PIO_CMD" run -e unitcams3)
cp "$ROOT/unitcams3/.pio/build/unitcams3/firmware.bin" "$DIST/unitcams3-camera.bin"
"$PYTHON_CMD" "$ROOT/tools/verify_artifacts.py"
(
  cd "$DIST"
  sha256sum beepbotdx-k132-ota1.bin saishubako-k132-ota1.bin k132-app-launcher-ota0.bin unitcams3-camera.bin > SHA256SUMS.txt
)
printf '\nBuild complete. Device/microSD were not accessed or changed.\n'
printf 'Artifacts and SHA-256 list: %s\n' "$DIST"
