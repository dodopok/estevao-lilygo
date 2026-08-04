#include "estevao_client.h"

#include "estevao_ca.h"
#include "estevao_storage.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <globals.h>

namespace {

constexpr size_t MAX_JSON_RESPONSE = 160 * 1024;
constexpr uint32_t HTTP_TIMEOUT_MS = 20000;

String joinUrl(const String &baseUrl, const String &path) {
    String url = baseUrl;
    while (url.endsWith("/")) url.remove(url.length() - 1);
    if (!path.startsWith("/")) url += "/";
    url += path;
    return url;
}

String statusError(int status) {
    switch (status) {
        case 400: return "Preferências ou payload inválido";
        case 401: return "API key recusada";
        case 404: return "Conteúdo não encontrado";
        case 429: return "Limite de requisições atingido";
        case 500:
        case 503: return "Serviço temporariamente indisponível";
        default: return "HTTP " + String(status);
    }
}

const char *caCertificate(String &overrideCertificate) {
    if (EstevaoStorage::loadText(EstevaoStorage::CA_PATH, overrideCertificate, 8192) &&
        overrideCertificate.startsWith("-----BEGIN CERTIFICATE-----")) {
        return overrideCertificate.c_str();
    }
    return ESTEVAO_CA_BUNDLE;
}

bool validDate(const String &date) {
    if (date.length() != 10 || date.charAt(4) != '-' || date.charAt(7) != '-') return false;

    for (size_t i = 0; i < date.length(); i++) {
        if (i == 4 || i == 7) continue;
        if (date.charAt(i) < '0' || date.charAt(i) > '9') return false;
    }
    return true;
}

} // namespace

String EstevaoClient::urlEncode(const String &value) {
    static const char hex[] = "0123456789ABCDEF";
    String encoded;
    encoded.reserve(value.length() + 16);

    for (size_t i = 0; i < value.length(); i++) {
        uint8_t c = static_cast<uint8_t>(value.charAt(i));
        bool safe = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' ||
                    c == '_' || c == '.' || c == '~';
        if (safe) encoded += static_cast<char>(c);
        else {
            encoded += '%';
            encoded += hex[(c >> 4) & 0x0F];
            encoded += hex[c & 0x0F];
        }
    }
    return encoded;
}

String EstevaoClient::datePath(const String &date) {
    if (!validDate(date)) return "";
    return date.substring(0, 4) + "/" + date.substring(5, 7) + "/" + date.substring(8, 10);
}

String EstevaoClient::preferencesJson() const {
    JsonDocument preferences;
    preferences["prayer_book_code"] = _config.prayerBookCode;
    preferences["bible_version"] = _config.bibleVersion;
    preferences["reading_type"] = _config.readingType;

    String result;
    serializeJson(preferences, result);
    return result;
}

String EstevaoClient::buildCalendarPath(const String &date) const {
    String path = "/api/v1/calendar/" + datePath(date);
    path += "?preferences=" + urlEncode(preferencesJson());
    return path;
}

String EstevaoClient::buildExplanationPath(const String &date) const {
    String path = "/api/v1/liturgical_explanation/" + datePath(date);
    path += "?preferences=" + urlEncode(preferencesJson());
    return path;
}

EstevaoResponse EstevaoClient::requestJson(const char *method, const String &path, const String &body) {
    EstevaoResponse response;
    if (WiFi.status() != WL_CONNECTED) {
        response.error = "Wi-Fi desconectado";
        return response;
    }
    if (!_config.hasEstevaoCredentials()) {
        response.error = "Configure a API key do dispositivo";
        return response;
    }
    if (!_config.baseUrl.startsWith("https://")) {
        response.error = "A API deve usar HTTPS";
        return response;
    }

    String caOverride;

    WiFiClientSecure secureClient;
    secureClient.setTimeout(HTTP_TIMEOUT_MS);
    secureClient.setCACert(caCertificate(caOverride));

    HTTPClient http;
    http.setTimeout(HTTP_TIMEOUT_MS);
    String url = joinUrl(_config.baseUrl, path);
    if (!http.begin(secureClient, url)) {
        response.error = "Não foi possível iniciar HTTPS";
        return response;
    }

    http.addHeader("Accept", "application/json");
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-API-Key", _config.apiKey);
    http.addHeader("User-Agent", "Estevao-LilyGO/0.1");

    int status = (strcmp(method, "GET") == 0) ? http.GET() : http.POST(body);
    response.httpStatus = status;
    if (status <= 0) {
        response.error = http.errorToString(status);
        http.end();
        return response;
    }

    int contentLength = http.getSize();
    size_t maxResponse = MAX_JSON_RESPONSE;
    if (contentLength > 0 && static_cast<size_t>(contentLength) > maxResponse) {
        response.error = "Resposta excede o limite do dispositivo";
        http.end();
        return response;
    }

    response.body = http.getString();
    http.end();
    if (response.body.length() > maxResponse) {
        response.body = "";
        response.error = "Resposta excede o limite do dispositivo";
        return response;
    }

    response.ok = status >= 200 && status < 300;
    if (!response.ok) response.error = statusError(status);
    return response;
}

EstevaoResponse EstevaoClient::getCalendar(const String &date) {
    if (!validDate(date)) return {false, 400, false, "", "Data inválida"};
    return requestJson("GET", buildCalendarPath(date));
}

EstevaoResponse EstevaoClient::getExplanation(const String &date) {
    if (!validDate(date)) return {false, 400, false, "", "Data inválida"};
    return requestJson("GET", buildExplanationPath(date));
}

String EstevaoClient::todayDate() {
    time_t epoch = localTime;
    if (epoch < 1577836800) epoch = rtc.getEpoch();
    if (epoch < 1577836800) epoch = time(nullptr);
    if (epoch < 1577836800) return "";

    struct tm timeInfo;
    gmtime_r(&epoch, &timeInfo);
    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d", timeInfo.tm_year + 1900, timeInfo.tm_mon + 1, timeInfo.tm_mday);
    return String(buffer);
}
