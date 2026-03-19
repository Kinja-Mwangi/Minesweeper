#include "mbed.h"
#include <cstdlib>
#include <time.h>
#include <ctype.h> 
// #include "TextLCD.h"

int RNG(int lB, int uB) // lB inclusive, uB exclusive
{
    return (rand() % uB) + lB; 
}

class Map
{
    private : class Cell
    {
        public : char _type;
        public : char _symbol;
        public : int _mineCount = 0;

        public : Cell()
        {
            _type = 'E';
            _symbol = 'o';
        }

        public : Cell(char type, char symbol, int pos)
        {
            _type = type;
            _symbol = symbol;
        }

        public : void Open()
        {
            if (_type == 'E')
            {
                _symbol = _mineCount + 48;
            }

            else if (_type == 'M')
            {
                _symbol = 'X';
            }
        }

        public : void Flag()
        {
            _symbol = 'F';
        }

        public : void UnFlag()
        {
            _symbol = 'o'; // because you can only flag unopened cells
        }
    };

    public : int _size;
    int _mineCount;
    Cell* _grid;
    int _pos = 0;
    bool _gameSet = false;

    public : Map(int size, int minMines, int maxMines)
    {
        _size = size;
        _mineCount = RNG(minMines, maxMines + 1);
        _grid = new Cell[_size * _size];
    }

    // public : void Reset()
    // {
    //     for (int i = 0; i < _size * _size; i++)
    //     {
    //         _grid[i] = Cell('E', 'o');
    //     }
    // }

    public : int* GetNeighbours(int pos)
    {
        static int neighbours[8];

        int count = 0;

        if (pos % _size != 0) // if not on left column
        {
            if (pos >= _size) // if not on top row
            {
                neighbours[count] = pos - (_size + 1); // top left
                count++;
            }

            neighbours[count] = pos - 1; // centre left
            count++;

            if (pos < (_size * (_size - 1))) // if not on bottom row
            {
                neighbours[count] = pos + (_size - 1); // bottom left
                count++;
            }
        }

        if ((pos + 1) % _size != 0) // if not on right column
        {
            if (pos >= _size) // if not on top row
            {
                neighbours[count] = pos - (_size - 1); // top right
                count++;
            }

            neighbours[count] = pos + 1; // centre right
            count++;

            if (pos < (_size * (_size - 1))) // if not on bottom row
            {
                neighbours[count] = pos + (_size + 1); // bottom right
                count++;
            }
        }

        if (pos >= _size) // if not on top row
        {
            neighbours[count] = pos - _size; // top centre
            count++;
        }

        if (pos < (_size * (_size - 1))) // if not on bottom row
        {
            neighbours[count] = pos + _size; // bottom centre
            count++;
        }

        for (int i = count; i < 8; i++)
        {
            // neighbours[i] = _pos;
            neighbours[i] = (_size * _size) + 1; // dodgey fix - fills the remaining neighbour cells with positions beyond the bounds of the grid which therefore technically don't exist (should hopefully work for my purposes)
        }

        return neighbours;
    }

    public : void PlaceMines()
    {
        for (int i = 0; i < _mineCount; i++)
        {
            int randCell;
            bool valid;

            do
            {
                valid = true;
                randCell = RNG(0, _size * _size);

                // printf("%d\n\n", randCell);

                if (_grid[randCell]._type == 'M' || randCell == _pos)
                {
                    valid = false;
                }

                else
                {
                    for (int j = 0; j < 8; j++)
                    {
                        // if (randCell == GetNeighbours(_pos)[j]) // TO DO : Fix this (may have to change GetNeighbours() to return an int array of cell positions)
                        // {
                        //     valid = false;
                        // }

                        if (randCell == _pos || randCell == _pos - 1 || randCell == _pos + 1 || randCell == _pos - _size || randCell == _pos - _size - 1 || randCell == _pos - _size + 1 || randCell == _pos + _size || randCell == _pos + _size - 1 || randCell == _pos + _size + 1)
                        {
                            valid = false;
                        }
                    }
                }
            }
            while (!valid);
            // while (_grid[randCell]._type == 'M' || randCell == _pos || randCell == _pos - 1 || randCell == _pos + 1 || randCell == _pos - _size || randCell == _pos - _size - 1 || randCell == _pos - _size + 1 || randCell == _pos + _size || randCell == _pos + _size - 1 || randCell == _pos + _size + 1);

            _grid[randCell]._type = 'M';
        }
    }

