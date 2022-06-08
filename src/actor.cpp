#include <math.h>
#include <stdio.h>
#include "main.hpp"

Actor::Actor(int x, int y, int ch, const char* name, const TCODColor &color) :
    // for mutable name, use { strcpy(this->name, name) }
    x(x), y(y), ch(ch), name(name), color(color),
    blocks(true), attacker(NULL), destructible(NULL),
    ai(NULL), carryable(NULL), container(NULL) {
}

Actor::~Actor() {
    if (attacker) delete attacker;
    if (destructible) delete destructible;
    if (ai) delete ai;
    if (carryable) delete carryable;
    if (container) delete container;
}
void Actor::update() {
    if (ai) ai->update(this);
}

void Actor::render() const {
    TCODConsole::root->setChar(x, y, ch);
    TCODConsole::root->setCharForeground(x, y, color);
}

float Actor::getDistance(int cx, int cy) const {
    int dx = x - cx;
    int dy = y - cy;
    return sqrtf(dx * dx + dy * dy);
}
