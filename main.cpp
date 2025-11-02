#include <iostream>
#include <string>
#include <vector>
using namespace std;


class Character {
protected:
    string name;
    int x, y;
    int lives;

public:
    Character(string n = "Anonim", int x0 = 0, int y0 = 0, int v = 3) {
        name = n;
        x = x0;
        y = y0;
        lives = v;

        cout << "Constructor Character apelat pentru " << name << endl;
    }

    Character(const Character& other) {
        name = other.name;
        x = other.x;
        y = other.y;
        lives = other.lives;
        cout << "Copiere Character pentru " << name << endl;
    }///constructoru de copiere

    virtual ~Character() {
        cout << "Destructor Character apelat pentru " << name << endl;
    }

    virtual void moveLeft() { x--; }
    virtual void moveRight() { x++; }
    virtual void jump() { y += 2; }
    virtual void takeDamage() {
        if (lives > 0) lives--;
    }

    int getX() const { return x; }
    int getY() const { return y; }
    int getLives() const { return lives; }
    string getName() const { return name; }


    friend ostream& operator<<(ostream& os, const Character& c) {
        os << c.name << " este la (" << c.x << ", " << c.y
           << ") si are " << c.lives << " vieti.";
        return os;
    }
};


class Fireboy : public Character {
public:
    Fireboy(int x0 = 0, int y0 = 0) : Character("Fireboy", x0, y0) {}//luam la constrctor doar caracteristiciile lui character


    void touchFire() {
        cout << "Fireboy este imun la foc." << endl;
    }

    void touchWater() {
        cout << "Fireboy a atins apa si a pierdut o viata!" << endl;
        takeDamage();
    }
};

class Watergirl : public Character {
public:
    Watergirl(int x0 = 0, int y0 = 0) : Character("Watergirl", x0, y0) {}

    void touchWater() {
        cout << "Watergirl este imuna la apa." << endl;
    }

    void touchFire() {
        cout << "Watergirl a atins focul si a pierdut o viata!" << endl;
        takeDamage();
    }
};


class Tile {
    string type; // posibile sunt doar "empty" "fire" "water" "exit"
public:
    Tile(string t = "empty") : type(t) {}

    string getType() const { return type; }


    void interact(Character& c) const {
        if (type == "fire") {
            if (c.getName() == "Watergirl")
                ((Watergirl&)c).touchFire();
            else
                cout << c.getName() << " trece prin foc fara probleme.\n";
        }
        else if (type == "water") {
            if (c.getName() == "Fireboy")
                ((Fireboy&)c).touchWater();
            else
                cout << c.getName() << " trece prin apa fara probleme.\n";
        }
        else if (type == "exit") {
            cout << c.getName() << " a ajuns la iesire!\n";
        }
    }

    friend ostream& operator<<(ostream& os, const Tile& t) {
        os << t.type;
        return os;
    }
};


class Level {
private:
    Fireboy fireboy;
    Watergirl watergirl;
    int width, height;
    vector<vector<Tile>> map; // harta de tile uri

public:
    Level(int fx = 0, int fy = 0, int wx = 5, int wy = 0, int w = 10, int h = 1)
        : fireboy(fx, fy), watergirl(wx, wy), width(w), height(h) {

        // inițializăm harta cu un rând de 10 tile uri goale
        map = vector<vector<Tile>>(height, vector<Tile>(width, Tile("empty")));

        // plasăm foc apa si exit
        map[0][3] = Tile("fire");
        map[0][6] = Tile("water");
        map[0][9] = Tile("exit");

        cout << "Nivel creat! Du- l pe fireboy si pe watergirl la iesire!\n";
    }

    void printMap() const {
        cout << "Harta nivelului:\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x)
                cout << "[" << map[y][x] << "]";
            cout << endl;
        }
    }

    void playRound() {
        cout << "\n--- Runda incepe ---\n";

        fireboy.moveRight();
        watergirl.moveRight();

        // luam tile ul pe care calca
        if (fireboy.getX() < width)
            map[fireboy.getY()][fireboy.getX()].interact(fireboy);

        if (watergirl.getX() < width)
            map[watergirl.getY()][watergirl.getX()].interact(watergirl);

        cout << "\nPozitii curente:\n";
        cout << fireboy << endl;
        cout << watergirl << endl;

        checkWin();
    }

    void checkWin() {
        if (fireboy.getLives() <= 0 || watergirl.getLives() <= 0) {
            cout << "Unul dintre personaje a murit. Game Over!\n";
            return;
        }

        if (fireboy.getX() >= width - 1 && watergirl.getX() >= width - 1) {
            cout << "Ambii au ajuns la iesire! FELICITARI!\n";
        } else {
            cout << "Inca nu au ajuns la iesire.\n";
        }
    }
};


int main() {
    Level level1(0, 0, 3, 0, 10, 1);
    level1.printMap();

    for (int i = 1; i <= 5; i++) {
        cout << "\nRunda " << i << ":\n";
        level1.playRound();
    }

    return 0;
}
