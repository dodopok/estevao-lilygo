#ifndef __ESTEVAO_MODELS_H__
#define __ESTEVAO_MODELS_H__

#include <Arduino.h>

struct EstevaoConfig {
    String baseUrl;
    String apiKey;
    String openaiBaseUrl;
    String openaiApiKey;
    String openaiModel;
    String mcpServerUrl;
    String mcpToken;
    String prayerBookCode;
    String bibleVersion;
    String readingType;
    String language;
    String timezone;
    uint32_t voiceMaxMs;

    EstevaoConfig()
        : baseUrl("https://api.caminhoanglicano.com.br"), apiKey(""), openaiBaseUrl("https://api.openai.com"),
          openaiApiKey(""), openaiModel("gpt-5.6-luna"), mcpServerUrl("https://estevao-mcp.dodopok.dev/mcp"),
          mcpToken(""), prayerBookCode("loc_2015"),
          bibleVersion("nvi"), readingType("semicontinuous"), language("pt-BR"), timezone("America/Sao_Paulo"),
          voiceMaxMs(8000) {}

    bool hasEstevaoCredentials() const { return baseUrl.length() > 0 && apiKey.length() > 0; }

    bool hasAssistantCredentials() const {
        return openaiBaseUrl.length() > 0 && openaiApiKey.length() > 0 && openaiModel.length() > 0 &&
               mcpServerUrl.length() > 0;
    }
};

struct EstevaoResponse {
    bool ok = false;
    int httpStatus = 0;
    bool fromCache = false;
    String body;
    String error;
};

#endif