    public : void PlaceNumbers()
    {
        for (int i = 0; i < _size * _size; i++)
        {
            for (int j = 0; j < 8; j++) // TO DO : Fix this so GetNeighbours can be used
            {
                if (_grid[GetNeighbours(i)[j]]._type == 'M')
                {
                    _grid[i]._mineCount++;
                }
            }

            // if (_grid[i]._type != 'M')
            // {
            //     if (i % _size != 0) // if not on left column
            //     {
            //         if (i >= _size) // if not on top row
            //         {
            //             if (_grid[i - (_size + 1)]._type == 'M') // top left
            //             {
            //                 _grid[i]._mineCount++;
            //             }
            //         }

            //         if (_grid[i - 1]._type == 'M') // centre left
            //         {
            //             _grid[i]._mineCount++;
            //         }

            //         if (i < (_size * (_size - 1))) // if not on bottom row
            //         {
            //             if (_grid[i + (_size - 1)]._type == 'M') // bottom left
            //             {
            //                 _grid[i]._mineCount++;
            //             }
            //         }
            //     }

            //     if ((i + 1) % _size != 0) // if not on right column
            //     {
            //         if (i >= _size) // if not on top row
            //         {
            //             if (_grid[i - (_size - 1)]._type == 'M') // top right
            //             {
            //                 _grid[i]._mineCount++;
            //             }
            //         }

            //         if (_grid[i + 1]._type == 'M') // centre right
            //         {
            //             _grid[i]._mineCount++;
            //         }

            //         if (i < (_size * (_size - 1))) // if not on bottom row
            //         {
            //             if (_grid[i + (_size + 1)]._type == 'M') //bottom right
            //             {
            //                 _grid[i]._mineCount++;
            //             }
            //         }
            //     }

            //     if (i >= _size) // if not on top row
            //     {
            //         if (_grid[i - _size]._type == 'M') // top centre
            //         {
            //             _grid[i]._mineCount++;
            //         }
            //     }

            //     if (i < (_size * (_size - 1))) // if not on bottom row
            //     {
            //         if (_grid[i + _size]._type == 'M') // bottom centre
            //         {
            //             _grid[i]._mineCount++;
            //         }
            //     }
            // }
        }
    }

    void FloodFill(int start)
    {
        int queue[_size * _size]; // max possible
        int head = 0;
        int tail = 0;

        // add starting cell
        queue[tail++] = start;

        while (head < tail)
        {
            int pos = queue[head++];
            Cell& cell = _grid[pos]; // taking this cell by reference so any changes to cell in the method are applied to the actual cell

            // skip if already revealed or flagged
            if (cell._symbol != 'o' || cell._symbol == 'F' || cell._type == 'M')
            {
                continue;
            }

            cell.Open();

            // only expand if it's a 0
            if (cell._mineCount != 0)
            {
                continue;
            }

            // check neighbours

            for (int i = 0; i < 8; i++)
            {
                pos = GetNeighbours(pos)[i];

                if (_grid[pos]._symbol == 'o' && _grid[pos]._type == 'E')
                {
                    queue[tail++] = pos;
                }
            }

            // for (int dy = -1; dy <= 1; dy++)
            // {
            //     for (int dx = -1; dx <= 1; dx++)
            //     {
            //         if (dx == 0 && dy == 0) continue;

            //         int nx = p.x + dx;
            //         int ny = p.y + dy;

            //         // bounds check
            //         if (nx >= 0 && nx < _size && ny >= 0 && ny < _size)
            //         {
            //             Cell& neighbour = getCell(nx, ny);

            //             if (!neighbour._revealed && !neighbour._flagged)
            //             {
            //                 queue[tail++] = {nx, ny};
            //             }
            //         }
            //     }
            // }
        }
    }

