#ifndef __ESTEVAO_MODELS_H__
#define __ESTEVAO_MODELS_H__

#include <Arduino.h>

struct EstevaoConfig {
    String baseUrl;
    String apiKey;
    String prayerBookCode;
    String bibleVersion;
    String readingType;
    String language;
    String timezone;
    uint32_t voiceMaxMs;

    EstevaoConfig()
        : baseUrl("https://api.caminhoanglicano.com.br"), apiKey(""), prayerBookCode("loc_2015"),
          bibleVersion("nvi"), readingType("semicontinuous"), language("pt-BR"), timezone("America/Sao_Paulo"),
          voiceMaxMs(8000) {}

    bool hasCredentials() const { return baseUrl.length() > 0 && apiKey.length() > 0; }
};

struct EstevaoResponse {
    bool ok = false;
    int httpStatus = 0;
    bool fromCache = false;
    String body;
    String error;
};

#endif
