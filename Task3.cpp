#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace sf;
using namespace std;

const int WIDTH = 800;
const int HEIGHT = 600;
const int GRID_SIZE = 20;
const int GRID_WIDTH = WIDTH / GRID_SIZE;
const int GRID_HEIGHT = HEIGHT / GRID_SIZE;

struct SnakeSegment {
    int x, y;
};

class SnakeGame {
private:
    RenderWindow window;
    vector<SnakeSegment> snake;
    Vector2i food;
    Vector2i direction;
    bool directionChanged;
    int score;
    int speed;
    bool gameOver;
    bool paused;

    Font font;
    Text scoreText;
    Text gameOverText;
    Text pauseText;
    RectangleShape foodShape;
    vector<RectangleShape> snakeShapes;
    SoundBuffer eatBuffer;
    SoundBuffer gameOverBuffer;
    Sound eatSound;
    Sound gameOverSound;

    void init() {
        window.create(VideoMode(WIDTH, HEIGHT), "Snake Game");
        window.setFramerateLimit(60);

        snake = {{GRID_WIDTH / 2, GRID_HEIGHT / 2}};
        direction = {1, 0};
        directionChanged = false;
        score = 0;
        speed = 10;
        gameOver = false;
        paused = false;

        srand(static_cast<unsigned>(time(nullptr)));

        if (!font.loadFromFile("Roboto-Regular.ttf")) {
            cerr << "Failed to load font Roboto-Regular.ttf\n";
        }

        scoreText.setFont(font);
        scoreText.setCharacterSize(24);
        scoreText.setFillColor(Color::White);
        scoreText.setPosition(10, 10);

        gameOverText.setFont(font);
        gameOverText.setCharacterSize(48);
        gameOverText.setFillColor(Color::Red);
        gameOverText.setString("GAME OVER!\nPress R to restart");
        FloatRect textRect = gameOverText.getLocalBounds();
        gameOverText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        gameOverText.setPosition(WIDTH/2.0f, HEIGHT/2.0f);

        pauseText.setFont(font);
        pauseText.setCharacterSize(48);
        pauseText.setFillColor(Color::Yellow);
        pauseText.setString("PAUSED\nPress P to resume");
        textRect = pauseText.getLocalBounds();
        pauseText.setOrigin(textRect.left + textRect.width/2.0f, textRect.top + textRect.height/2.0f);
        pauseText.setPosition(WIDTH/2.0f, HEIGHT/2.0f);

        foodShape.setSize(Vector2f(GRID_SIZE-2, GRID_SIZE-2));
        foodShape.setFillColor(Color::Red);
        placeFood();

        if (!eatBuffer.loadFromFile("eat.wav")) {
            cerr << "Failed to load eat.wav\n";
        }
        if (!gameOverBuffer.loadFromFile("gameover.wav")) {
            cerr << "Failed to load gameover.wav\n";
        }
        eatSound.setBuffer(eatBuffer);
        gameOverSound.setBuffer(gameOverBuffer);
    }

    void placeFood() {
        bool onSnake;
        do {
            onSnake = false;
            food.x = rand() % GRID_WIDTH;
            food.y = rand() % GRID_HEIGHT;

            for (const auto& segment : snake) {
                if (segment.x == food.x && segment.y == food.y) {
                    onSnake = true;
                    break;
                }
            }
        } while (onSnake);
    }

    void handleInput() {
        Event event;
        while (window.pollEvent(event)) {
            if (event.type == Event::Closed)
                window.close();

            if (event.type == Event::KeyPressed && !directionChanged) {
                if (event.key.code == Keyboard::Up && direction.y != 1) {
                    direction = {0, -1};
                    directionChanged = true;
                }
                else if (event.key.code == Keyboard::Down && direction.y != -1) {
                    direction = {0, 1};
                    directionChanged = true;
                }
                else if (event.key.code == Keyboard::Left && direction.x != 1) {
                    direction = {-1, 0};
                    directionChanged = true;
                }
                else if (event.key.code == Keyboard::Right && direction.x != -1) {
                    direction = {1, 0};
                    directionChanged = true;
                }
                else if (event.key.code == Keyboard::R && gameOver) {
                    reset();
                }
                else if (event.key.code == Keyboard::P) {
                    paused = !paused;
                }
            }
        }
    }

    void update() {
        if (gameOver || paused) return;

        static int frameCounter = 0;
        if (++frameCounter < 60 / speed) return;
        frameCounter = 0;

        SnakeSegment newHead = {snake[0].x + direction.x, snake[0].y + direction.y};
        snake.insert(snake.begin(), newHead);
        directionChanged = false;

        if (snake[0].x == food.x && snake[0].y == food.y) {
            score += 10;
            eatSound.play();
            placeFood();
            if (score % 50 == 0 && speed < 20) {
                speed += 2;
            }
        } else {
            snake.pop_back();
        }

        if (snake[0].x < 0 || snake[0].x >= GRID_WIDTH || snake[0].y < 0 || snake[0].y >= GRID_HEIGHT) {
            gameOver = true;
            gameOverSound.play();
        }

        for (size_t i = 1; i < snake.size(); ++i) {
            if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
                gameOver = true;
                gameOverSound.play();
                break;
            }
        }

        scoreText.setString("Score: " + to_string(score) + "\nSpeed: " + to_string(speed));

        snakeShapes.clear();
        for (size_t i = 0; i < snake.size(); ++i) {
            RectangleShape segment(Vector2f(GRID_SIZE - 2, GRID_SIZE - 2));
            segment.setPosition(snake[i].x * GRID_SIZE + 1, snake[i].y * GRID_SIZE + 1);

            float ratio = static_cast<float>(i) / snake.size();
            segment.setFillColor(Color(50 + ratio * 205, 205 - ratio * 100, 50));

            snakeShapes.push_back(segment);
        }
    }

    void render() {
        window.clear(Color(30, 30, 30));

        for (int x = 0; x < WIDTH; x += GRID_SIZE) {
            for (int y = 0; y < HEIGHT; y += GRID_SIZE) {
                RectangleShape cell(Vector2f(GRID_SIZE - 1, GRID_SIZE - 1));
                cell.setPosition(x, y);
                cell.setFillColor(Color(50, 50, 50));
                window.draw(cell);
            }
        }

        foodShape.setPosition(food.x * GRID_SIZE + 1, food.y * GRID_SIZE + 1);
        window.draw(foodShape);

        for (const auto& segment : snakeShapes) {
            window.draw(segment);
        }

        window.draw(scoreText);
        if (gameOver) window.draw(gameOverText);
        else if (paused) window.draw(pauseText);

        window.display();
    }

    void reset() {
        snake = {{GRID_WIDTH / 2, GRID_HEIGHT / 2}};
        direction = {1, 0};
        score = 0;
        speed = 10;
        gameOver = false;
        placeFood();
    }

public:
    SnakeGame() {
        init();
    }

    void run() {
        while (window.isOpen()) {
            handleInput();
            update();
            render();
        }
    }
};

int main() {
    SnakeGame game;
    game.run();
    return 0;
}
