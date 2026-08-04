#ifndef __ESTEVAO_CLIENT_H__
#define __ESTEVAO_CLIENT_H__

#include "estevao_models.h"

#include <FS.h>

class EstevaoClient {
public:
    explicit EstevaoClient(const EstevaoConfig &config) : _config(config) {}

    EstevaoResponse getCalendar(const String &date);
    EstevaoResponse getExplanation(const String &date);
    EstevaoResponse askText(const String &question, const String &date);
    EstevaoResponse askAudio(FS *fs, const String &audioPath, const String &date);

    static String todayDate();

private:
    const EstevaoConfig &_config;

    EstevaoResponse requestJson(const char *method, const String &path, const String &body = "");
    EstevaoResponse requestAudio(const String &path, FS *fs, const String &audioPath);

    String buildCalendarPath(const String &date) const;
    String buildExplanationPath(const String &date) const;
    String buildAssistantPath(const String &date) const;
    String preferencesJson() const;

    static String urlEncode(const String &value);
    static String datePath(const String &date);
};

#endif
