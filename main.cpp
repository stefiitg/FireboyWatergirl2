// (păstrat includes și using-urile tale)
#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <cassert>

using std::string;
using std::vector;
using std::cout;
using std::endl;

enum class TileType { Empty, Solid, Fire, Water, ExitFire, ExitWater };

static string toString(TileType t) {
    switch (t) {
        case TileType::Empty: return "Empty";
        case TileType::Solid: return "Solid";
        case TileType::Fire: return "Fire";
        case TileType::Water: return "Water";
        case TileType::ExitFire: return "ExitFire";
        case TileType::ExitWater: return "ExitWater";
    }
    return "Unknown";
}

class Tile {
private:
    TileType type;
    sf::RectangleShape rect; // pt fallback
    int gx, gy;
    static constexpr float TILE_SIZE = 48.f;

    void initShape() {
        rect.setSize({TILE_SIZE, TILE_SIZE});
        rect.setPosition(gx * TILE_SIZE, gy * TILE_SIZE);
        switch (type) {
            case TileType::Empty: rect.setFillColor(sf::Color(80,80,80)); break;
            case TileType::Solid: rect.setFillColor(sf::Color(120,120,120)); break;
            case TileType::Fire: rect.setFillColor(sf::Color::Red); break;
            case TileType::Water: rect.setFillColor(sf::Color::Blue); break;
            case TileType::ExitFire: rect.setFillColor(sf::Color(255,140,0)); break;
            case TileType::ExitWater: rect.setFillColor(sf::Color(0,200,100)); break;
        }
    }

public:

    explicit Tile(TileType t , int col , int row )
        : type(t), gx(col), gy(row) {
        initShape();
    }

    Tile(const Tile& other) = default;
    Tile& operator=(const Tile& other) = default;
    ~Tile() = default;

    TileType getType() const { return type; }
    int getCol() const { return gx; }
    int getRow() const { return gy; }
    static float getSize() { return TILE_SIZE; }

    void draw(sf::RenderTarget& target) const {
        if (type != TileType::Empty) target.draw(rect);
    }

    sf::FloatRect bounds() const {
        return rect.getGlobalBounds();
    }

    friend std::ostream& operator<<(std::ostream& os, const Tile& t) {
        os << "[" << toString(t.type) << " @" << t.gx << "," << t.gy << "]";
        return os;
    }
};


class Character {
private:
    string name;
    sf::Texture texture;
    sf::Sprite sprite;
    sf::RectangleShape fallbackShape;
    bool usingTexture;
    string texturePath;            // --- nou: păstrăm calea pentru a reîncărca la copy
    sf::Vector2f position;
    sf::Vector2f velocity;
    int lives;
    bool onGround;

    float speed = 160.f;
    float jumpImpulse = 360.f;
    static constexpr float GRAVITY = 900.f;

    void initFallbackShape(const sf::Color& c, const sf::Vector2f& size) {
        fallbackShape.setSize(size);
        fallbackShape.setFillColor(c);
        fallbackShape.setPosition(position);
    }

    void tryLoadTexture() {
        if (!texturePath.empty()) {
            // încercăm să încărcăm textura — dacă eșuează, rămânem cu fallback
            if (texture.loadFromFile(texturePath)) {
                usingTexture = true;
                sprite.setTexture(texture);
                float factor = Tile::getSize() / static_cast<float>(texture.getSize().y);
                if (texture.getSize().y > 0) sprite.setScale(factor, factor);
                sprite.setPosition(position);
            } else {
                usingTexture = false;
                initFallbackShape(sf::Color::White, {Tile::getSize(), Tile::getSize()});
            }
        } else {
            usingTexture = false;
            initFallbackShape(sf::Color::White, {Tile::getSize(), Tile::getSize()});
        }
    }

public:

    Character(const string& nm, const string& texPath,
              const sf::Vector2f& pos = {0.f,0.f}, int lifeCount = 3,
              const sf::Color& fallbackColor = sf::Color::White)
        : name(nm), usingTexture(false), texturePath(texPath),
          position(pos), velocity(0.f, 0.f), lives(lifeCount), onGround(false)
    {
        tryLoadTexture();
        if (!usingTexture) {
            // aplicăm culoarea fallback cerută
            initFallbackShape(fallbackColor, {Tile::getSize(), Tile::getSize()});
            fallbackShape.setPosition(position);
        } else {
            sprite.setPosition(position);
        }
    }

    void setOnGround(bool state) {
         onGround=state;
    }

