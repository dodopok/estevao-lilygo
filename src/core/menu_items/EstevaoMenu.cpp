#include "EstevaoMenu.h"

#include "core/display.h"
#include "core/mykeyboard.h"
#include "core/scrollableTextArea.h"
#include "core/utils.h"
#include "modules/estevao/estevao_client.h"
#include "modules/estevao/openai_client.h"
#include "modules/estevao/estevao_storage.h"
#include "modules/others/mic.h"

#include <ArduinoJson.h>
#include <WiFi.h>

namespace {

constexpr size_t MAX_TEXT_FIELD = 14000;

String variantText(JsonVariant value) {
    if (value.isNull()) return "";

    String text = value.as<String>();
    if (text.length() == 0) serializeJson(value, text);
    return text;
}

String fieldText(JsonVariant object, const char *first, const char *second = nullptr) {
    if (!object.is<JsonObject>()) return "";
    String value = variantText(object[first]);
    if (value.length() == 0 && second != nullptr) value = variantText(object[second]);
    return value;
}

String documentField(JsonDocument &document, const char *first, const char *second = nullptr) {
    String value = variantText(document[first]);
    if (value.length() == 0 && second != nullptr) value = variantText(document[second]);
    return value;
}

JsonArray calendarCelebrations(JsonDocument &document) {
    JsonArray celebrations = document["celebrations"].as<JsonArray>();
    if (!celebrations.isNull() && celebrations.size() > 0) return celebrations;

    // Keep compatibility with the original proposed API contract.
    JsonArray commemorations = document["commemorations"].as<JsonArray>();
    return !commemorations.isNull() && commemorations.size() > 0 ? commemorations : celebrations;
}

bool sameCalendarDate(const String &requestedDate, const String &documentDate) {
    if (requestedDate == documentDate) return true;
    if (requestedDate.length() != 10 || documentDate.length() != 10) return false;
    if (requestedDate.charAt(4) != '-' || requestedDate.charAt(7) != '-') return false;
    if (documentDate.charAt(2) != '/' || documentDate.charAt(5) != '/') return false;

    String normalizedDocumentDate = documentDate.substring(6, 10) + "-" + documentDate.substring(3, 5) + "-" +
                                    documentDate.substring(0, 2);
    return requestedDate == normalizedDocumentDate;
}

void addLabeledLine(ScrollableTextArea &area, const String &label, const String &value) {
    if (value.length() > 0) area.addLine(label + ": " + value);
}

String truncateText(String text, size_t maxBytes = MAX_TEXT_FIELD) {
    if (text.length() <= maxBytes) return text;
    text.remove(maxBytes);
    text += "\n\n[conteúdo reduzido para o display]";
    return text;
}

String answerText(const String &payload) {
    JsonDocument doc;
    if (deserializeJson(doc, payload)) return payload;

    String answer = variantText(doc["answer"]);
    if (answer.length() == 0) answer = variantText(doc["message"]);
    if (answer.length() == 0) answer = variantText(doc["text"]);
    if (answer.length() == 0) answer = variantText(doc["output_text"]);
    if (answer.length() == 0 && doc["data"].is<JsonObject>()) answer = variantText(doc["data"]["answer"]);

    if (answer.length() == 0 && doc["output"].is<JsonArray>()) {
        for (JsonVariant item : doc["output"].as<JsonArray>()) {
            if (!item.is<JsonObject>()) continue;

            if (item["type"].as<String>() == "message") {
                JsonArray contents = item["content"].as<JsonArray>();
                for (JsonVariant content : contents) {
                    String contentType = content["type"].as<String>();
                    if (contentType == "output_text" || contentType == "text") {
                        answer = variantText(content["text"]);
                        if (answer.length() > 0) break;
                    }
                }
            }
            if (answer.length() == 0) answer = variantText(item["text"]);
            if (answer.length() > 0) break;
        }
    }
    return answer.length() > 0 ? answer : payload;
}

void addReading(ScrollableTextArea &area, const char *label, JsonVariant reading) {
    if (reading.isNull()) return;

    String reference = fieldText(reading, "reference", "passage");
    String translation = fieldText(reading, "translation", "version");
    area.addLine("");
    area.addLine(label);
    if (reference.length() > 0) area.addLine(reference);
    if (translation.length() > 0) area.addLine("Tradução: " + translation);

    JsonVariant content = reading["content"];
    if (content.isNull()) content = reading["text"];
    String body = variantText(content);
    if (body.length() > 0) area.addLine(truncateText(body));
}

void addReadingSet(ScrollableTextArea &area, const char *title, JsonVariant readings) {
    if (readings.isNull()) return;
    area.addLine("");
    area.addLine(title);
    addReading(area, "Primeira leitura", readings["first_reading"]);
    addReading(area, "Salmo", readings["psalm"]);
    addReading(area, "Segunda leitura", readings["second_reading"]);
    addReading(area, "Evangelho", readings["gospel"]);
}

} // namespace

