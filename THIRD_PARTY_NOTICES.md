# Third-party notices

This repository combines code with separate copyright holders. The MIT licenses below are compatible, but their copyright notices are not interchangeable and must remain with their respective code.

- **beepBot DX** (`beepbotdx/`): retain `beepbotdx/LICENSE`, which names **Brad McNally**. The original source README and artwork are also retained. K132/no-PSRAM and launcher-return integration changes do not remove the upstream notice.
- **Saishubako** (`saishubako/`): retain `saishubako/LICENSE`, which names **Saishubako contributors**. The source was adapted for the K132 no-PSRAM build and launcher-return integration; its original notice remains in place.
- **New launcher and repository integration** (`launcher/`, root build scripts and `tools/`): covered by the root `LICENSE`, Copyright (c) 2026 nakamuland. Any changes made to upstream-derived files remain subject to the applicable upstream notice as well.

PlatformIO dependencies are fetched rather than vendored here. Their own license terms continue to apply.

## MX11 printing integration

- **NimBLE-Arduino 2.3.6**: https://github.com/h2zero/NimBLE-Arduino/tree/2.3.6 — Apache-2.0, with component notices retained in the fetched library.
- The printer command framing and sequence are ported from the owner's standalone [SAISHUBAKO](https://github.com/nakamuland/SAISHUBAKO), retaining its MIT attribution in `saishubako/LICENSE`.
- Protocol references include the [MX11 command sequence](https://stackoverflow.com/questions/79301981/mx11-cat-thermal-printer-android-sdk) and [FunPrint-compatible protocol implementation](https://github.com/eraldonio/funprint).
- Japanese font data is provided by M5GFX and remains subject to its bundled /efont/ notices.
