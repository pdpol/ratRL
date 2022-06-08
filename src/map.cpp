#include "main.hpp"

static const int ROOM_MAX_SIZE = 12;
static const int ROOM_MIN_SIZE = 6;
static const int MAX_ROOM_MONSTERS = 3;
static const int MAX_ROOM_ITEMS = 2;

class BspListener : public ITCODBspCallback {
private:
    Map& map;
    int roomNum;
    int lastx;
    int lasty;
public:
    BspListener(Map& map) : map(map), roomNum(0) {}

    bool visitNode(TCODBsp* node, void* userData) {
        if (node->isLeaf()) {
            int x, y, width, height;
            // dig a room
            TCODRandom* rng = TCODRandom::getInstance();
            width = rng->getInt(ROOM_MIN_SIZE, node->w - 2);
            height = rng->getInt(ROOM_MIN_SIZE, node->h - 2);
            x = rng->getInt(node->x + 1, node->x + node->w - width - 1);
            y = rng->getInt(node->y + 1, node->y + node->h - height - 1);
            map.createRoom(roomNum == 0, x, y, x + width - 1, y + height - 1);

            if (roomNum != 0) {
                // dig a corridor from last room
                map.dig(lastx, lasty, x + width / 2, lasty);
                map.dig(x + width / 2, lasty, x + width / 2, y + height / 2);
            }

            lastx = x + width / 2;
            lasty = y + height / 2;
            roomNum++;
        }

        return true;
    }
};

Map::Map(int width, int height) : width(width), height(height) {
    tiles = new Tile[width * height];
    map = new TCODMap(width, height);
    TCODBsp bsp(0, 0, width, height);
    bsp.splitRecursive(NULL, 8, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
    BspListener listener(*this);
    bsp.traverseInvertedLevelOrder(&listener, NULL);
}

Map::~Map() {
    delete [] tiles;
    delete map;
}

void Map::addItem(int x, int y) {
    TCODRandom* rng = TCODRandom::getInstance();
    int dice = rng->getInt(0, 100);
    if (dice < 70) {
        // health potion
        Actor* healthPotion = new Actor(x, y, '!', "health potion", TCODColor::violet);
        healthPotion->blocks = false;
        healthPotion->carryable = new Healer(4);
        engine.actors.push(healthPotion);
    } else if (dice < 70 + 10) {
        // scroll of lightning bolt
        Actor* scrollOfLightningBolt = new Actor(x, y, '?', "scroll of lightning bolt", TCODColor::lightYellow);
        scrollOfLightningBolt->blocks = false;
        scrollOfLightningBolt->carryable = new LightningBolt(5, 20);
        engine.actors.push(scrollOfLightningBolt);
    } else if (dice < 70 + 10 + 10) {
        // scroll of fireball
        Actor* scrollOfFireball = new Actor(x, y, '?', "scroll of fireball", TCODColor::lightYellow);
        scrollOfFireball->blocks = false;
        scrollOfFireball->carryable = new Fireball(3, 12);
        engine.actors.push(scrollOfFireball);
    } else {
        Actor* scrollOfConfusion = new Actor(x, y, '?', "scroll of confusion", TCODColor::lightYellow);
        scrollOfConfusion->blocks = false;
        scrollOfConfusion->carryable = new Confuser(10, 8);
        engine.actors.push(scrollOfConfusion);
    }
}

void Map::addMonster(int x, int y) {
    TCODRandom* rng = TCODRandom::getInstance();
    if (rng->getInt(0, 100) < 80) {
        Actor* orc = new Actor(x, y, 'o', "orc", TCODColor::desaturatedGreen);
        orc->destructible = new MonsterDestructible(10, 0, "dead orc");
        orc->attacker = new Attacker(3);
        orc->ai = new MonsterAi();

        engine.actors.push(orc);
    } else {
        Actor* troll = new Actor(x, y, 'T', "troll", TCODColor::darkerGreen);
        troll->destructible = new MonsterDestructible(16, 1, "troll carcass");
        troll->attacker = new Attacker(4);
        troll->ai = new MonsterAi();
        engine.actors.push(troll);
    }
}

bool Map::canWalk(int x, int y) const {
    if (isWall(x, y)) {
        return false;
    }

    for (auto actor : engine.actors) {
        if (actor->blocks && actor->x == x && actor->y == y) {
            return false;
        }
    }
    return true;
}

// This logic will likely apply to more than just walls. "Impassable" really
bool Map::isWall(int x, int y) const {
    return !map->isWalkable(x, y);
}

bool Map::isExplored(int x, int y) const {
    return tiles[x + y * width].explored;
}

bool Map::isInFov(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return false;
    }

    if (map->isInFov(x, y)) {
        tiles[x + y * width].explored = true;
        return true;
    }

    return false;
}

void Map::computeFov() {
    map->computeFov(engine.player->x, engine.player->y, engine.fovRadius);
}

void Map::dig(int x1, int y1, int x2, int y2) {
    if (x2 < x1) {
        int tmp = x2;
        x2 = x1;
        x1 = tmp;
    }

    if (y2 < y1) {
        int tmp = y2;
        y2 = y1;
        y1 = tmp;
    }

    for (int xTile = x1; xTile <= x2; xTile++) {
        for (int yTile = y1; yTile <= y2; yTile++) {
            map->setProperties(xTile, yTile, true, true);
        }
    }
}

void Map::createRoom(bool first, int x1, int y1, int x2, int y2) {
    dig(x1, y1, x2, y2);
    if (first) {
        engine.player->x = (x1 + x2) / 2;
        engine.player->y = (y1 + y2) / 2;
    } else {
        TCODRandom* rng = TCODRandom::getInstance();
        int numMonsters = rng->getInt(0, MAX_ROOM_MONSTERS);
        while (numMonsters > 0) {
            int x = rng->getInt(x1, x2);
            int y = rng->getInt(y1, y2);
            if (canWalk(x, y)) {
                addMonster(x, y);
            }
            numMonsters--;
        }

        int numItems = rng->getInt(0, MAX_ROOM_ITEMS);
        while (numItems > 0) {
            int x = rng->getInt(x1, x2);
            int y = rng->getInt(y1, y2);
            if (canWalk(x, y)) {
                addItem(x, y);
            }
            numItems--;
        }
    }
}

// Read docs on these colors. Probably going to one a single place to define all colors in the game?
void Map::render() const {
    static const TCODColor darkWall(0, 0, 100);
    static const TCODColor darkGround(50, 50, 150);
    static const TCODColor lightWall(130, 110, 50);
    static const TCODColor lightGround(200, 180, 50);

    for (int x=0; x < width; x++) {
        for (int y=0; y < height; y++) {
            if (isInFov(x, y)) {
                TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? lightWall : lightGround);
            } else if (isExplored(x, y)) {
                TCODConsole::root->setCharBackground(x, y, isWall(x, y) ? darkWall : darkGround);
            }
        }
    }
}