void EstevaoMenu::loadConfig() { EstevaoStorage::loadConfig(_config); }

EstevaoClient EstevaoMenu::client() const { return EstevaoClient(_config); }

OpenAIClient EstevaoMenu::assistantClient() const { return OpenAIClient(_config); }

void EstevaoMenu::optionsMenu() {
    loadConfig();
    returnToMenu = false;
    options = {
        {"Hoje",             [this]() { showToday(); }                         },
        {"Coleta",           [this]() { showCollect(); }                       },
        {"Comemorações",    [this]() { commemorationsMenu(); }               },
        {"Leituras",         [this]() { readingsMenu(); }                      },
        {"Explicação",       [this]() { showExplanation(); }                   },
        {"Pergunte",         [this]() { assistantMenu(); }                     },
        {"Configurações",    [this]() { settingsMenu(); }                      },
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Estêvão");
    options.clear();
}

void EstevaoMenu::drawIcon(float scale) {
    clearIconArea();

    int bookWidth = static_cast<int>(scale * 44);
    int bookHeight = static_cast<int>(scale * 54);
    int left = iconCenterX - bookWidth / 2;
    int top = iconCenterY - bookHeight / 2;
    int spine = iconCenterX;

    tft.drawRoundRect(left, top, bookWidth, bookHeight, static_cast<int>(scale * 4), bruceConfig.priColor);
    tft.drawLine(spine, top + 2, spine, top + bookHeight - 2, bruceConfig.priColor);
    tft.drawLine(left + scale * 8, top + scale * 15, spine - scale * 6, top + scale * 15, bruceConfig.priColor);
    tft.drawLine(spine + scale * 6, top + scale * 15, left + bookWidth - scale * 8, top + scale * 15, bruceConfig.priColor);
    tft.drawLine(left + scale * 8, top + scale * 25, spine - scale * 6, top + scale * 25, bruceConfig.priColor);
    tft.drawLine(spine + scale * 6, top + scale * 25, left + bookWidth - scale * 8, top + scale * 25, bruceConfig.priColor);
}

bool EstevaoMenu::loadCalendar(bool refresh, String &date, String &payload, String &error) {
    date = "";
    payload = "";
    error = "";

    String requestedDate = EstevaoClient::todayDate();

    if (!refresh && _calendarPayload.length() > 0 &&
        (requestedDate.length() == 0 || _calendarDate == requestedDate)) {
        date = _calendarDate;
        payload = _calendarPayload;
        return true;
    }

    if (!refresh) {
        String cachedDate;
        String cachedPayload;
        if (EstevaoStorage::loadCalendar(cachedDate, cachedPayload) &&
            (requestedDate.length() == 0 || cachedDate == requestedDate)) {
            _calendarDate = cachedDate;
            _calendarPayload = cachedPayload;
            date = cachedDate;
            payload = cachedPayload;
            return true;
        }
    }

    if (refresh && requestedDate.length() > 0 && _config.hasEstevaoCredentials()) {
        EstevaoResponse response = client().getCalendar(requestedDate);
        if (response.ok && response.body.length() > 0) {
            if (!EstevaoStorage::saveCalendar(requestedDate, response.body)) {
                Serial.println("WARN: não foi possível salvar o cache do calendário");
            }
            _calendarDate = requestedDate;
            _calendarPayload = response.body;
            date = requestedDate;
            payload = response.body;
            return true;
        }
        error = response.error;
    }

    if (_calendarPayload.length() > 0 &&
        (requestedDate.length() == 0 || _calendarDate == requestedDate)) {
        date = _calendarDate;
        payload = _calendarPayload;
        return true;
    }

    if (EstevaoStorage::loadCalendar(date, payload)) {
        _calendarDate = date;
        _calendarPayload = payload;
        return true;
    }
    if (error.length() == 0) {
        error = requestedDate.length() == 0 ? "Relógio ainda não sincronizado" : "Nenhum calendário salvo";
    }
    return false;
}

bool EstevaoMenu::loadExplanation(bool refresh, String &payload, String &error) {
    payload = "";
    error = "";

    String date = EstevaoClient::todayDate();
    if (refresh && date.length() > 0 && _config.hasEstevaoCredentials()) {
        EstevaoResponse response = client().getExplanation(date);
        if (response.ok && response.body.length() > 0) {
            EstevaoStorage::saveExplanation(response.body);
            payload = response.body;
            return true;
        }
        error = response.error;
    }

    if (EstevaoStorage::loadExplanation(payload)) return true;
    if (error.length() == 0) error = "Nenhuma explicação salva";
    return false;
}

void EstevaoMenu::showText(const String &title, const String &text, size_t maxBytes) {
    ScrollableTextArea area(title);
    area.fromString(truncateText(text, maxBytes));
    area.show(true);
}

void EstevaoMenu::showResponseError(const String &error) {
    displayError(error.length() > 0 ? error : "Não foi possível concluir a consulta", true);
}

void EstevaoMenu::showToday() {
    String date, payload, error;
    if (!loadCalendar(true, date, payload, error)) {
        showResponseError(error);
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, payload)) {
        showText("Estêvão | " + date, payload);
        return;
    }

    ScrollableTextArea area("ESTÊVÃO | HOJE");
    String documentDate = documentField(doc, "date");
    String dateLabel = documentDate;
    if (documentDate.length() > 0 && !sameCalendarDate(date, documentDate)) dateLabel += " (cache)";
    addLabeledLine(area, "Data", dateLabel);
    addLabeledLine(area, "Dia", documentField(doc, "day_of_week"));
    addLabeledLine(area, "Estação", documentField(doc, "liturgical_season", "season"));
    addLabeledLine(area, "Cor", documentField(doc, "liturgical_color", "color"));
    addLabeledLine(area, "Ano litúrgico", documentField(doc, "liturgical_year"));
    addLabeledLine(area, "Domingo", documentField(doc, "sunday_name"));
    addLabeledLine(area, "Semana", documentField(doc, "week_of_season", "proper_week"));
    addLabeledLine(area, "É domingo", documentField(doc, "is_sunday"));
    addLabeledLine(area, "É dia santo", documentField(doc, "is_holy_day"));

    JsonVariant celebration = doc["celebration"];
    String celebrationName = fieldText(celebration, "name", "title");
    if (celebrationName.length() == 0) celebrationName = variantText(celebration);
    addLabeledLine(area, "Celebração", celebrationName);

    JsonArray celebrations = calendarCelebrations(doc);
    if (!celebrations.isNull()) addLabeledLine(area, "Comemorações", String(celebrations.size()));
    else if (!doc["saint"].isNull()) addLabeledLine(area, "Santo", fieldText(doc["saint"], "name", "title"));

    area.addLine("");
    area.addLine(WiFi.status() == WL_CONNECTED ? "Sincronização: online" : "Sincronização: offline");
    area.addLine("Encoder: gire para rolar; botão para voltar");
    area.show(true);
}

