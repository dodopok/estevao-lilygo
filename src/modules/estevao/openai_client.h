#ifndef __ESTEVAO_OPENAI_CLIENT_H__
#define __ESTEVAO_OPENAI_CLIENT_H__

#include "estevao_models.h"

#include <FS.h>

class OpenAIClient {
public:
    explicit OpenAIClient(const EstevaoConfig &config) : _config(config) {}

    EstevaoResponse askText(const String &question, const String &date);
    EstevaoResponse askAudio(FS *fs, const String &audioPath, const String &date);

private:
    const EstevaoConfig &_config;

    EstevaoResponse requestJson(const String &path, const String &body);
    EstevaoResponse transcribeAudio(FS *fs, const String &audioPath);

    String responsesBody(const String &question, const String &date) const;
    String contextInput(const String &question, const String &date) const;
};

#endif
