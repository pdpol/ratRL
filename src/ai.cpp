#include <math.h>
#include "main.hpp"

static const int TRACKING_TURNS = 3;

MonsterAi::MonsterAi() : moveCount(0) {
}

void PlayerAi::update(Actor* owner) {
    if (owner->destructible && owner->destructible->isDead()) {
        return;
    }

    int dx = 0;
    int dy = 0;
    switch(engine.lastKey.vk) {
        case TCODK_UP: dy = -1; break;
        case TCODK_DOWN: dy = 1; break;
        case TCODK_LEFT: dx = -1; break;
        case TCODK_RIGHT: dx = 1; break;
        case TCODK_CHAR: handleActionKey(owner, engine.lastKey.c); break;
        default: break;
    }

    if (dx !=0 || dy != 0) {
        engine.gameStatus = Engine::NEW_TURN;
        if (moveOrAttack(owner, owner->x + dx, owner->y + dy)) {
            engine.map->computeFov();
        }
    }
}

bool PlayerAi::moveOrAttack(Actor* owner, int xTarget, int yTarget) {
    if (engine.map->isWall(xTarget, yTarget)) return false;

    // attack living actors
    for (auto actor : engine.actors) {
        if (actor->destructible && !actor->destructible->isDead() && actor->x == xTarget && actor->y == yTarget) {
            owner->attacker->attack(owner, actor);
            return false;
        }
    }

    // items or corpses
    // small optimization - pluck these out in the first loop and store in a variable?
    // maybe actors is usually small enough the overhead for that isn't worth it
    for (auto actor : engine.actors) {
        bool corpseOrItem = (actor->destructible && actor->destructible->isDead()) || actor->carryable;
        if (corpseOrItem && actor->x == xTarget && actor->y == yTarget) {
            engine.gui->message(TCODColor::lightGrey, "There's a %s here.\n", actor->name);
        }
    }

    owner->x = xTarget;
    owner->y = yTarget;
    return true;
}

void PlayerAi::handleActionKey(Actor* owner, int ascii) {
    switch (ascii) {
        // TODO: game crash if inventory empty!
        case 'd': // drop item
        {
            Actor* actor = chooseFromInventory(owner);
            if (actor) {
                actor->carryable->drop(actor, owner);
                engine.gameStatus = Engine::NEW_TURN;
            }
        }
        break;
        case 'g': // pick up item
        {
            bool found=false;
            for (auto actor : engine.actors) {
                if (actor->carryable && actor->x == owner->x && actor->y == owner->y) {
                        if (actor->carryable->pickUp(actor, owner)) {
                            found = true;
                            engine.gui->message(TCODColor::lightGrey, "You pick up the %s.", actor->name);
                            break;
                        } else if (!found) {
                            found = true;
                            engine.gui->message(TCODColor::red, "Your inventory is full.");
                        }
                }
            }
            if (!found) {
                engine.gui->message(TCODColor::lightGrey, "There's nothing here that you can pick up.");
            }
            engine.gameStatus = Engine::NEW_TURN;
        }
        break;
        case 'i': // display inventory
        {
            Actor* actor = chooseFromInventory(owner);
            if (actor) {
                actor->carryable->use(actor, owner);
                engine.gameStatus = Engine::NEW_TURN;
            }
        }
        break;
    }
}

Actor* PlayerAi::chooseFromInventory(Actor* owner) {
    static const int INVENTORY_WIDTH = 50;
    static const int INVENTORY_HEIGHT = 28;
    static TCODConsole console(INVENTORY_WIDTH, INVENTORY_HEIGHT);

    console.setDefaultForeground(TCODColor(200, 180, 50));
    console.printFrame(0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT, true, TCOD_BKGND_DEFAULT, "inventory");

    console.setDefaultForeground(TCODColor::white);
    int shortcut = 'a';
    int y = 1;
    for (auto actor : owner->container->inventory) {
        console.print(2, y, "(%c) %s", shortcut, actor->name); 
        shortcut++;
        y++;
    }

    TCODConsole::blit(&console, 0, 0, INVENTORY_WIDTH, INVENTORY_HEIGHT,
            TCODConsole::root, engine.screenWidth / 2 - INVENTORY_WIDTH / 2,
            engine.screenHeight / 2 - INVENTORY_HEIGHT / 2);
    TCODConsole::flush();

    TCOD_key_t key;
    TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);

    if (key.vk == TCODK_CHAR) {
        int actorIndex = key.c - 'a';
        if (actorIndex >= 0 && actorIndex < owner->container->inventory.size()) {
            return owner->container->inventory.get(actorIndex);
        }

        return NULL;
    }
}

void MonsterAi::update(Actor* owner) {
    if (owner->destructible && owner->destructible->isDead()) {
        return;

    }

    if (engine.map->isInFov(owner->x, owner->y) ) {
        moveCount = TRACKING_TURNS;
    } else {
        moveCount--;
    }

    if (moveCount > 0) {
        moveOrAttack(owner, engine.player->x, engine.player->y);
    }
}

void MonsterAi::moveOrAttack(Actor* owner, int xTarget, int yTarget) {
    int dx = xTarget - owner->x;
    int dy = yTarget - owner->y;
    int stepdx = (dx > 0 ? 1 : -1);
    int stepdy = (dy > 0 ? 1 : -1);
    float distance = sqrtf(dx * dx + dy * dy);

    if (distance >= 2) {
        dx = (int)(round(dx/distance));
        dy = (int)(round(dy/distance));

        if (engine.map->canWalk(owner->x + dx, owner->y + dy)) {
            owner->x += dx;
            owner->y += dy;
        } else if (engine.map->canWalk(owner->x + stepdx, owner->y)) {
            owner->x += stepdx;
        } else if (engine.map->canWalk(owner->x, owner->y + stepdy)) {
            owner->y += stepdy;
        }
    } else if (owner->attacker) {
        owner->attacker->attack(owner, engine.player);
    }
}

ConfusedMonsterAi::ConfusedMonsterAi(int numTurns, Ai* oldAi) : numTurns(numTurns), oldAi(oldAi) {
}

void ConfusedMonsterAi::update(Actor* owner) {
    TCODRandom* rng = TCODRandom::getInstance();
    int dx = rng->getInt(-1, 1);
    int dy = rng->getInt(-1, 1);

    if (dx != 0 || dy != 0) {
        int xDest = owner->x + dx;
        int yDest = owner->y + dy;
        if (engine.map->canWalk(xDest, yDest)) {
            owner->x = xDest;
            owner->y = yDest;
        } else {
            Actor* actor = engine.getActor(xDest, yDest);
            if (actor) {
                owner->attacker->attack(owner, actor);
            }
        }
    }
    numTurns--;
    if (numTurns == 0) {
        owner->ai = oldAi;
        delete this;
    }
}