void EstevaoMenu::showCollect() {
    String date, payload, error;
    if (!loadCalendar(false, date, payload, error)) {
        showResponseError(error);
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, payload)) {
        showResponseError("Calendário inválido");
        return;
    }

    ScrollableTextArea area("COLETA | " + date);
    JsonArray collects = doc["collect"].as<JsonArray>();
    if (collects.isNull() || collects.size() == 0) {
        area.addLine("Nenhuma coleta disponível.");
    } else {
        for (JsonVariant collect : collects) {
            String title = fieldText(collect, "title", "module_title");
            if (title.length() > 0) area.addLine(title);
            addLabeledLine(area, "Celebração", fieldText(collect, "subtitle"));
            area.addLine(truncateText(variantText(collect["text"])));
            area.addLine("");
        }
    }
    area.show(true);
}

void EstevaoMenu::commemorationsMenu() {
    options = {
        {"Ver comemorações", [this]() { showCommemorations(); }},
        {"Voltar",            []() {}                              },
    };
    loopOptions(options, MENU_TYPE_SUBMENU, "Comemorações");
}

void EstevaoMenu::showCommemorations() {
    String date, payload, error;
    if (!loadCalendar(false, date, payload, error)) {
        showResponseError(error);
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, payload)) {
        showResponseError("Calendário inválido");
        return;
    }

    ScrollableTextArea area("COMEMORAÇÕES | " + date);
    JsonArray celebrations = calendarCelebrations(doc);
    if (!celebrations.isNull() && celebrations.size() > 0) {
        for (JsonVariant celebration : celebrations) {
            String name = fieldText(celebration, "name", "title");
            area.addLine(name.length() > 0 ? name : variantText(celebration));
            addLabeledLine(area, "Tipo", fieldText(celebration, "type"));
            addLabeledLine(area, "Cor", fieldText(celebration, "color"));
            addLabeledLine(area, "Grau", fieldText(celebration, "rank"));
            area.addLine("");
        }
    } else {
        JsonVariant saint = doc["saint"];
        String name = fieldText(saint, "name", "title");
        area.addLine(name.length() > 0 ? name : "Nenhuma comemoração exposta pela API.");
    }
    area.show(true);
}

