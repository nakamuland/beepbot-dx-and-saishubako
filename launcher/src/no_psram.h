#pragma once
// Standard Cardputer K132 has no PSRAM. Remove the generic QSPI-PSRAM
// configuration before Arduino/M5Unified compiles its startup hooks.
#include <sdkconfig.h>
#undef CONFIG_SPIRAM
#undef CONFIG_SPIRAM_SUPPORT
#undef CONFIG_SPIRAM_BOOT_INIT
#undef CONFIG_SPIRAM_USE_MALLOC
#undef BOARD_HAS_PSRAM
