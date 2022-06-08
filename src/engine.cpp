#include "main.hpp"

Engine::Engine(int screenWidth, int screenHeight) : gameStatus(STARTUP), fovRadius(10), screenWidth(screenWidth), screenHeight(screenHeight) {
    TCODConsole::initRoot(screenWidth, screenHeight, "Vermin", false);
    player = new Actor(40, 25, 'r', "player", TCODColor::white);
    player->destructible = new PlayerDestructible(30, 2, "your cadaver");
    player->attacker = new Attacker(5);
    player->ai = new PlayerAi();
    player->container = new Container(26);
    actors.push(player);
    map = new Map(80, 45);
    gui = new Gui();

    gui->message(TCODColor::red, "Welcome stranger!\nPrepare to perish in the Tombs of the Ancient Kings.");
}

Engine::~Engine() {
    actors.clearAndDelete();
    delete map;
    delete gui;
}

void Engine::update() {
    if (gameStatus == STARTUP) map->computeFov();
    gameStatus = IDLE;
    TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE, &lastKey, &mouse);
    player->update();
    if (gameStatus == NEW_TURN) {
        for (auto actor: actors) {
            if (actor != player) {
                actor->update();
            }
        }
    }
}

void Engine::render() {
    TCODConsole::root->clear();
    map->render();

    for (auto actor : actors) {
        if (actor != player && map->isInFov(actor->x, actor->y)) {
            actor->render();
        }
    }
    player->render();
    gui->render();
    TCODConsole::root->print(1, screenHeight - 2, "HP: %d/%d", (int)player->destructible->hp, (int)player->destructible->maxHp);
}

void Engine::sendToBack(Actor* actor) {
    actors.remove(actor);
    actors.insertBefore(actor, 0);
}

Actor* Engine::getClosestMonster(int x, int y, float range) const {
    Actor* closest = NULL;
    float bestDistance = 1E6f;

    for (auto actor : actors) {
        if (actor != player && actor->destructible && !actor->destructible->isDead()) {
            float distance = actor->getDistance(x, y);
            if (distance < bestDistance && (distance <= range || range == 0.0f)) {
                bestDistance = distance;
                closest = actor;
            }
        }
    }
    return closest;
}

Actor* Engine::getActor(int x, int y) const {
    for (auto actor : actors) {
        if (actor->x == x && actor->y == y && actor->destructible && !actor->destructible->isDead()) {
            return actor;
        }
    }
    return NULL;
}

bool Engine::pickATile(int* x, int* y, float maxRange) {
    while (!TCODConsole::isWindowClosed()) {
        printf("picking a tile\n");
        printf("lastkey before render: %c\n", lastKey.c);
        render();
        printf("lastkey after render: %c\n", lastKey.c);

        // highlight range
        for (int cx = 0; cx < map->width; cx++) {
            for (int cy = 0; cy < map->height; cy++) {
                if (map->isInFov(cx, cy) && (maxRange == 0 || player->getDistance(cx, cy) <= maxRange)) {
                    TCODColor color = TCODConsole::root->getCharBackground(cx, cy);
                    color = color * 1.2f;
                    TCODConsole::root->setCharBackground(cx, cy, color);
                }
            }
        }
        TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE, &lastKey, &mouse);

        if (map->isInFov(mouse.cx, mouse.cy) && (maxRange == 0 || player->getDistance(mouse.cx, mouse.cy) <= maxRange)) {
            TCODConsole::root->setCharBackground(mouse.cx, mouse.cy, TCODColor::white);

            if (mouse.lbutton_pressed) {
                *x = mouse.cx;
                *y = mouse.cy;
                return true;
            }
        }

        // bizarre hack - TCODK_TEXT seems to have some weird behavior that break the check
        // against TCODK_NONE. i can see in the tcod history that text caused problems for
        // pythont tutorial users in the past and so it was changed to not use key.c (to key.text),
        // but obviously that doesn't help if you're looking for vk specifically.
        if (mouse.rbutton_pressed || (lastKey.vk != TCODK_NONE && lastKey.vk != TCODK_TEXT)) {
            return false;
        }
        TCODConsole::flush();
    }
    return false;
}