void EstevaoMenu::readingsMenu() {
    options = {
        {"Eucaristia",       [this]() { showReadingSet("EUCARISTIA", "readings"); }        },
        {"Oração Matutina",  [this]() { showReadingSet("ORAÇÃO MATUTINA", "morning_readings"); }},
        {"Oração Vespertina",[this]() { showReadingSet("ORAÇÃO VESPERTINA", "evening_readings"); }},
        {"Voltar",           []() {}                                                       },
    };
    loopOptions(options, MENU_TYPE_SUBMENU, "Leituras");
}

void EstevaoMenu::showReadingSet(const char *title, const char *jsonKey) {
    String date, payload, error;
    if (!loadCalendar(false, date, payload, error)) {
        showResponseError(error);
        return;
    }

    JsonDocument doc;
    if (deserializeJson(doc, payload)) {
        showResponseError("Calendário inválido");
        return;
    }

    ScrollableTextArea area(String(title) + " | " + date);
    JsonVariant readings = doc[jsonKey];
    if (readings.isNull() &&
        (String(jsonKey) == "morning_readings" || String(jsonKey) == "evening_readings")) {
        // LOC 2015 may not expose a separate office set; use the day's readings.
        readings = doc["readings"];
    }
    addReadingSet(area, title, readings);
    if (area.getMaxLines() == 0) area.addLine("Nenhuma leitura disponível.");
    area.show(true);
}

void EstevaoMenu::showExplanation() {
    String payload, error;
    if (!loadExplanation(true, payload, error)) {
        showResponseError(error);
        return;
    }
    showText("EXPLICAÇÃO LITÚRGICA", payload, 18000);
}

