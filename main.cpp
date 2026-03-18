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

        public : Cell(char type, char symbol)
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

    public : void Reset()
    {
        for (int i = 0; i < _size * _size; i++)
        {
            _grid[i] = Cell('E', 'o');
        }
    }

    public : void PlaceMines()
    {
        for (int i = 0; i < _mineCount; i++)
        {
            int randCell;

            do
            {
                randCell = RNG(0, _size * _size);
            }
            while (_grid[randCell]._type != 'M' && randCell != _pos && randCell != _pos - 1 && randCell != _pos + 1 && randCell != _pos - _size && randCell != _pos - _size - 1 && randCell != _pos - _size + 1 && randCell != _pos + _size && randCell != _pos + _size - 1 && randCell != _pos + _size + 1);

            _grid[randCell]._type = 'M';
        }
    }

    public : void PlaceNumbers()
    {
        for (int i = 0; i < _size * _size; i++)
        {
            // int count = 0;

            if (_grid[i]._type != 'M')
            {
                if (i % _size != 0) // if not on left column
                {
                    if (i >= _size) // if not on top row
                    {
                        if (_grid[i - (_size + 1)]._type == 'M') // top left
                        {
                            _grid[i]._mineCount++;
                        }
                    }

                    if (_grid[i - 1]._type == 'M') // centre left
                    {
                        _grid[i]._mineCount++;
                    }

                    if (i < (_size * (_size - 1))) // if not on bottom row
                    {
                        if (_grid[i + (_size - 1)]._type == 'M') // bottom left
                        {
                            _grid[i]._mineCount++;
                        }
                    }
                }

                if ((i + 1) % _size != 0) // if not on right column
                {
                    if (i >= _size) // if not on top row
                    {
                        if (_grid[i - (_size - 1)]._type == 'M') // top right
                        {
                            _grid[i]._mineCount++;
                        }
                    }

                    if (_grid[i + 1]._type == 'M') // centre right
                    {
                        _grid[i]._mineCount++;
                    }

                    if (i < (_size * (_size - 1))) // if not on bottom row
                    {
                        if (_grid[i + (_size + 1)]._type == 'M') //bottom right
                        {
                            _grid[i]._mineCount++;
                        }
                    }
                }

                if (i >= _size) // if not on top row
                {
                    if (_grid[i - _size]._type == 'M') // top centre
                    {
                        _grid[i]._mineCount++;
                    }
                }

                if (i < (_size * (_size - 1))) // if not on bottom row
                {
                    if (_grid[i + _size]._type == 'M') // bottom centre
                    {
                        _grid[i]._mineCount++;
                    }
                }
            }

            // if (count > 0)
            // {
            //     _grid[i]._type = (char)(count + 48);
            //     // _grid[i]._type = 'N';
            // }
        }
    }

    void Update(char input)
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
                PlaceMines();
                PlaceNumbers();
                _gameSet = true;
            }

            if (_grid[_pos]._symbol != 'o')
            {
                _grid[_pos].Open();
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
            printf("%c ", _grid[i]._symbol);

            if (i == _pos)
            {
                printf("@");
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
    DigitalIn Start(BUTTON1);

    Map m(8, 4, 16);
    
    printf("Welcome To Minesweeper!\n\n");

    while (true)
    {
        m.DisplayMap();
        printf("\n\n");

        char i = GetInput(); 

        m.Update(i);
    }

    while (true) 
    {
        sleep();
    }
}