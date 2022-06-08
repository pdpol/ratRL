#include "main.hpp"

Healer::Healer(float amount) : amount(amount) {
}

bool Healer::use(Actor* owner, Actor* wearer) {
    if (wearer->destructible) {
        float amountHealed = wearer->destructible->heal(amount);
        if (amountHealed > 0) {
            return Carryable::use(owner, wearer);
        }
    }
    return false;
}

