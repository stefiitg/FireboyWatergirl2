#include <iostream>
#include <string>
using namespace std;


class Character {
protected:
    string name;
    int x, y;
    int lives;

public:
    Character(string n = "Anonim", int x0 = 0, int y0 = 0, int v = 3)
         {
        name=n;
        x=x0; y=y0;
        lives=v;

        cout << "Constructor Character apelat pentru " << name << endl;
    }

    Character(const Character& other)
        {
        name=other.name;
        x=other.x; y=other.y;
        lives=other.lives;
        cout << "Copiere Character pentru " << name << endl;
    } ///constructoru de copiere

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
    Fireboy(int x0 = 0, int y0 = 0) : Character("Fireboy", x0, y0) {} //luam la constrctor doar caracteristiciile lui character

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
        cout << "Watergirl a atins focul si pierdut o viata!" << endl;
        takeDamage();
    }
};


class Level {
private:
    Fireboy fireboy;
    Watergirl watergirl;
    int exitX, exitY;

public:
    Level(int fx = 0, int fy = 0, int wx = 5, int wy = 0, int ex = 10, int ey = 0) {
        fireboy = Fireboy(fx, fy);
        watergirl = Watergirl(wx, wy);
        exitX = ex;
        exitY = ey;

        cout << "Nivel creat! Du-i pe fireboy si pe watergirl la iesire \n";
    }

    void playRound() {
        cout << "\n--- Runda incepe ---\n";
        fireboy.moveRight();
        watergirl.moveRight();

        fireboy.touchWater();
        watergirl.touchFire();

        cout << "\nPozitii curente:\n";
        cout << fireboy << endl;
        cout << watergirl << endl;

        checkWin();
    }

    void checkWin() {
        if (fireboy.getLives() <= 0 || watergirl.getLives() <= 0) {
            cout << "Unul dintre personaje a murit. Game Over!!!!!!!\n";
            return;
        }

        if (fireboy.getX() >= exitX && watergirl.getX() >= exitX) {
            cout << " Ambii au ajuns la iesire! FELICITARI! ye\n";
        } else {
            cout << "Inca nu au ajuns la iesire.\n";
        }
    }
};


int main() {

    Level level1(0, 0, 3, 0, 10, 0);

    for (int i = 1; i <= 4; i++) {
        cout << "\nRunda " << i << ":\n";
        level1.playRound();
    }

    return 0;
}
