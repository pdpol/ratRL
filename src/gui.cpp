#include "main.hpp"

static const int PANEL_HEIGHT = 7;
static const int BAR_WIDTH = 20;

static const int MSG_X = BAR_WIDTH + 2;
static const int MSG_HEIGHT = PANEL_HEIGHT - 1;

Gui::Gui() {
    console = new TCODConsole(engine.screenWidth, PANEL_HEIGHT);
}

Gui::~Gui() {
    delete console;
    log.clearAndDelete();
}

void Gui::render() {
    console->setDefaultBackground(TCODColor::black);
    console->clear();
    renderBar(1, 1, BAR_WIDTH, "HP", engine.player->destructible->hp, engine.player->destructible->maxHp, TCODColor::lightRed, TCODColor::darkerRed);

    // draw the message log
    // check and see if there's a better way to handle this second y iterator
    int y = 1;
    float colorCoef=0.4f;
    for (auto message : log) {
        console->setDefaultForeground(message->color * colorCoef);
        console->print(MSG_X, y, message->text);
        y++;

        if (colorCoef < 1.0f) {
            colorCoef += 0.3f;
        }
    }

    renderMouseLook();

    TCODConsole::blit(console, 0, 0, engine.screenWidth, PANEL_HEIGHT, TCODConsole::root, 0, engine.screenHeight - PANEL_HEIGHT);
}

void Gui::renderBar(int x, int y, int width, const char* name, float value, float maxValue, const TCODColor &barColor, const TCODColor &bgColor) {
    console->setDefaultBackground(bgColor);
    console->rect(x, y, width, 1, false, TCOD_BKGND_SET);

    int barWidth = (int)(value / maxValue * width);
    if (barWidth > 0) {
        console->setDefaultBackground(barColor);
        console->rect(x, y, barWidth, 1, false, TCOD_BKGND_SET);
    }

    console->setDefaultForeground(TCODColor::white);
    console->printEx(x + width / 2, y, TCOD_BKGND_NONE, TCOD_CENTER, "%s : %g/%g", name, value, maxValue);
}

Gui::Message::Message(const char* text, const TCODColor& color) : text(strdup(text)), color(color) {
}

Gui::Message::~Message() {
    free(text);
}

void Gui::message(const TCODColor& color, const char* text, ...) {
    va_list ap;
    char buf[128];
    va_start(ap, text);
    vsprintf(buf, text, ap);
    va_end(ap);

    char* lineBegin = buf;
    char* lineEnd;

    do {
        if (log.size() == MSG_HEIGHT) {
            Message* toRemove = log.get(0);
            log.remove(toRemove);
            delete toRemove;
        }

        lineEnd = strchr(lineBegin, '\n');

        if (lineEnd) {
            *lineEnd = '\0';
        }

        Message* message = new Message(lineBegin, color);
        log.push(message);
        lineBegin = lineEnd + 1;
    } while (lineEnd);

}

void Gui::renderMouseLook() {
    if (!engine.map->isInFov(engine.mouse.cx, engine.mouse.cy)) {
        return;
    }

    char buf[128] = "";
    bool first = true;
    for (auto actor : engine.actors) {
        if (actor->x == engine.mouse.cx && actor->y == engine.mouse.cy) {
            if (!first) {
                strcat(buf, ", ");
            } else {
                first = false;
            }
            strcat(buf, actor->name);
        }
    }

    console->setDefaultForeground(TCODColor::lightGrey);
    console->print(1, 0, buf);
}
