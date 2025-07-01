#include <iostream>
#include <thread>
#include <vector>
#include <cmath>
#include <random>
#include <mutex>
#include <chrono>
#include <cstdio>
#include <memory>

using namespace std;

const int V = 5;
const int delayMs = 100;
const int R = 100;
const int circleSteps = 72;

std::mutex print_mutex;
std::mt19937 rng(std::random_device{}());

struct Point {
    float x, y;
};

// Базовий клас
class Ant {
protected:
    Point position;
    string name;
public:
    Ant(Point pos, const string& n) : position(pos), name(n) {}
    virtual void move() = 0;
    virtual ~Ant() = default;
};

// Мураха-робочий
class WorkerAnt : public Ant {
    Point home;
public:
    WorkerAnt(Point pos, int id) : Ant(pos, ""), home(pos) {
        char buffer[100];
        sprintf_s(buffer, sizeof(buffer), "Murakha-robochyi #%d", id);
        name = buffer;
    }

    void move() override {
        Point target = { 0, 0 };

        for (int i = 0; i < 2; ++i) {
            Point dest = (i == 0) ? target : home;
            float dx = dest.x - position.x;
            float dy = dest.y - position.y;
            float dist = sqrt(dx * dx + dy * dy);
            float vx = dx / dist * V;
            float vy = dy / dist * V;

            while (sqrt(pow(dest.x - position.x, 2) + pow(dest.y - position.y, 2)) > V) {
                position.x += vx;
                position.y += vy;

                {
                    lock_guard<mutex> lock(print_mutex);
                    cout << name << " rukhaietsia do (" << dest.x << ", " << dest.y
                        << ") -> pozytsiia: (" << position.x << ", " << position.y << ")" << endl;
                }

                this_thread::sleep_for(chrono::milliseconds(delayMs));
            }

            position = dest;
            {
                lock_guard<mutex> lock(print_mutex);
                cout << name << " prybuv do (" << position.x << ", " << position.y << ")" << endl;
            }
        }
    }
};

// Мураха-воїн
class WarriorAnt : public Ant {
public:
    WarriorAnt(Point center, int id) : Ant(center, "") {
        char buffer[100];
        sprintf_s(buffer, sizeof(buffer), "Murakha-voin #%d", id);
        name = buffer;
    }

    void move() override {
        float angle = 0;
        int revolutions = 1;

        for (int i = 0; i < circleSteps * revolutions; ++i) {
            float rad = angle * 3.14159265f / 180.0f;
            position.x = R * cos(rad);
            position.y = R * sin(rad);

            {
                lock_guard<mutex> lock(print_mutex);
                cout << name << " (po kolu) pozytsiia: (" << position.x << ", " << position.y << ")" << endl;
            }

            angle += 360.0f / circleSteps;
            if (angle >= 360.0f) angle -= 360.0f;

            this_thread::sleep_for(chrono::milliseconds(delayMs));
        }

        lock_guard<mutex> lock(print_mutex);
        cout << name << " zavershyv obertannia po kolu." << endl;
    }
};

int main() {
    int numWorkers = 2;
    int numWarriors = 2;

    vector<thread> threads;
    vector<unique_ptr<WorkerAnt>> workers;
    vector<unique_ptr<WarriorAnt>> warriors;

    // Створення мурах-робочих
    for (int i = 0; i < numWorkers; ++i) {
        Point start = { float(rand() % 200 + 200), float(rand() % 200 + 200) };
        auto ant = make_unique<WorkerAnt>(start, i + 1);
        threads.emplace_back(&WorkerAnt::move, ant.get());
        workers.push_back(std::move(ant));
    }

    // Створення мурах-воїнів
    for (int i = 0; i < numWarriors; ++i) {
        Point center = { 0, 0 };
        auto ant = make_unique<WarriorAnt>(center, i + 1);
        threads.emplace_back(&WarriorAnt::move, ant.get());
        warriors.push_back(std::move(ant));
    }

    // Очікування завершення потоків
    for (auto& t : threads) {
        t.join();
    }

    cout << "\nSymuliatsiia zavershena!" << endl;
    return 0;
}
