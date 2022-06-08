class Actor {
    public:
        int x;
        int y;
        int ch;
        // in order to allow this to be mutable, should be:
        // char name [MAX_NAME_LENGTH]
        const char* name;
        TCODColor color;
        bool blocks;
        Attacker* attacker;
        Destructible* destructible;
        Ai* ai;
        Carryable* carryable;
        Container* container;

        Actor(int x, int y, int ch, const char* name, const TCODColor &color);
        ~Actor();
        void update();
        void render() const;
        float getDistance(int cx, int cy) const;
};
