class Carryable {
public:
    virtual ~Carryable() {};
    bool pickUp(Actor* owner, Actor* wearer);
    void drop(Actor* owner, Actor* wearer);
    virtual bool use(Actor* owner, Actor* wearer);
};

class LightningBolt: public Carryable {
public:
    float range;
    float damage;
    LightningBolt(float range, float damage);
    bool use(Actor* owner, Actor* wearer);
};

class Fireball: public LightningBolt {
public:
    Fireball(float range, float damage);
    bool use(Actor* owner, Actor* wearer);
};

class Confuser : public Carryable {
public:
    int numTurns;
    float range;
    Confuser(int numTurns, float range);
    bool use(Actor* owner, Actor* wearer);
};
