#include <SFML/Graphics.hpp>
#include <vector>
#include <cmath>

const int WIDTH = 1200;
const int HEIGHT = 700;

const float GRAVITY = 1200.0f;
const float BOUNCE = 0.6f;
const float GROUND_AMPLITUDE = 60.0f;
const float GROUND_FREQUENCY = 0.008f;

float groundHeight(float x)
{
    return HEIGHT - 120
        + std::sin(x * GROUND_FREQUENCY) * GROUND_AMPLITUDE
        + std::sin(x * GROUND_FREQUENCY * 2) * (GROUND_AMPLITUDE * 0.4f);
}

struct Ball
{
    sf::CircleShape shape;
    sf::Vector2f velocity;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "Gravity Simulation");
    window.setFramerateLimit(60);

    // Земля
    sf::VertexArray ground(sf::LineStrip, WIDTH);
    for (int x = 0; x < WIDTH; x++)
    {
        ground[x].position = { (float)x, groundHeight(x) };
        ground[x].color = sf::Color::Green;
    }

    // Шары
    std::vector<Ball> balls;
    for (int i = 0; i < 6; i++)
    {
        Ball b;
        b.shape = sf::CircleShape(18);
        b.shape.setOrigin(18, 18);
        b.shape.setPosition(200 + i * 80, 100);
        b.shape.setFillColor(sf::Color::White);
        b.velocity = { 0.f, 0.f };
        balls.push_back(b);
    }

    sf::Clock clock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        float dt = clock.restart().asSeconds();

        for (auto& ball : balls)
        {
            // Гравитация
            ball.velocity.y += GRAVITY * dt;

            sf::Vector2f pos = ball.shape.getPosition();
            pos += ball.velocity * dt;

            float groundY = groundHeight(pos.x);

            // Столкновение с землёй
            if (pos.y + ball.shape.getRadius() > groundY)
            {
                pos.y = groundY - ball.shape.getRadius();
                ball.velocity.y *= -BOUNCE;
            }

            ball.shape.setPosition(pos);
        }

        window.clear(sf::Color(20, 20, 20));
        window.draw(ground);
        for (auto& ball : balls)
            window.draw(ball.shape);
        window.display();
    }

    return 0;
}
