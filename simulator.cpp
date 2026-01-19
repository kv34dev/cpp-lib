#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <memory>
#include <algorithm>
#include <chrono>
#include <thread>
#include <iomanip>

// Вектор 2D для физических вычислений
struct Vec2 {
    double x, y;
    
    Vec2(double x = 0, double y = 0) : x(x), y(y) {}
    
    Vec2 operator+(const Vec2& v) const { return Vec2(x + v.x, y + v.y); }
    Vec2 operator-(const Vec2& v) const { return Vec2(x - v.x, y - v.y); }
    Vec2 operator*(double s) const { return Vec2(x * s, y * s); }
    Vec2 operator/(double s) const { return Vec2(x / s, y / s); }
    
    double length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalize() const { 
        double len = length();
        return len > 0 ? Vec2(x / len, y / len) : Vec2(0, 0);
    }
    double dot(const Vec2& v) const { return x * v.x + y * v.y; }
};

// Базовый класс для частиц
class Particle {
protected:
    Vec2 pos, vel, acc;
    double mass, radius;
    int charge;
    
public:
    Particle(Vec2 p, Vec2 v, double m, double r, int c = 0) 
        : pos(p), vel(v), acc(0, 0), mass(m), radius(r), charge(c) {}
    
    virtual ~Particle() = default;
    
    virtual void update(double dt) {
        vel = vel + acc * dt;
        pos = pos + vel * dt;
        acc = Vec2(0, 0);
    }
    
    void applyForce(const Vec2& force) {
        acc = acc + force / mass;
    }
    
    Vec2 getPos() const { return pos; }
    Vec2 getVel() const { return vel; }
    double getMass() const { return mass; }
    double getRadius() const { return radius; }
    int getCharge() const { return charge; }
    
    virtual char getSymbol() const { return 'o'; }
};

// Тяжёлая частица (с гравитацией)
class HeavyParticle : public Particle {
public:
    HeavyParticle(Vec2 p, Vec2 v, double m, double r) 
        : Particle(p, v, m, r, 0) {}
    
    char getSymbol() const override { return 'O'; }
};

// Заряженная частица (электромагнетизм)
class ChargedParticle : public Particle {
public:
    ChargedParticle(Vec2 p, Vec2 v, double m, double r, int c) 
        : Particle(p, v, m, r, c) {}
    
    char getSymbol() const override { return charge > 0 ? '+' : '-'; }
};

// Квадродерево для оптимизации столкновений
struct QuadTreeNode {
    Vec2 center;
    double size;
    std::vector<Particle*> particles;
    std::unique_ptr<QuadTreeNode> children[4];
    
    QuadTreeNode(Vec2 c, double s) : center(c), size(s) {}
    
    bool contains(const Vec2& p) const {
        return std::abs(p.x - center.x) <= size && 
               std::abs(p.y - center.y) <= size;
    }
    
    void insert(Particle* p) {
        if (!contains(p->getPos())) return;
        
        if (particles.size() < 4 || size < 1.0) {
            particles.push_back(p);
        } else {
            if (!children[0]) subdivide();
            for (auto& child : children) child->insert(p);
        }
    }
    
    void subdivide() {
        double hs = size / 2;
        children[0].reset(new QuadTreeNode(Vec2(center.x - hs, center.y - hs), hs));
        children[1].reset(new QuadTreeNode(Vec2(center.x + hs, center.y - hs), hs));
        children[2].reset(new QuadTreeNode(Vec2(center.x - hs, center.y + hs), hs));
        children[3].reset(new QuadTreeNode(Vec2(center.x + hs, center.y + hs), hs));
    }
};

// Основной симулятор
class PhysicsSimulator {
private:
    std::vector<std::unique_ptr<Particle>> particles;
    double width, height;
    double gravity, coulomb, damping;
    std::mt19937 rng;
    
public:
    PhysicsSimulator(double w, double h) 
        : width(w), height(h), gravity(100.0), coulomb(5000.0), damping(0.99) {
        rng.seed(std::random_device{}());
    }
    
    void addParticle(std::unique_ptr<Particle> p) {
        particles.push_back(std::move(p));
    }
    
