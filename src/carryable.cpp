#include "main.hpp"

// i don't really like the use of owner and wearer here
// it kind of feels like wearer is the owner, and owner is the carryable...
bool Carryable::pickUp(Actor* owner, Actor* wearer) {
    if (wearer->container && wearer->container->add(owner)) {
        engine.actors.remove(owner);
        return true;
    }
    return false;
}

void Carryable::drop(Actor* owner, Actor* wearer) {
    if (wearer->container) {
        wearer->container->remove(owner);
        engine.actors.push(owner);
        owner->x = wearer->x;
        owner->y = wearer->y;
        engine.gui->message(TCODColor::lightGrey, "%s drops a %s.", wearer->name, owner->name);
    }
}

bool Carryable::use(Actor* owner, Actor* wearer) {
    if (wearer->container) {
        wearer->container->remove(owner);
        delete owner;
        return true;
    }
    return false;
}

LightningBolt::LightningBolt(float range, float damage) : range(range), damage(damage) {
}

bool LightningBolt::use(Actor* owner, Actor* wearer) {
    Actor* closestMonster = engine.getClosestMonster(wearer->x, wearer->y, range);
    if (!closestMonster) {
        engine.gui->message(TCODColor::lightGrey, "No enemy is close enough to strike.");
        return false;
    }

    engine.gui->message(TCODColor::lightBlue,
            "A lightning bolt strikes the %s with a loud thunder!\n"
            "The damage is %g hit points.",
            closestMonster->name, damage);
    closestMonster->destructible->takeDamage(closestMonster, damage);

    return Carryable::use(owner, wearer);
}

Fireball::Fireball(float range, float damage) : LightningBolt(range, damage) {
}

bool Fireball::use(Actor* owner, Actor* wearer) {
    printf("using fireball");
    engine.gui->message(TCODColor::cyan, "Left-click a target tile for the fireball,\nor right-click to cancel.");
    int x;
    int y;
    if (!engine.pickATile(&x, &y)) {
        return false;
    }

    engine.gui->message(TCODColor::orange, "The fireball explodes, burning everything within %g tiles!", range);

    for (auto actor : engine.actors) {
        if (actor->destructible && !actor->destructible->isDead() && actor->getDistance(x, y) <= range) {
            engine.gui->message(TCODColor::orange, "The %s gets burned for %g hit points.", actor->name, damage); 
            actor->destructible->takeDamage(actor,damage);
        }
    }
    return Carryable::use(owner, wearer);
}

Confuser::Confuser(int numTurns, float range) : numTurns(numTurns), range(range) {
}

bool Confuser::use(Actor* owner, Actor* wearer) {
    engine.gui->message(TCODColor::cyan, "Left-click an enemy to confuse it,\nor  right-click to cancel.");
    int x;
    int y;
    if (!engine.pickATile(&x, &y, range)) {
        return false;
    }

    Actor* actor = engine.getActor(x, y);
    if (!actor) {
        return false;
    }

    Ai* confusedAi = new ConfusedMonsterAi(numTurns, actor->ai);
    actor->ai = confusedAi;
    engine.gui->message(TCODColor::lightGreen, "The eyes of the %s look vacant,\nas he starts to stumble around!", actor->name);
    return Carryable::use(owner, wearer);
}
