class Gui {
public:
    Gui();
    ~Gui();
    void render();
    void message(const TCODColor &color, const char* text, ...);

protected:
    TCODConsole* console;
    struct Message {
        char* text;
        TCODColor color;
        Message(const char* text, const TCODColor &color);
        ~Message();
    };
    TCODList<Message *> log;

    void renderBar(int x, int y, int width, const char* name, float value, float maxValue, const TCODColor &barColor, const TCODColor &bgColor);
    void renderMouseLook();
};
