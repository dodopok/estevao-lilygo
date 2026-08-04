#include "estevao_storage.h"

#include <ArduinoJson.h>
#include <LittleFS.h>

namespace {

void ensureDirectories() {
    if (!LittleFS.exists("/estevao")) LittleFS.mkdir("/estevao");
    if (!LittleFS.exists("/estevao/cache")) LittleFS.mkdir("/estevao/cache");
}

String readConfigString(JsonDocument &doc, const char *key, const String &fallback) {
    if (doc[key].isNull()) return fallback;
    return doc[key].as<String>();
}

} // namespace

namespace EstevaoStorage {

bool loadConfig(EstevaoConfig &config) {
    EstevaoConfig defaults;
    config = defaults;
    ensureDirectories();

    File file = LittleFS.open(CONFIG_PATH, FILE_READ);
    if (!file) return false;

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();
    if (error) return false;

    config.baseUrl = readConfigString(doc, "base_url", config.baseUrl);
    config.apiKey = readConfigString(doc, "api_key", config.apiKey);
    config.openaiBaseUrl = readConfigString(doc, "openai_base_url", config.openaiBaseUrl);
    config.openaiApiKey = readConfigString(doc, "openai_api_key", config.openaiApiKey);
    config.openaiModel = readConfigString(doc, "openai_model", config.openaiModel);
    config.mcpServerUrl = readConfigString(doc, "mcp_server_url", config.mcpServerUrl);
    config.mcpToken = readConfigString(doc, "mcp_token", config.mcpToken);
    config.prayerBookCode = readConfigString(doc, "prayer_book_code", config.prayerBookCode);
    config.bibleVersion = readConfigString(doc, "bible_version", config.bibleVersion);
    config.readingType = readConfigString(doc, "reading_type", config.readingType);
    config.language = readConfigString(doc, "language", config.language);
    config.timezone = readConfigString(doc, "timezone", config.timezone);

    if (!doc["voice_max_ms"].isNull()) {
        uint32_t voiceMaxMs = doc["voice_max_ms"].as<uint32_t>();
        config.voiceMaxMs = constrain(voiceMaxMs, 3000UL, 15000UL);
    }

    config.baseUrl.trim();
    while (config.baseUrl.endsWith("/")) config.baseUrl.remove(config.baseUrl.length() - 1);
    config.openaiBaseUrl.trim();
    while (config.openaiBaseUrl.endsWith("/")) config.openaiBaseUrl.remove(config.openaiBaseUrl.length() - 1);
    config.openaiModel.trim();
    config.mcpServerUrl.trim();
    while (config.mcpServerUrl.endsWith("/")) config.mcpServerUrl.remove(config.mcpServerUrl.length() - 1);
    config.mcpToken.trim();
    return true;
}

bool saveConfig(const EstevaoConfig &config) {
    ensureDirectories();

    File file = LittleFS.open(CONFIG_PATH, FILE_WRITE);
    if (!file) return false;

    JsonDocument doc;
    doc["base_url"] = config.baseUrl;
    doc["api_key"] = config.apiKey;
    doc["openai_base_url"] = config.openaiBaseUrl;
    doc["openai_api_key"] = config.openaiApiKey;
    doc["openai_model"] = config.openaiModel;
    doc["mcp_server_url"] = config.mcpServerUrl;
    doc["mcp_token"] = config.mcpToken;
    doc["prayer_book_code"] = config.prayerBookCode;
    doc["bible_version"] = config.bibleVersion;
    doc["reading_type"] = config.readingType;
    doc["language"] = config.language;
    doc["timezone"] = config.timezone;
    doc["voice_max_ms"] = config.voiceMaxMs;

    size_t written = serializeJsonPretty(doc, file);
    file.close();
    return written > 0;
}

bool saveText(const char *path, const String &text) {
    ensureDirectories();
    File file = LittleFS.open(path, FILE_WRITE);
    if (!file) return false;
    size_t written = file.write(reinterpret_cast<const uint8_t *>(text.c_str()), text.length());
    file.close();
    return written == text.length();
}

bool loadText(const char *path, String &text, size_t maxBytes) {
    text = "";
    File file = LittleFS.open(path, FILE_READ);
    if (!file) return false;

    size_t fileSize = file.size();
    if (fileSize > maxBytes) {
        file.close();
        return false;
    }

    text.reserve(fileSize + 1);
    while (file.available()) text += static_cast<char>(file.read());
    file.close();
    return true;
}

bool saveCalendar(const String &date, const String &payload) {
    return saveText(TODAY_PATH, payload) && saveText(TODAY_DATE_PATH, date);
}

bool loadCalendar(String &date, String &payload) {
    return loadText(TODAY_DATE_PATH, date, 32) && loadText(TODAY_PATH, payload);
}

bool saveExplanation(const String &payload) { return saveText(EXPLANATION_PATH, payload); }

bool loadExplanation(String &payload) { return loadText(EXPLANATION_PATH, payload); }

bool saveAnswer(const String &payload) { return saveText(ANSWER_PATH, payload); }

bool loadAnswer(String &payload) { return loadText(ANSWER_PATH, payload, 64 * 1024); }

} // namespace EstevaoStorage