    // copy ctor: reîncarcă textura din path (dacă există) în loc să copieze internals
    Character (const Character& other)
        : name(other.name),
          texturePath(other.texturePath),
          usingTexture(false),
          position(other.position),
          velocity(other.velocity),
          lives(other.lives),
          onGround(other.onGround),
          speed(other.speed),
          jumpImpulse(other.jumpImpulse)
    {
        tryLoadTexture();
        if (!usingTexture) {
            fallbackShape = other.fallbackShape;
            fallbackShape.setPosition(position);
        } else {
            sprite.setPosition(position);
        }
    }

    Character& operator=(const Character& other) {
        if (this == &other) return *this;
        name = other.name;
        texturePath = other.texturePath;
        position = other.position;
        velocity = other.velocity;
        lives = other.lives;
        onGround = other.onGround;
        speed = other.speed;
        jumpImpulse = other.jumpImpulse;

        tryLoadTexture();
        if (!usingTexture) {
            fallbackShape = other.fallbackShape;
            fallbackShape.setPosition(position);
        } else {
            sprite.setPosition(position);
        }
        return *this;
    }

    ~Character() = default;

    const string& getName() const { return name; }
    int getLives() const { return lives; }
    sf::Vector2f getPosition() const { return position; }

    sf::FloatRect bounds() const {
        if (usingTexture) return sprite.getGlobalBounds();
        return fallbackShape.getGlobalBounds();
    }

    void setPosition(const sf::Vector2f& p) {
        position = p;
        if (usingTexture) sprite.setPosition(position);
        else fallbackShape.setPosition(position);
    }

    bool update(float dt, const sf::FloatRect& worldBounds) {

        velocity.y += GRAVITY * dt;
        position += velocity * dt;

        if (position.y + Tile::getSize() > worldBounds.top + worldBounds.height) {
            position.y = worldBounds.top + worldBounds.height - Tile::getSize();
            velocity.y = 0.f;
            onGround = true;
        } else {
            onGround = false;
        }

        if (position.x < worldBounds.left) position.x = worldBounds.left;
        if (position.x + Tile::getSize() > worldBounds.left + worldBounds.width)
            position.x = worldBounds.left + worldBounds.width - Tile::getSize();

        if (usingTexture) sprite.setPosition(position);
        else fallbackShape.setPosition(position);

        return true;
    }

    void moveLeft(float dt) { position.x -= speed * dt; if (usingTexture) sprite.setPosition(position); else fallbackShape.setPosition(position); }
    void moveRight(float dt) { position.x += speed * dt; if (usingTexture) sprite.setPosition(position); else fallbackShape.setPosition(position); }

    void jump() {
        if (onGround) {
            velocity.y = -jumpImpulse;
            onGround = false;
        }
    }

    void takeDamageAndRespawn(const sf::Vector2f& respawnPos) {
        if (lives > 0) --lives;
        setPosition(respawnPos);
        velocity = {0.f, 0.f};
        onGround = false;
    }

    void draw(sf::RenderTarget& target) const {
        if (usingTexture) target.draw(sprite);
        else target.draw(fallbackShape);
    }

    friend std::ostream& operator<<(std::ostream& os, const Character& c) {
        os << c.name << " pos=(" << (int)c.position.x << "," << (int)c.position.y << ") lives=" << c.lives;
        return os;
    }

    void setFallbackAppearance(const sf::Color& c) {
        fallbackShape.setFillColor(c);
        fallbackShape.setSize({Tile::getSize(), Tile::getSize()});
        fallbackShape.setPosition(position);
    }
};


class Map {
private:
    vector<vector<Tile>> grid;
    int width, height;

    // helper to create grid - construit in-place fiecare Tile ca sa evitam copieri temporare
    void allocateGrid(int w, int h, TileType defaultType = TileType::Empty) {
        width = w; height = h;
        grid.clear();
        grid.resize(height);
        for (int r = 0; r < height; ++r) {
            grid[r].reserve(width);
            for (int c = 0; c < width; ++c) {
                grid[r].emplace_back(defaultType, c, r);
            }
        }
    }

public:

    Map(int w = 12, int h = 8, TileType defaultType = TileType::Empty) {
        allocateGrid(w,h,defaultType);
    }

    Map(const Map& other) {
        width = other.width;
        height = other.height;
        grid.reserve(height);
        for (int r = 0; r < height; ++r) {
            vector<Tile> row;
            row.reserve(width);
            for (int c = 0; c < width; ++c) row.push_back(other.grid[r][c]);
            grid.push_back(std::move(row));
        }
    }

