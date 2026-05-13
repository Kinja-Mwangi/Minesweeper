#include "Map.h"

#include <algorithm>

// To be used by the recursiveOpen function soon
constexpr Point* deltas = {
    Point{ -1, -1 }, // Top left
    Point{  0, -1 }, // Top centre
    Point{  1, -1 }, // Top right
    Point{  1,  0 }, // Middle right
    Point{  1,  1 }, // Bottom right
    Point{  0,  1 }, // Bottom centre
    Point{ -1,  1 }, // Bottom left
    Point{ -1,  0 }, // Middle left
};

Map::Map(int columns, int rows, int mines)
{
    m_Columns = columns;
    m_Rows = rows;
    m_Mines = mines;

    m_Grid.resize(columns);

    for(auto& column : m_Grid)
    {
        column.resize(rows);
    }

    reset(); // Good enough for now
}

void Map::getState() const
{
    return m_State;
}

void Map::getCell(int x, int y) const
{
    return m_Grid[x, y];
}

std::vector<Point> getAdjacentPoints(Point position) const
{
    std::vector<Point> points(8);

    for (int i = 0; i <= 7; i++)
    {
        if (isInBounds(position + deltas[i]))
            points.emplace_back(position + delta);
    }

    return points;
}

void Map::reset()
{
    for (auto& column : m_Grid)
    {
        for (auto& cell : column)
        {
            cell = Cell();
        }
    }

    m_State = GameState::NewGame;
}

void Map::openSquare(Point position)
{
    if (m_State == GameState::Playing)
        generateMines(position);

    auto gridSquare = m_Grid[position.x][position.y];
    if (gridSquare.open || gridSquare.flagged) return;

    m_Grid[position.x][position.y].open = true;

    if (!gridSquare.mine && gridSquare.adjacentMines == 0)
    {
        recursiveOpen(position);
    }
    else if (gridSquare.mine)
    {
        m_State == GameState::Lose;
    }

    if (hasWon()) m_State = GameState::Win;
}

void Map::flagSquare(Point position)
{
    m_Grid[position.x][position.y].flagged = !m_Grid[position.x][position.y].flagged;
}

bool Map::hasWon() const;
{
    for (auto& column : m_Grid)
    {
        for (auto& cell : column)
        {
            if (!cell.mine && !cell.open)
                return false;
        }
    }

    return true;
}

bool Map::isInBounds(Point position) const;
{
    return position.x >= 0 && position.X < m_Columns &&
           position.y >= 0 && position.y < m_Rows;
}

void Map::generateMines(Point position);
{
    for (int i = 0; i < m_Mines; i++)
    {
        int column;
        int row;
        std::vector<Point> adjacent;

        do
        {
            column = m_Random.getRandom(m_Columns);
            row = m_Random.getRandom(m_Rows);
            adjacent = getAdjacentPoints(position);
        } while (m_Grid[column][row].mine || (column == position.x && row == position.y) ||
                 std::find(adjacent.begin(), adjacent.end(), Point(column, row)) != adjacent.end())
        
        m_Grid[column][row].mine = true;

        for (auto p : getAdjacentPoints({column, row}))
        {
            m_Grid[column][row].adjacentMines++;
        }
    }

    m_State = GameState::Playing;
}

void Map::recursiveOpen(Point start);
{
    for (auto& point : getAdjacentPoints(start))
    {
        auto square = m_Grid[point.x][point.y];

        if (!isInBounds(point) || square.open || square.mine || square.flagged) continue;

        m_Grid[point.x][point.y].open = true;

        if (square.Mine)
        {
            m_State = GameState::Lose;
            break;
        }

        if (square.adjacentMines == 0)
        {
            recursiveOpen(point);
        }
    }
}