void EstevaoMenu::assistantMenu() {
    options = {
        {"Perguntar por texto", [this]() { askByText(); }},
        {"Perguntar por voz",   [this]() { askByVoice(); }},
        {"Última resposta",    [this]() { showLastAnswer(); }},
        {"Configurações",      [this]() { settingsMenu(); }},
        {"Voltar",             []() {}},
    };
    loopOptions(options, MENU_TYPE_SUBMENU, "Pergunte");
}

void EstevaoMenu::askByText() {
    String question = keyboard("", 160, "Pergunta ao Estevão:");
    if (question == "\x1B" || question.length() == 0) return;
    if (!_config.hasAssistantCredentials()) {
        showResponseError("Configure OpenAI e MCP em Configurações");
        return;
    }

    String date = EstevaoClient::todayDate();
    if (date.length() == 0) {
        String cachedDate, cachedPayload;
        EstevaoStorage::loadCalendar(cachedDate, cachedPayload);
        date = cachedDate;
    }
    if (date.length() == 0) {
        showResponseError("Relógio ainda não sincronizado");
        return;
    }

    displayInfo("Consultando a IA...", false);
    EstevaoResponse response = assistantClient().askText(question, date);
    if (!response.ok) {
        showResponseError(response.error);
        return;
    }

    EstevaoStorage::saveAnswer(response.body);
    showText("RESPOSTA", answerText(response.body), 18000);
}

void EstevaoMenu::askByVoice() {
#if !defined(MIC_SPM1423) && !defined(MIC_INMP441)
    showResponseError("Este build não possui microfone configurado");
    return;
#else
    if (!_config.hasAssistantCredentials()) {
        showResponseError("Configure OpenAI e MCP em Configurações");
        return;
    }

    String date = EstevaoClient::todayDate();
    if (date.length() == 0) {
        showResponseError("Relógio ainda não sincronizado");
        return;
    }

    const String audioPath = "/estevao/cache/question.wav";
    uint32_t audioBytes = 0;
    bool cancelled = false;
    unsigned long startedAt = millis();
    unsigned long lastDraw = 0;

    drawMainBorderWithTitle("PERGUNTA POR VOZ");
    tft.setTextSize(FM);
    tft.setCursor(BORDER_PAD_X, BORDER_PAD_Y + 2 * LH);
    tft.println("Fale agora...");
    tft.setTextSize(FP);
    tft.setCursor(BORDER_PAD_X, BORDER_PAD_Y + 4 * LH);
    tft.println("Botão central encerra");

    bool recorded = mic_record_wav_to_path(
        &LittleFS,
        audioPath,
        _config.voiceMaxMs,
        &audioBytes,
        1.0f,
        [&]() {
            if (check(EscPress)) {
                cancelled = true;
                return false;
            }
            if (check(SelPress)) {
                cancelled = true;
                return false;
            }
            if (millis() - lastDraw > 250) {
                lastDraw = millis();
                tft.fillRect(BORDER_PAD_X, BORDER_PAD_Y + 6 * LH, tftWidth - 2 * BORDER_PAD_X, 2 * LH, bruceConfig.bgColor);
                tft.setCursor(BORDER_PAD_X, BORDER_PAD_Y + 6 * LH);
                tft.printf("%lus / %lus", (millis() - startedAt) / 1000, _config.voiceMaxMs / 1000);
            }
            return true;
        }
    );

    if (cancelled) {
        LittleFS.remove(audioPath);
        displayInfo("Gravação cancelada", true);
        return;
    }
    if (!recorded || audioBytes < 44) {
        LittleFS.remove(audioPath);
        showResponseError("Não foi possível gravar o microfone");
        return;
    }

    displayInfo("Transcrevendo e consultando IA...", false);
    EstevaoResponse response = assistantClient().askAudio(&LittleFS, audioPath, date);
    LittleFS.remove(audioPath);
    if (!response.ok) {
        showResponseError(response.error);
        return;
    }

    EstevaoStorage::saveAnswer(response.body);
    showText("RESPOSTA", answerText(response.body), 18000);
#endif
}

