#pragma once
#include <Arduino.h>

struct RomajiEntry { const char* keys; const char* kana; };
static const RomajiEntry romajiTable[] = {
    {"a", "あ"},
    {"i", "い"},
    {"u", "う"},
    {"e", "え"},
    {"o", "お"},
    {"ka", "か"},
    {"ki", "き"},
    {"ku", "く"},
    {"ke", "け"},
    {"ko", "こ"},
    {"sa", "さ"},
    {"si", "し"},
    {"su", "す"},
    {"se", "せ"},
    {"so", "そ"},
    {"ta", "た"},
    {"ti", "ち"},
    {"tu", "つ"},
    {"te", "て"},
    {"to", "と"},
    {"na", "な"},
    {"ni", "に"},
    {"nu", "ぬ"},
    {"ne", "ね"},
    {"no", "の"},
    {"ha", "は"},
    {"hi", "ひ"},
    {"hu", "ふ"},
    {"he", "へ"},
    {"ho", "ほ"},
    {"ma", "ま"},
    {"mi", "み"},
    {"mu", "む"},
    {"me", "め"},
    {"mo", "も"},
    {"ra", "ら"},
    {"ri", "り"},
    {"ru", "る"},
    {"re", "れ"},
    {"ro", "ろ"},
    {"ga", "が"},
    {"gi", "ぎ"},
    {"gu", "ぐ"},
    {"ge", "げ"},
    {"go", "ご"},
    {"za", "ざ"},
    {"zi", "じ"},
    {"zu", "ず"},
    {"ze", "ぜ"},
    {"zo", "ぞ"},
    {"da", "だ"},
    {"di", "ぢ"},
    {"du", "づ"},
    {"de", "で"},
    {"do", "ど"},
    {"ba", "ば"},
    {"bi", "び"},
    {"bu", "ぶ"},
    {"be", "べ"},
    {"bo", "ぼ"},
    {"pa", "ぱ"},
    {"pi", "ぴ"},
    {"pu", "ぷ"},
    {"pe", "ぺ"},
    {"po", "ぽ"},
    {"xa", "ぁ"},
    {"xi", "ぃ"},
    {"xu", "ぅ"},
    {"xe", "ぇ"},
    {"xo", "ぉ"},
    {"ya", "や"},
    {"yu", "ゆ"},
    {"yo", "よ"},
    {"wa", "わ"},
    {"wo", "を"},
    {"shi", "し"},
    {"chi", "ち"},
    {"tsu", "つ"},
    {"fu", "ふ"},
    {"ji", "じ"},
    {"nya", "にゃ"},
    {"nyu", "にゅ"},
    {"nyo", "にょ"},
    {"xtsu", "っ"},
    {"ltsu", "っ"},
    {"kya", "きゃ"},
    {"kyu", "きゅ"},
    {"kyo", "きょ"},
    {"gya", "ぎゃ"},
    {"gyu", "ぎゅ"},
    {"gyo", "ぎょ"},
    {"sya", "しゃ"},
    {"syu", "しゅ"},
    {"syo", "しょ"},
    {"sha", "しゃ"},
    {"shu", "しゅ"},
    {"sho", "しょ"},
    {"zya", "じゃ"},
    {"zyu", "じゅ"},
    {"zyo", "じょ"},
    {"jya", "じゃ"},
    {"jyu", "じゅ"},
    {"jyo", "じょ"},
    {"ja", "じゃ"},
    {"ju", "じゅ"},
    {"jo", "じょ"},
    {"tya", "ちゃ"},
    {"tyu", "ちゅ"},
    {"tyo", "ちょ"},
    {"cha", "ちゃ"},
    {"chu", "ちゅ"},
    {"cho", "ちょ"},
    {"cya", "ちゃ"},
    {"cyu", "ちゅ"},
    {"cyo", "ちょ"},
    {"dya", "ぢゃ"},
    {"dyu", "ぢゅ"},
    {"dyo", "ぢょ"},
    {"hya", "ひゃ"},
    {"hyu", "ひゅ"},
    {"hyo", "ひょ"},
    {"bya", "びゃ"},
    {"byu", "びゅ"},
    {"byo", "びょ"},
    {"pya", "ぴゃ"},
    {"pyu", "ぴゅ"},
    {"pyo", "ぴょ"},
    {"mya", "みゃ"},
    {"myu", "みゅ"},
    {"myo", "みょ"},
    {"rya", "りゃ"},
    {"ryu", "りゅ"},
    {"ryo", "りょ"},
};

inline bool utf8Continuation(char c) { return (uint8_t(c) & 0xc0) == 0x80; }

inline unsigned utf8Offset(const String& text, unsigned characters) {
    unsigned i = 0;
    while (i < text.length() && characters--) {
        do { ++i; } while (i < text.length() && utf8Continuation(text[i]));
    }
    return i;
}

inline unsigned utf8Length(const String& text) {
    unsigned count = 0;
    for (unsigned i = 0; i < text.length(); ++i)
        if (!utf8Continuation(text[i])) ++count;
    return count;
}

inline String utf8Slice(const String& text, unsigned start, unsigned count) {
    return text.substring(utf8Offset(text, start), utf8Offset(text, start + count));
}

inline void eraseLastCharacter(String& text) {
    if (text.isEmpty()) return;
    unsigned i = text.length() - 1;
    while (i && utf8Continuation(text[i])) --i;
    text.remove(i);
}

// Consume complete syllables and retain only a prefix that can still be completed.
inline String convertRomaji(String& pending) {
    String converted;
    while (!pending.isEmpty()) {
        if (pending == "n" || pending == "nn") break;
        if (pending.startsWith("nn")) {
            converted += "ん";
            pending.remove(0, strchr("aiueoy", pending[2]) ? 1 : 2);
            continue;
        }
        bool matched = false;
        for (const auto& entry : romajiTable) {
            if (pending == entry.keys) {
                converted += entry.kana;
                pending = "";
                matched = true;
                break;
            }
        }
        if (matched) break;
        char first = pending[0];
        char second = pending.length() > 1 ? pending[1] : '\0';
        if (first == 'n' && second && !strchr("aiueoyn", second)) {
            converted += "ん";
            pending.remove(0, second == 39 ? 2 : 1);
            continue;
        }
        if (first == second && first != 'n' && !strchr("aiueo", first)) {
            converted += "っ";
            pending.remove(0, 1);
            continue;
        }
        bool prefix = false;
        for (const auto& entry : romajiTable) {
            if (!strncmp(entry.keys, pending.c_str(), pending.length())) {
                prefix = true;
                break;
            }
        }
        if (prefix) break;
        converted += first;
        pending.remove(0, 1);
    }
    return converted;
}
