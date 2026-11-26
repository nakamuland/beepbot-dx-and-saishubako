# Saishubako source for the K132 dual-app build

This directory contains Saishubako source adapted for the standard M5Stack Cardputer K132 no-PSRAM build and the dual-app launcher's OTA app1-to-app0 Fn+Q return path. The source uses the project's K132 board profile and `no_psram.h` configuration. Its `setRotation(1)` display setting is retained from the supplied Saishubako source; it is not a new launcher modification. The original MIT notice is preserved in [`LICENSE`](LICENSE), including the Saishubako contributors attribution.

Build the combined project from the repository root using `./build_all.sh` or `build_all.ps1` as described in the root [README](../README.md). This directory intentionally does not include the archive's full-flash image, device-specific flash backups, or standalone installer/flash scripts. The combined build creates an app-only OTA image for app1; it is not the original 0x0 full-flash image.

At runtime, Saishubako saves notes and recordings under `SAISHU` on the Cardputer's microSD card. On a standard Cardputer K132, the owner confirmed that the Saishubako-only rotated image in the rebuilt launcher booted in landscape and Fn+Q returned to the launcher. This check covers display, boot and return behavior; it does not by itself verify recording/playback or every interaction. Back up the SD card before first use.

## MX11 printing

Open a saved note with text and press **Fn+P** to print its record number and body.
The driver in `src/mx11.h` uses the 384-dot, `51 78` / `AE01` protocol verified with the owner's MX11 and standalone SAISHUBAKO.
It renders Japanese text with the bundled M5GFX Japanese font and transfers raster rows over BLE.
The integration uses NimBLE-Arduino **2.3.6** for Arduino core 3.2.0.

Disconnect the iPhone FunPrint app before connecting from the Cardputer. If necessary, turn off Bluetooth in the iPhone Settings and restart MX11.
Other MX11 variants may use a different protocol. The success message confirms transfer, so inspect the printed paper to confirm physical output.
Printing is synchronous. Return to the launcher with **Fn+Q** after printing has finished.
