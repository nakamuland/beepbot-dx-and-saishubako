#pragma once

// Load the selected ESP-IDF configuration, then prevent Arduino's PSRAM
// startup hook and PSRAM heap integration from being compiled into this
// no-PSRAM Cardputer build. The selected qio_qspi SDK variant otherwise has
// generic QSPI-PSRAM support enabled in its shared configuration header.
#include <sdkconfig.h>

#undef CONFIG_SPIRAM
#undef CONFIG_SPIRAM_SUPPORT
#undef CONFIG_SPIRAM_BOOT_INIT
#undef CONFIG_SPIRAM_USE_MALLOC
#undef BOARD_HAS_PSRAM
