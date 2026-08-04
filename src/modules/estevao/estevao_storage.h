#ifndef __ESTEVAO_STORAGE_H__
#define __ESTEVAO_STORAGE_H__

#include "estevao_models.h"

namespace EstevaoStorage {

static constexpr const char *CONFIG_PATH = "/estevao/config.json";
static constexpr const char *CA_PATH = "/estevao/ca.pem";
static constexpr const char *TODAY_PATH = "/estevao/cache/today.json";
static constexpr const char *TODAY_DATE_PATH = "/estevao/cache/today-date.txt";
static constexpr const char *EXPLANATION_PATH = "/estevao/cache/explanation.json";
static constexpr const char *ANSWER_PATH = "/estevao/cache/last-answer.json";

bool loadConfig(EstevaoConfig &config);
bool saveConfig(const EstevaoConfig &config);

bool saveText(const char *path, const String &text);
bool loadText(const char *path, String &text, size_t maxBytes = 160 * 1024);

bool saveCalendar(const String &date, const String &payload);
bool loadCalendar(String &date, String &payload);

bool saveExplanation(const String &payload);
bool loadExplanation(String &payload);

bool saveAnswer(const String &payload);
bool loadAnswer(String &payload);

} // namespace EstevaoStorage

#endif
