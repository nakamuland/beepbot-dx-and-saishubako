# Third-party notices

This repository combines code with separate copyright holders. The MIT licenses below are compatible, but their copyright notices are not interchangeable and must remain with their respective code.

- **beepBot DX** (`beepbotdx/`): retain `beepbotdx/LICENSE`, which names **Brad McNally**. The original source README and artwork are also retained. K132/no-PSRAM and launcher-return integration changes do not remove the upstream notice.
- **Saishubako** (`saishubako/`): retain `saishubako/LICENSE`, which names **Saishubako contributors**. The source was adapted for the K132 no-PSRAM build and launcher-return integration; its original notice remains in place.
- **New launcher and repository integration** (`launcher/`, root build scripts and `tools/`): covered by the root `LICENSE`, Copyright (c) 2026 nakamuland. Any changes made to upstream-derived files remain subject to the applicable upstream notice as well.

PlatformIO dependencies are fetched rather than vendored here. Their own license terms continue to apply.

## UnitCamS3 integration

- New UART camera firmware and Cardputer photo UI are covered by the root MIT license.
- The camera firmware uses the Espressif Arduino ESP32 platform and its bundled [esp32-camera](https://github.com/espressif/esp32-camera) driver (Apache-2.0). Dependency license notices remain in the fetched platform.
- OV2640 and Grove pin assignments follow [M5Stack's Unit CamS3 documentation](https://docs.m5stack.com/en/unit/Unit_CamS3).

## MX11 printing integration

- **NimBLE-Arduino 2.3.6**: https://github.com/h2zero/NimBLE-Arduino/tree/2.3.6 — Apache-2.0, with component notices retained in the fetched library.
- The printer command framing and sequence are ported from the owner's standalone [SAISHUBAKO](https://github.com/nakamuland/SAISHUBAKO), retaining its MIT attribution in `saishubako/LICENSE`.
- Protocol references include the [MX11 command sequence](https://stackoverflow.com/questions/79301981/mx11-cat-thermal-printer-android-sdk) and [FunPrint-compatible protocol implementation](https://github.com/eraldonio/funprint).
- Japanese font data is provided by M5GFX and remains subject to its bundled /efont/ notices.
