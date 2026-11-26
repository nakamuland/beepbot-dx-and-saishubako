#include <M5Cardputer.h>
#include <esp_err.h>
#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_system.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>

extern "C" {
extern const uint8_t beepbotdx_ota_start[];
extern const uint8_t beepbotdx_ota_end[];
extern const uint8_t saishubako_ota_start[];
extern const uint8_t saishubako_ota_end[];
}

namespace {
constexpr uint16_t BG = 0x0841;
constexpr uint16_t INK = 0xDEDA;
constexpr uint16_t ACCENT = 0x7F31;
constexpr size_t WRITE_CHUNK = 4096;

struct AppImage {
    const char* label;
    const uint8_t* begin;
    const uint8_t* end;
};

const AppImage APPS[] = {
    {"beepBot DX", beepbotdx_ota_start, beepbotdx_ota_end},
    {"Saishubako", saishubako_ota_start, saishubako_ota_end},
};
constexpr size_t APP_COUNT = sizeof(APPS) / sizeof(APPS[0]);
size_t selected = 0;
String statusText = "Select an app to start";

void drawScreen() {
    auto& d = M5Cardputer.Display;
    d.fillScreen(BG);
    d.setFont(&fonts::Font0);
    d.setTextSize(1);
    d.setTextColor(ACCENT, BG);
    d.setCursor(8, 8);
    d.print("CARDPUTER K132 / APP LAUNCHER");
    d.drawFastHLine(8, 23, 224, 0x31C6);

    for (size_t i = 0; i < APP_COUNT; ++i) {
        int y = 44 + static_cast<int>(i) * 30;
        d.setTextColor(i == selected ? TFT_WHITE : INK, BG);
        d.setCursor(18, y);
        d.printf("%c %u  %s", i == selected ? '>' : ' ', static_cast<unsigned>(i + 1), APPS[i].label);
    }

    d.setTextColor(ACCENT, BG);
    d.setCursor(8, 108);
    d.print(statusText.substring(0, 36));
    d.setTextColor(INK, BG);
    d.setCursor(8, 124);
    d.print("1/2: launch  ;/.:select  Enter:launch");
}

void showError(const char* message) {
    statusText = message;
    drawScreen();
    Serial.printf("Launcher error: %s\n", message);
}

bool launchApp(size_t index) {
    if (index >= APP_COUNT) return false;
    const AppImage& image = APPS[index];
    const size_t imageSize = static_cast<size_t>(image.end - image.begin);
    const esp_partition_t* running = esp_ota_get_running_partition();
    const esp_partition_t* launcher = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_0, nullptr);
    const esp_partition_t* target = esp_partition_find_first(
        ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_OTA_1, nullptr);

    if (!running || running->subtype != ESP_PARTITION_SUBTYPE_APP_OTA_0 || !launcher || !target) {
        showError("Launcher/OTA slots not found; no app written");
        return false;
    }
    if (imageSize == 0 || imageSize > target->size) {
        showError("Embedded app exceeds OTA slot; no app written");
        return false;
    }

    statusText = String("Preparing ") + image.label;
    drawScreen();
    Serial.printf("Installing %s: %u bytes to app1 at 0x%06x\n",
                  image.label, static_cast<unsigned>(imageSize), static_cast<unsigned>(target->address));

    esp_ota_handle_t handle = 0;
    esp_err_t result = esp_ota_begin(target, imageSize, &handle);
    if (result != ESP_OK) {
        showError("Could not prepare app slot");
        return false;
    }

    bool handleOpen = true;
    for (size_t offset = 0; offset < imageSize; offset += WRITE_CHUNK) {
        const size_t amount = std::min(WRITE_CHUNK, imageSize - offset);
        result = esp_ota_write(handle, image.begin + offset, amount);
        if (result != ESP_OK) {
            esp_ota_abort(handle);
            handleOpen = false;
            showError("App copy failed; launcher remains");
            Serial.printf("esp_ota_write failed at %u: %s\n",
                          static_cast<unsigned>(offset), esp_err_to_name(result));
            return false;
        }
        if ((offset / WRITE_CHUNK) % 16 == 0 || offset + amount == imageSize) {
            const unsigned percent = static_cast<unsigned>((offset + amount) * 100 / imageSize);
            statusText = String("Copying ") + image.label + " " + percent + "%";
            drawScreen();
        }
    }

    result = esp_ota_end(handle);
    handleOpen = false;
    if (result != ESP_OK) {
        showError("App image validation failed; launcher remains");
        Serial.printf("esp_ota_end failed: %s\n", esp_err_to_name(result));
        return false;
    }
    if (handleOpen) esp_ota_abort(handle);

    result = esp_ota_set_boot_partition(target);
    if (result != ESP_OK) {
        showError("Could not select app; launcher remains");
        Serial.printf("esp_ota_set_boot_partition failed: %s\n", esp_err_to_name(result));
        return false;
    }

    statusText = String("Starting ") + image.label;
    drawScreen();
    Serial.printf("Starting %s\n", image.label);
    delay(250);
    esp_restart();
    return true;
}

void handleKeys() {
    if (!M5Cardputer.Keyboard.isChange() || !M5Cardputer.Keyboard.isPressed()) return;
    auto state = M5Cardputer.Keyboard.keysState();
    if (state.enter) {
        launchApp(selected);
        return;
    }
    for (auto key : state.word) {
        if (key == '1') { selected = 0; drawScreen(); launchApp(0); return; }
        if (key == '2') { selected = 1; drawScreen(); launchApp(1); return; }
        if (key == ';' || key == ',') {
            selected = (selected + APP_COUNT - 1) % APP_COUNT;
            drawScreen();
            return;
        }
        if (key == '.' || key == '/') {
            selected = (selected + 1) % APP_COUNT;
            drawScreen();
            return;
        }
    }
}
} // namespace

void setup() {
    Serial.begin(115200);
    delay(250);
    auto cfg = M5.config();
    cfg.fallback_board = m5::board_t::board_M5Cardputer;
    M5Cardputer.begin(cfg, true);
    M5Cardputer.Display.setRotation(0);
    M5Cardputer.Display.setBrightness(100);
    M5Cardputer.Display.setTextColor(INK, BG);
    const esp_partition_t* running = esp_ota_get_running_partition();
    Serial.printf("K132 launcher: running=%s / embedded beepBot=%u Saishubako=%u\n",
                  running ? running->label : "none",
                  static_cast<unsigned>(beepbotdx_ota_end - beepbotdx_ota_start),
                  static_cast<unsigned>(saishubako_ota_end - saishubako_ota_start));
    drawScreen();
}

void loop() {
    M5Cardputer.update();
    handleKeys();
    delay(10);
}