void EstevaoMenu::showLastAnswer() {
    String payload;
    if (!EstevaoStorage::loadAnswer(payload)) {
        showResponseError("Nenhuma resposta salva");
        return;
    }
    showText("ÚLTIMA RESPOSTA", answerText(payload), 18000);
}

void EstevaoMenu::settingsMenu() {
    options = {
        {String("API Estêvão: ") + (_config.apiKey.length() > 0 ? "configurada" : "ausente"), [this]() {
             String value = keyboard(_config.apiKey, 180, "API Estêvão:", true);
             if (value != "\x1B") {
                 _config.apiKey = value;
                 EstevaoStorage::saveConfig(_config);
             }
             settingsMenu();
         }},
        {String("OpenAI: ") + (_config.openaiApiKey.length() > 0 ? "configurada" : "ausente"), [this]() {
             String value = keyboard(_config.openaiApiKey, 220, "API OpenAI:", true);
             if (value != "\x1B") {
                 _config.openaiApiKey = value;
                 EstevaoStorage::saveConfig(_config);
             }
             settingsMenu();
         }},
        {String("MCP URL: ") + (_config.mcpServerUrl.length() > 0 ? "configurada" : "ausente"), [this]() {
             String value = keyboard(_config.mcpServerUrl, 220, "URL MCP:");
             if (value != "\x1B" && value.length() > 0) {
                 _config.mcpServerUrl = value;
                 _config.mcpServerUrl.trim();
                 while (_config.mcpServerUrl.endsWith("/")) _config.mcpServerUrl.remove(_config.mcpServerUrl.length() - 1);
                 EstevaoStorage::saveConfig(_config);
             }
             settingsMenu();
         }},
        {String("MCP token: ") + (_config.mcpToken.length() > 0 ? "configurado" : "ausente"), [this]() {
             String value = keyboard(_config.mcpToken, 220, "Token MCP:", true);
             if (value != "\x1B") {
                 _config.mcpToken = value;
                 EstevaoStorage::saveConfig(_config);
             }
             settingsMenu();
         }},
        {String("Modelo IA: ") + _config.openaiModel, [this]() {
             String value = keyboard(_config.openaiModel, 64, "Modelo OpenAI:");
             if (value != "\x1B" && value.length() > 0) {
                 _config.openaiModel = value;
                 EstevaoStorage::saveConfig(_config);
             }
             settingsMenu();
         }},
        {String("Prayer Book: ") + _config.prayerBookCode, [this]() {
             String value = keyboard(_config.prayerBookCode, 48, "Prayer Book:");
             if (value != "\x1B" && value.length() > 0) {
                 _config.prayerBookCode = value;
                 EstevaoStorage::saveConfig(_config);
             }
             settingsMenu();
         }},
        {String("Bíblia: ") + _config.bibleVersion, [this]() {
             String value = keyboard(_config.bibleVersion, 32, "Versão da Bíblia:");
             if (value != "\x1B" && value.length() > 0) {
                 _config.bibleVersion = value;
                 EstevaoStorage::saveConfig(_config);
             }
             settingsMenu();
         }},
        {"Voz: 8 segundos", [this]() {
             _config.voiceMaxMs = 8000;
             EstevaoStorage::saveConfig(_config);
             settingsMenu();
         }},
        {"HTTPS: CA embutida", []() {
             displayInfo("CA para Estêvão e OpenAI\nOverride: /estevao/ca.pem", true);
         }},
        {"Salvar e voltar", [this]() {
             EstevaoStorage::saveConfig(_config);
             backToMenu();
         }},
    };
    addOptionToMainMenu();
    loopOptions(options, MENU_TYPE_SUBMENU, "Configurações");
}
