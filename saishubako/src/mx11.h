#pragma once

#include <M5Cardputer.h>
#include <NimBLEDevice.h>
#include <algorithm>
#include <vector>
#include "romaji.h"

// The 51 78 / AE01 path was verified on the owner's FunPrint-compatible MX11.
// AE03 being present does not imply that this particular unit uses MXW01 jobs.
namespace mx11 {
constexpr int WIDTH = 384;
constexpr int LINE_HEIGHT = 24;

inline uint8_t crc8(const uint8_t* data, size_t size) {
    uint8_t crc = 0;
    for (size_t i = 0; i < size; ++i) {
        crc ^= data[i];
        for (int bit = 0; bit < 8; ++bit) {
            crc = (crc << 1) ^ ((crc & 0x80) ? 0x07 : 0);
        }
    }
    return crc;
}

inline bool send(NimBLEClient& client, NimBLERemoteCharacteristic& tx,
                 uint8_t command, const uint8_t* data, size_t size) {
    if (!client.isConnected() || size > WIDTH / 8 || client.getMTU() < 23) return false;
    uint8_t packet[56] = {0x51, 0x78, command, 0, uint8_t(size), 0};
    if (size) memcpy(packet + 6, data, size);
    packet[size + 6] = crc8(data, size);
    packet[size + 7] = 0xff;
    const size_t chunkSize = client.getMTU() - 3;
    for (size_t offset = 0; offset < size + 8; offset += chunkSize) {
        const size_t amount = std::min(chunkSize, size + 8 - offset);
        if (!client.isConnected() || !tx.writeValue(packet + offset, amount, !tx.canWriteNoResponse())) {
            return false;
        }
        delay(12);
    }
    return true;
}

inline const char* printConnected(NimBLEClient& client, const String& note, uint32_t id) {
    auto* service = client.getService(NimBLEUUID(uint16_t(0xae30)));
    if (!service) service = client.getService(NimBLEUUID(uint16_t(0xaf30)));
    if (!service) return "MX11 service unavailable";
    auto* tx = service->getCharacteristic(NimBLEUUID(uint16_t(0xae01)));
    if (!tx || (!tx->canWrite() && !tx->canWriteNoResponse())) return "MX11 channel unavailable";
    auto* rx = service->getCharacteristic(NimBLEUUID(uint16_t(0xae02)));
    if (!rx || !rx->canNotify() ||
        !rx->subscribe(true, [](NimBLERemoteCharacteristic*, uint8_t*, size_t, bool) {})) {
        return "MX11 notify setup failed";
    }
    delay(50);

    M5Canvas canvas(&M5Cardputer.Display);
    canvas.setColorDepth(1);
    if (!canvas.createSprite(WIDTH, LINE_HEIGHT)) return "Not enough memory to print";
    canvas.setFont(&fonts::efontJA_10);
    canvas.setTextSize(2);
    canvas.setTextColor(TFT_WHITE);

    std::vector<String> lines;
    lines.push_back("SAISHUBAKO #" + String(id));
    String current;
    for (unsigned i = 0; i < note.length();) {
        const unsigned start = i++;
        while (i < note.length() && utf8Continuation(note[i])) ++i;
        const String character = note.substring(start, i);
        if (canvas.textWidth(current + character) > WIDTH - 16) {
            lines.push_back(current);
            current = "";
        }
        current += character;
    }
    if (!current.isEmpty()) lines.push_back(current);

    const uint8_t zero = 0;
    const uint8_t quality = 0x32;
    const uint8_t latticeStart[] = {0xaa, 0x55, 0x17, 0x38, 0x44, 0x5f, 0x5f, 0x5f, 0x44, 0x38, 0x2c};
    const uint8_t energy[] = {0xe0, 0x2e};
    const uint8_t drawingMode = 1;
    const uint8_t speed = 25;
    // This MX11 accepts printing without status notifications. A missing reply
    // to A3 must not block the command sequence that worked on the hardware.
    if (!send(client, *tx, 0xa3, &zero, 1) ||
        !send(client, *tx, 0xa4, &quality, 1) ||
        !send(client, *tx, 0xa6, latticeStart, sizeof(latticeStart)) ||
        !send(client, *tx, 0xaf, energy, sizeof(energy)) ||
        !send(client, *tx, 0xbe, &drawingMode, 1) ||
        !send(client, *tx, 0xbd, &speed, 1)) return "MX11 setup failed";

    for (const auto& text : lines) {
        canvas.fillScreen(TFT_BLACK);
        canvas.drawString(text, 8, 1);
        for (int y = 0; y < LINE_HEIGHT; ++y) {
            uint8_t row[WIDTH / 8] = {};
            for (int x = 0; x < WIDTH; ++x) {
                if (canvas.readPixel(x, y)) row[x / 8] |= 1 << (x % 8);
            }
            if (!send(client, *tx, 0xa2, row, sizeof(row))) return "Print interrupted";
        }
    }
    const uint8_t latticeEnd[] = {0xaa, 0x55, 0x17, 0, 0, 0, 0, 0, 0, 0, 0x17};
    const uint8_t feed[] = {48, 0};
    if (!send(client, *tx, 0xa6, latticeEnd, sizeof(latticeEnd)) ||
        !send(client, *tx, 0xa1, feed, sizeof(feed))) return "MX11 paper feed failed";
    delay(300);
    return "Sent to MX11. Check paper.";
}

inline const char* printNote(const String& note, uint32_t id) {
    static bool initialized = false;
    if (!initialized) {
        if (!NimBLEDevice::init("")) return "Bluetooth initialization failed";
        NimBLEDevice::setMTU(128);
        initialized = true;
    }
    auto* scan = NimBLEDevice::getScan();
    scan->setActiveScan(true);
    auto results = scan->getResults(4000);
    const char* result = "MX11 not found. Check power.";
    for (int i = 0; i < results.getCount(); ++i) {
        const auto* printer = results.getDevice(i);
        if (!printer || printer->getName() != "MX11") continue;
        auto* client = NimBLEDevice::createClient();
        if (!client) { result = "Cannot create BLE client"; break; }
        client->setConnectTimeout(5000);
        result = client->connect(printer) ? printConnected(*client, note, id) : "MX11 connection failed";
        if (client->isConnected()) client->disconnect();
        NimBLEDevice::deleteClient(client);
        break;
    }
    scan->clearResults();
    Serial.printf("MX11: %s\n", result);
    return result;
}
} // namespace mx11
