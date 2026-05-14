#pragma once

#include "Cell.h"
#include "../utils/Point.h"

#include <vector>

enum class GameState : std::uint8_t
{
    NewGame = 0,
    Playing = 1,
    Win = 2,
    Lose = 3
};

class Map
{
    int m_Columns;
    int m_Rows;
    int m_Mines;

    // Column then row
    std::vector<std::vector<Cell>> m_Grid;

    GameState m_State;

public:
    Map(int columns, int rows, int mines);

    GameState getState() const;
    Cell getCell(int x, int y) const;
    std::vector<Point> getAdjacentPoints(Point position) const;

    void reset();
    void openSquare(Point position);
    void flagSquare(Point position);

private:
    bool hasWon() const;
    bool isInBounds(Point position) const;

    void generateMines(Point position);
    void recursiveOpen(Point start);
};