    void update(double dt) {
        // Гравитация и электростатика
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                Vec2 diff = particles[j]->getPos() - particles[i]->getPos();
                double dist = diff.length();
                
                if (dist < 0.1) continue;
                
                Vec2 dir = diff.normalize();
                
                // Гравитация (притяжение)
                double gravForce = gravity * particles[i]->getMass() * 
                                   particles[j]->getMass() / (dist * dist);
                particles[i]->applyForce(dir * gravForce);
                particles[j]->applyForce(dir * (-gravForce));
                
                // Кулоновская сила (отталкивание/притяжение)
                if (particles[i]->getCharge() != 0 && particles[j]->getCharge() != 0) {
                    double coulombForce = coulomb * particles[i]->getCharge() * 
                                          particles[j]->getCharge() / (dist * dist);
                    particles[i]->applyForce(dir * (-coulombForce));
                    particles[j]->applyForce(dir * coulombForce);
                }
            }
        }
        
        // Обновление позиций и обработка столкновений
        for (auto& p : particles) {
            p->update(dt);
            
            Vec2 pos = p->getPos();
            Vec2 vel = p->getVel();
            
            // Столкновения со стенами
            if (pos.x < p->getRadius()) {
                p->applyForce(Vec2((p->getRadius() - pos.x) * 1000, 0));
            }
            if (pos.x > width - p->getRadius()) {
                p->applyForce(Vec2((width - p->getRadius() - pos.x) * 1000, 0));
            }
            if (pos.y < p->getRadius()) {
                p->applyForce(Vec2(0, (p->getRadius() - pos.y) * 1000));
            }
            if (pos.y > height - p->getRadius()) {
                p->applyForce(Vec2(0, (height - p->getRadius() - pos.y) * 1000));
            }
        }
        
        // Столкновения между частицами
        for (size_t i = 0; i < particles.size(); ++i) {
            for (size_t j = i + 1; j < particles.size(); ++j) {
                Vec2 diff = particles[j]->getPos() - particles[i]->getPos();
                double dist = diff.length();
                double minDist = particles[i]->getRadius() + particles[j]->getRadius();
                
                if (dist < minDist) {
                    Vec2 dir = diff.normalize();
                    double overlap = minDist - dist;
                    
                    particles[i]->applyForce(dir * (-overlap * 500));
                    particles[j]->applyForce(dir * (overlap * 500));
                }
            }
        }
    }
    
    void render() const {
        // Очистка экрана
        std::cout << "\033[2J\033[H";
        
        // Создание буфера для рендеринга
        const int w = static_cast<int>(width);
        const int h = static_cast<int>(height);
        std::vector<std::vector<char>> buffer(h, std::vector<char>(w, ' '));
        
        // Отрисовка границ
        for (int x = 0; x < w; ++x) {
            buffer[0][x] = '-';
            buffer[h-1][x] = '-';
        }
        for (int y = 0; y < h; ++y) {
            buffer[y][0] = '|';
            buffer[y][w-1] = '|';
        }
        
        // Отрисовка частиц
        for (const auto& p : particles) {
            Vec2 pos = p->getPos();
            int x = static_cast<int>(pos.x);
            int y = static_cast<int>(pos.y);
            
            if (x >= 0 && x < w && y >= 0 && y < h) {
                buffer[y][x] = p->getSymbol();
            }
        }
        
        // Вывод буфера
        for (const auto& row : buffer) {
            for (char c : row) std::cout << c;
            std::cout << '\n';
        }
        
        std::cout << "\nЧастиц: " << particles.size() 
                  << " | FPS: ~60 | ESC для выхода\n";
    }
    
    void generateRandomParticles(int count) {
        std::uniform_real_distribution<> posX(5, width - 5);
        std::uniform_real_distribution<> posY(5, height - 5);
        std::uniform_real_distribution<> vel(-20, 20);
        std::uniform_int_distribution<> type(0, 2);
        
        for (int i = 0; i < count; ++i) {
            Vec2 p(posX(rng), posY(rng));
            Vec2 v(vel(rng), vel(rng));
            
            int t = type(rng);
            if (t == 0) {
                addParticle(std::unique_ptr<Particle>(new Particle(p, v, 1.0, 0.5, 0)));
            } else if (t == 1) {
                addParticle(std::unique_ptr<Particle>(new HeavyParticle(p, v, 5.0, 1.0)));
            } else {
                int charge = (i % 2 == 0) ? 1 : -1;
                addParticle(std::unique_ptr<Particle>(new ChargedParticle(p, v, 1.0, 0.5, charge)));
            }
        }
    }
};

int main() {
    const double WIDTH = 80;
    const double HEIGHT = 30;
    const double DT = 0.016; // ~60 FPS
    
    PhysicsSimulator sim(WIDTH, HEIGHT);
    
    std::cout << "=== Симулятор частиц с физикой ===\n";
    std::cout << "Генерация 30 случайных частиц...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    sim.generateRandomParticles(30);
    
    // Основной игровой цикл
    for (int frame = 0; frame < 600; ++frame) {
        auto start = std::chrono::high_resolution_clock::now();
        
        sim.update(DT);
        sim.render();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        // Ограничение FPS
        int sleep_time = 16 - elapsed.count();
        if (sleep_time > 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
        }
    }
    
    std::cout << "\nСимуляция завершена!\n";
    return 0;
}