    Map& operator=(const Map& other) {
        if (this == &other) return *this;
        width = other.width;
        height = other.height;
        grid.clear();
        grid.reserve(height);
        for (int r = 0; r < height; ++r) {
            vector<Tile> row;
            row.reserve(width);
            for (int c = 0; c < width; ++c) row.push_back(other.grid[r][c]);
            grid.push_back(std::move(row));
        }
        return *this;
    }

    ~Map() {}

    int getWidth() const { return width; }
    int getHeight() const { return height; }

    void generateAscendingPlatforms(unsigned seed = 0) {
        for (int r=0;r<height;++r)
            for (int c=0;c<width;++c)
                grid[r][c] = Tile(TileType::Empty, c, r);

        std::mt19937 rng((seed==0)? std::random_device{}() : seed);
        std::uniform_int_distribution<int> gapDist(1,3);
        std::uniform_int_distribution<int> lengthDist(1,3);

        int currentRow = height - 2; // start near bottom
        int c = 1;
        while (c < width-1 && currentRow > 0) {
            int len = lengthDist(rng);
            for (int k = 0; k < len && c < width-1; ++k) {
                grid[currentRow][c] = Tile(TileType::Solid, c, currentRow);
                ++c;
            }
            int gap = gapDist(rng);
            c += gap;

            if (currentRow > 1) currentRow -= 1;
        }

        grid[height-2][2] = Tile(TileType::Fire, 2, height-2);
        grid[height-3][width-3] = Tile(TileType::Water, width-3, height-3);

        grid[1][width-2] = Tile(TileType::ExitFire, width-2, 1);
        grid[1][1] = Tile(TileType::ExitWater, 1, 1);
    }

    TileType getTileTypeAtGrid(int col, int row) const {
        if (col < 0 || col >= width || row < 0 || row >= height) return TileType::Solid;
        return grid[row][col].getType();
    }

    TileType getTileTypeAtWorld(float x, float y) const {
        int col = static_cast<int>(x / Tile::getSize());
        int row = static_cast<int>(y / Tile::getSize());
        return getTileTypeAtGrid(col, row);
    }

    void draw(sf::RenderTarget& target) const {
        for (int r = 0; r < height; ++r)
            for (int c = 0; c < width; ++c)
                grid[r][c].draw(target);
    }

    friend std::ostream& operator<<(std::ostream& os, const Map& m) {
        os << "Map " << m.width << "x" << m.height << "\n";
        for (int r = 0; r < m.height; ++r) {
            for (int c = 0; c < m.width; ++c) {
                os << m.grid[r][c];
            }
            os << "\n";
        }
        return os;
    }

    sf::FloatRect worldBounds() const {
        return sf::FloatRect(0.f, 0.f, width * Tile::getSize(), height * Tile::getSize());
    }

    sf::Vector2f respawnWorldPosForFire() const {
        return sf::Vector2f(Tile::getSize() * 1.f, Tile::getSize() * (height - 2));
    }
    sf::Vector2f respawnWorldPosForWater() const {
        return sf::Vector2f(Tile::getSize() * 5.f, Tile::getSize() * (height - 2));
    }
};

static bool intersects(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.intersects(b);
}


class Game {
private:
    sf::RenderWindow window;
    Map map;
    Character fireboy;
    Character watergirl;
    bool fireboyAtExit = false;
    bool watergirlAtExit = false;
    bool won = false;
    sf::Font font;
    bool fontLoaded = false; // --- nou
    sf::Text winText;


