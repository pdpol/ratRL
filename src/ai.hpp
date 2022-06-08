class Ai {
public:
    virtual ~Ai() {};
    virtual void update(Actor* owner) = 0;   
};

class MonsterAi : public Ai {
public:
    MonsterAi();
    void update(Actor* owner);

protected:
    int moveCount;

    void moveOrAttack(Actor* owner, int xTarget, int yTarget);
};

class ConfusedMonsterAi : public Ai {
public:
    ConfusedMonsterAi(int numTurns, Ai* oldAi);
    void update(Actor* owner);
protected:
    int numTurns;
    Ai* oldAi;
};

class PlayerAi : public Ai {
public:
    void update(Actor* owner);

protected:
    bool moveOrAttack(Actor* owner, int xTarget, int yTarget);
    void handleActionKey(Actor* owner, int ascii);
    Actor* chooseFromInventory(Actor* owner);
};
