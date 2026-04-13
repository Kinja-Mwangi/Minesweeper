#include "mbed.h"
#include <cstdlib>
#include <time.h>
#include <ctype.h> 
#include "TextLCD.h"

TextLCD lcd(D0, D1, D2, D3, D4, D5, TextLCD::LCD20x4); // Connect these nucleo pins to RS, E, D4, D5, D6 and D7 pins of the LCD

int RNG(int lB, int uB) // lB inclusive, uB exclusive
{
    int r = uB - lB;
    return (rand() % r) + lB; 
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
            _symbol = '+';
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
                _symbol = _mineCount + 48; // convert mine count into a character
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
            _symbol = '+';
        }
    };

    public : int _size;
    int _mineCount;
    Cell* _grid;
    public : int _pos = 0;
    bool _gameInitialized = false;
    public : int _start = 0;
    public : int _end;

    public : Map(int size, int minMines, int maxMines)
    {
        _size = size;
        _mineCount = RNG(minMines, maxMines + 1);
        _grid = new Cell[_size * _size];
        _end = _size * 4;
    }

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
            neighbours[i] = -1; // Dodgey fix - fills the remaining neighbour cells with positions beyond the bounds of the grid which therefore technically don't exist (should work for now)
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


                if (_grid[randCell]._type == 'M' || randCell == _pos)
                {
                    valid = false;
                }

                else
                {
                    // if (randCell == _pos || randCell == _pos - 1 || randCell == _pos + 1 || randCell == _pos - _size || randCell == _pos - _size - 1 || randCell == _pos - _size + 1 || randCell == _pos + _size || randCell == _pos + _size - 1 || randCell == _pos + _size + 1)
                    // {
                    //     valid = false;
                    // }
                
                    for (int j = 0; j < 8; j++)
                    {
                        if (randCell == GetNeighbours(_pos)[j])
                        {
                            valid = false;
                        }
                    }
                }
            }
            while (!valid);

            _grid[randCell]._type = 'M';
        }
    }

    public : void PlaceNumbers()
    {
        for (int i = 0; i < _size * _size; i++)
        {
            // Place numbers using GetNeighbours
            for (int j = 0; j < 8; j++)
            {
                if (_grid[GetNeighbours(i)[j]]._type == 'M')
                {
                    _grid[i]._mineCount++;
                }
            }
        }
    }

    void FloodFill(int start)
    {
        int queue[_size * _size];
        int head = 0;
        int tail = 0;

        // Add starting cell
        queue[tail++] = start;

        while (head < tail)
        {
            int pos = queue[head++];
            Cell& cell = _grid[pos]; // This cell is addressed by reference so any changes to cell in the method are applied to the actual cell

            // Skip if already revealed or flagged
            if (cell._symbol != '+' || cell._symbol == 'F' || cell._type == 'M')
            {
                continue;
            }

            cell.Open();

            // Only expand if minecount = 0
            if (cell._mineCount != 0)
            {
                continue;
            }

            int* neighbours = GetNeighbours(pos);

            // Check neighbours
            for (int i = 0; i < 8; i++)
            {
                int neighbour = neighbours[i];

                if (_grid[neighbour]._symbol == '+' && _grid[neighbour]._type == 'E' && neighbour >= 0 /*dodgey workaround*/)
                {
                    queue[tail++] = neighbour;
                }
            }
        }
    }

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

        // thread_sleep_for(100);

        return input;
    }

    public : void Update()
    {
        char input = GetInput();
        
        if (_pos >= _size && input == 'U') // if not on top row
        {
            _pos -= _size;
        }

        else if (_pos < _size * (_size - 1) && input == 'D') // if not on bottom row
        {
            _pos += _size;
        }

        else if (_pos % _size != 0 && input == 'L') // if not on left column
        {
            _pos--;
        }

        else if ((_pos + 1) % _size != 0 && input == 'R') // if not on right column
        {
            _pos++;
        }

        else if (input == 'A')
        {
            if (!_gameInitialized)
            {
                srand(HAL_GetTick()); // seed RNG
                PlaceMines();
                PlaceNumbers();
                _gameInitialized = true;
            }

            if (_grid[_pos]._symbol == '+')
            {   
                if (_grid[_pos]._mineCount == 0)
                {
                    FloodFill(_pos);
                }

                else
                {
                    _grid[_pos].Open();
                }
            }
        }

        else if (input == 'B')
        {
            if (_grid[_pos]._symbol == '+')
            {
                _grid[_pos].Flag();
            }
            
            else if (_grid[_pos]._symbol == 'F')
            {
                _grid[_pos].UnFlag();
            }
        }
    }

    // Display on LCD Display
    public : void Display()
    {
        char cursor = '#';
        lcd.locate(0,0);

        if (_pos < _start)
        {
            if (_start >= 0)
            {
                _start -= _size;
            }
            
            _end -= _size;
        }

        else if (_pos > _end)
        {
            _start += _size;

            if (_end <= _size * _size)
            {
                _end += _size;
            }
        }

        for (int i = _start; i < _end; i++)
        {
            if (i == _pos)
            {
                lcd.printf("%c ", cursor);
            }

            else if (_grid[i]._symbol == '+')
            {
                lcd.printf("%c ", 219);
            }

            else
            {
                lcd.printf("%c ", _grid[i]._symbol);
            }

            if ((i + 1) % _size == 0 && i > 0)
            {
                lcd.locate(0, (i + 1) / _size);
            }
        }
    }

    // Display on serial port terminal (Coolterm)
    public : void Print()
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

        printf("\n\n");
    }
};

int main()
{
    Map m(10, 10, 18);
    // Display d(20, 4);
    
    printf("Welcome To Minesweeper!\n\n");

    while (true)
    {
        // d.Update();
        m.Print();
        m.Update();
    }

    // while (true) 
    // {
    //     sleep();
    // }
}