    void processInput(float dt) {

        if (!won) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) fireboy.moveLeft(dt);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) fireboy.moveRight(dt);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) fireboy.jump();

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::F)) watergirl.moveLeft(dt);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::H)) watergirl.moveRight(dt);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) watergirl.jump();
        }
    }


    void handleCollisions(Character& ch, const sf::Vector2f& respawnPos, bool& reachedExitForCharacter, TileType whatExit) {
        sf::FloatRect cb = ch.bounds();

        int leftCol = static_cast<int>(cb.left / Tile::getSize());
        int rightCol = static_cast<int>((cb.left + cb.width) / Tile::getSize());
        int topRow = static_cast<int>(cb.top / Tile::getSize());
        int bottomRow = static_cast<int>((cb.top + cb.height) / Tile::getSize());

        leftCol = std::max(leftCol, 0);
        rightCol = std::min(rightCol, map.getWidth()-1);
        topRow = std::max(topRow, 0);
        bottomRow = std::min(bottomRow, map.getHeight()-1);

        for (int r = topRow; r <= bottomRow; ++r) {
            for (int c = leftCol; c <= rightCol; ++c) {
                TileType tt = map.getTileTypeAtGrid(c, r);
                if (tt == TileType::Solid) {

                    sf::FloatRect tileRect(c * Tile::getSize(), r * Tile::getSize(), Tile::getSize(), Tile::getSize());
                    if (intersects(cb, tileRect)) {

                        float charCenterY = cb.top + cb.height * 0.5f;
                        float tileCenterY = tileRect.top + tileRect.height * 0.5f;
                        ch.setOnGround(true);
                        if (charCenterY < tileCenterY) {
                            ch.setPosition({cb.left, tileRect.top - cb.height});
                        } else {
                            ch.setPosition({cb.left, tileRect.top + tileRect.height});
                        }
                    }
                }

                if (tt == TileType::Fire) {
                    if (ch.getName() == "Watergirl") {
                        ch.takeDamageAndRespawn(respawnPos);
                        reachedExitForCharacter = false;
                        return;
                    }
                } else if (tt == TileType::Water) {
                    if (ch.getName() == "Fireboy") {
                        ch.takeDamageAndRespawn(respawnPos);
                        reachedExitForCharacter = false;
                        return;
                    }
                } else if (tt == TileType::ExitFire && ch.getName() == "Fireboy") {
                    sf::FloatRect tileRect(c * Tile::getSize(), r * Tile::getSize(), Tile::getSize(), Tile::getSize());
                    if (intersects(cb, tileRect)) {
                        reachedExitForCharacter = true;
                    }
                } else if (tt == TileType::ExitWater && ch.getName() == "Watergirl") {
                    sf::FloatRect tileRect(c * Tile::getSize(), r * Tile::getSize(), Tile::getSize(), Tile::getSize());
                    if (intersects(cb, tileRect)) {
                        reachedExitForCharacter = true;
                    }
                }
            }
        }

    }


    void update(float dt) {
        if (won) return;

        sf::FloatRect world = map.worldBounds();

        fireboy.update(dt, world);
        watergirl.update(dt, world);

        handleCollisions(fireboy, map.respawnWorldPosForFire(), fireboyAtExit, TileType::ExitFire);
        handleCollisions(watergirl, map.respawnWorldPosForWater(), watergirlAtExit, TileType::ExitWater);

        if (fireboyAtExit && watergirlAtExit) {
            won = true;
            if (fontLoaded) {
                winText.setString("WIN");
                sf::FloatRect tb = winText.getLocalBounds();
                winText.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
                winText.setPosition(window.getSize().x/2.f, window.getSize().y/2.f - 20.f);
            }
        }
    }

    void render() {
        window.clear(sf::Color(40,40,40));
        map.draw(window);
        fireboy.draw(window);
        watergirl.draw(window);
        if (won && fontLoaded) window.draw(winText);
        window.display();
    }

public:

    Game(int mapW = 14, int mapH = 9)
        : window(sf::VideoMode( (unsigned)(mapW * Tile::getSize()), (unsigned)(mapH * Tile::getSize()) ), "Fireboy & Watergirl"),
          map(mapW, mapH),
          fireboy("Fireboy", "Desktop/fireboy.png", {Tile::getSize()*1.f, Tile::getSize()*(mapH-2.f)}, 3, sf::Color::Red),
          watergirl("Watergirl", "Desktop/watergirl.png", {Tile::getSize()*5.f, Tile::getSize()*(mapH-2.f)}, 3, sf::Color::Blue)
    {

        map.generateAscendingPlatforms(/*seed*/ 12345);

        if (!font.loadFromFile("Desktop/arial.ttf")) {
            cout << "Warning: font 'Desktop/arial.ttf' not found. Win text will not be displayed.\n";
            fontLoaded = false;
        } else {
            fontLoaded = true;
            winText.setFont(font);
            winText.setCharacterSize(48);
            winText.setFillColor(sf::Color::Yellow);
            winText.setStyle(sf::Text::Bold);
        }

        fireboy.setFallbackAppearance(sf::Color::Red);
        watergirl.setFallbackAppearance(sf::Color::Blue);
    }

    friend std::ostream& operator<<(std::ostream& os, const Game& g) {
        os << "Game state:\n";
        os << "Map: " << g.map.getWidth() << "x" << g.map.getHeight() << "\n";
        os << "Fireboy: " << g.fireboy << "\n";
        os << "Watergirl: " << g.watergirl << "\n";
        os << "Won: " << (g.won ? "true" : "false") << "\n";
        return os;
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            sf::Event ev;
            while (window.pollEvent(ev)) {
                if (ev.type == sf::Event::Closed) window.close();
            }

            float dt = clock.restart().asSeconds();
            processInput(dt);
            update(dt);
            render();
        }
    }
};


int main() {
    Game game(14, 9);
    cout << game << endl;
    game.run();
    return 0;
}