    public : void Update(char input)
    {
        if (_pos >= _size && input == 'U') // if not on top row
        {
            _pos -= _size;
            // Cursor(_size, _grid[_pos + _size]._symbol);
        }

        else if (_pos < _size * (_size - 1) && input == 'D') // if not on bottom row
        {
            _pos += _size;
            // Cursor(-_size, _grid[_pos - _size]._symbol);
        }

        else if (_pos % _size != 0 && input == 'L') // if not on left column
        {
            _pos--;
            // Cursor(1, _grid[_pos + 1]._symbol);
        }

        else if ((_pos + 1) % _size != 0 && input == 'R') // if not on right column
        {
            _pos++;
            // Cursor(-1, _grid[_pos - 1]._symbol);
        }

        else if (input == 'A')
        {
            if (!_gameSet)
            {
                srand(HAL_GetTick()); // seed RNG
                PlaceMines();
                PlaceNumbers();
                _gameSet = true;
            }

            if (_grid[_pos]._symbol == 'o')
            {
                // _grid[_pos].Open();
                FloodFill(_pos);
                // TO DO : Add flood fill
            }
        }

        else if (input == 'B')
        {
            if (_grid[_pos]._symbol == 'o')
            {
                _grid[_pos].Flag();
            }
            
            else if (_grid[_pos]._symbol == 'F')
            {
                _grid[_pos].UnFlag();
            }
        }
    }

    void CheckWin()
    {

    }

    // void Cursor(int move, char pSymbol)
    // {
    //     if (_pos != 0)
    //     {
    //         _grid[_pos + move]._symbol = pSymbol;
    //         _grid[_pos]._symbol = '@';
    //     }
    // }


    public : void DisplayMap()
    {
        for (int i = 0; i < _size * _size; i++)
        {
            if (i == _pos)
            {
                printf("@ ");
            }

            // else if (_grid[i]._type == 'M')
            // {
            //     printf("X ");
            // }

            // else if (_grid[i]._mineCount > 0)
            // {
            //     printf("%d ", _grid[i]._mineCount);
            // }

            else
            {
                printf("%c ", _grid[i]._symbol);
            }
            
            if ((i + 1) % _size == 0 && i > 0)
            {
                printf("\n");
            }
        }   
    }
};

char GetInput()
{
    char input = ' ';

    DigitalIn A (D8, PullDown);
    DigitalIn B (D9, PullDown);
    DigitalIn U (D10, PullDown);
    DigitalIn D (D11, PullDown);
    DigitalIn L (D12, PullDown);
    DigitalIn R (D13, PullDown);

    while (input == ' ')
    {
        if (A)
        {
            input = 'A';
        }

        else if (B)
        {
            input =  'B';
        }

        else if (U)
        {
            input = 'U';
        }

        else if (D)
        {
            input = 'D';
        }

        else if (L)
        {
            input = 'L';
        }

        else if (R)
        {
            input = 'R';
        }
    }

    thread_sleep_for(100);

    return input;
}

// main() runs in its own thread in the OS
int main()
{
    // DigitalIn B(BUTTON1);

    // while (B)
    // {

    // }

    // srand(HAL_GetTick());

    Map m(8, 8, 16);
    
    printf("Welcome To Minesweeper!\n\n");

    while (true)
    {
        m.DisplayMap();
        printf("\n\n");

        char i = GetInput(); 

        m.Update(i);

        // for (int i = 0; i < 8; i++)
        // {
        //     printf("%d ", m.GetNeighbours(m._pos)[i]);
        // }

        // printf("\n\n");
    }

    while (true) 
    {
        sleep();
    }
}