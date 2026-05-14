#pragma once

struct Cell
{
    bool mine = false;
    bool open = false;
    bool flagged = false;
    int adjacentMines = 0;
};
