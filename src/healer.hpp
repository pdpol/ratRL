class Healer: public Carryable {
public:
    float amount;

    Healer(float amount);
    bool use(Actor* owner, Actor* wearer);
};
