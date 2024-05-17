#include "lib/console.h"
#include <cstddef>
#include <curses.h>
#include <ncurses.h>
#include <chrono>
#include <thread>
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>

class Frame
{
public:
    Frame() {}

    void limit()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(MS_PER_FRAME));
    }
private:
    static constexpr long MS_PER_FRAME { 75 };
};

enum class UserInput { None, Left, Right, Up, Down, Quit };

class Input
{
public:
    Input(const Console& _console)
    {
        console = _console;
    }

    UserInput Read()
    {
        int i = console.read();
        switch (i)
        {
            case 'q': return UserInput::Quit;
            case KEY_LEFT: return UserInput::Left;
            case KEY_RIGHT: return UserInput::Right;
            case KEY_UP: return UserInput::Up;
            case KEY_DOWN: return UserInput::Down;
            default: return UserInput::None;
        }
    }

private:
    Console console{};
};

class Player
{
public:
    Player(int _xMax, int _yMax) 
    {
        xMax = _xMax;
        yMax = _yMax;
    }

    void Update(UserInput userInput)
    {
        if (userInput == UserInput::Left)
        {
            speed[0] = -1;
            speed[1] = 0;
        }
        else if (userInput == UserInput::Right)
        {
            speed[0] = 1;
            speed[1] = 0;
        }
        else if (userInput == UserInput::Up)
        {
            speed[0] = 0;
            speed[1] = -1;
        }
        else if (userInput == UserInput::Down)
        {
            speed[0] = 0;
            speed[1] = 1;
        }
            

        if ((speed[0] > 0 && x < xMax) || (speed[0] < 0 && x > 0))
            x += speed[0];
        if ((speed[1] > 0 && y < yMax) || (speed[1] < 0 && y > 0))
            y += speed[1];
    }

    static const char ascii = '@';

    int GetX() const { return x; }
    int GetY() const { return y; }


private:
    int x{0};
    int y{0};
    int xMax{};
    int yMax{};
    std::vector<int> speed{1 ,0};
};

class Pellet
{
public:
    Pellet(int _width, int _height)
    {
        width = _width;
        height = _height;
    }

    bool Update(const Player& player)
    {
        if (!spawned)
        {
            x = std::rand() % width;
            y = std::rand() % height;
            spawned = true;
        }

        if (spawned && player.GetX() == x && player.GetY() == y)
        {
            spawned = false;
            return true;
        }
        
        return false;
    }

    int GetX() { return x; }
    int GetY() { return y; }
    
    static const char ascii = 'O';
    

private:    
    int x{};
    int y{};
    int width{};
    int height{};
    bool spawned{};
};

class Game
{
public:
    Game(int _width, int _height) : 
        player(_width-1, _height-1),
        pellet(_width, _height)
    {
        width = _width;
        height = _height;
        map = std::vector(height, std::string(width, empty));
        std::srand(std::time(nullptr));
    }

    void Update(UserInput userInput)
    {
        map[player.GetY()][player.GetX()] = empty;
        player.Update(userInput);
        map[player.GetY()][player.GetX()] = player.ascii;

        if (!pellet.Update(player))
            map[pellet.GetY()][pellet.GetX()] = pellet.ascii;
        else
            score++;
    }

    const std::vector<std::string>& GetMap() { return map; }
    int GetScore() { return score; }

private:
    static constexpr char empty = ' ';
    int width;
    int height;
    int score{};
    std::vector<std::string> map{};
    Player player;
    Pellet pellet;
}; 

class Render
{
public:
    Render(const Console& _console)
    {
        console = _console;
    }

    void Draw(const std::vector<std::string>& map)
    {
        for (int row = 0; row < map.size(); row++)
        {
            console.moveCursor(row, 0);
            console.print(map[row]);
        }
    }

private:
    Console console{};
};

int main()
{
    Frame frame{};
    Console console{};
    Render render{console};
    Game game{console.width, console.height};
    Input input{console};
    
    while (1)
    {
        frame.limit();

        UserInput userInput = input.Read();

        if (userInput == UserInput::Quit) break;

        game.Update(userInput);

        render.Draw(game.GetMap());
    }

    console.moveCursor(0, 0);
    console.print("You scored " + std::to_string(game.GetScore()));

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    return 0;
}