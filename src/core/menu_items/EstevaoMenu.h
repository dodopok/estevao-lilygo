#ifndef __ESTEVAO_MENU_H__
#define __ESTEVAO_MENU_H__

#include "MenuItemInterface.h"

#include "modules/estevao/estevao_client.h"
#include "modules/estevao/estevao_models.h"

class EstevaoMenu : public MenuItemInterface {
public:
    EstevaoMenu() : MenuItemInterface("Estêvão") {}

    void optionsMenu(void) override;
    void drawIcon(float scale) override;

    bool hasTheme() override { return false; }
    const String &themePath() override {
        static const String emptyPath = "";
        return emptyPath;
    }

private:
    EstevaoConfig _config;

    void loadConfig();
    EstevaoClient client() const;

    void todayMenu();
    void showToday();
    void showCollect();
    void commemorationsMenu();
    void showCommemorations();
    void readingsMenu();
    void showReadingSet(const char *title, const char *jsonKey);
    void showExplanation();
    void assistantMenu();
    void askByText();
    void askByVoice();
    void showLastAnswer();
    void settingsMenu();

    bool loadCalendar(bool refresh, String &date, String &payload, String &error);
    bool loadExplanation(bool refresh, String &payload, String &error);
    void showText(const String &title, const String &text, size_t maxBytes = 14000);
    void showResponseError(const String &error);
};

#endif
