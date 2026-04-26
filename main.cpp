#include "mbed.h"
#include "TextLCD.h"
#include <ctype.h>
#include <time.h>

TextLCD lcd(D0, D1, D2, D3, D4, D5, TextLCD::LCD20x4); // Connect these nucleo pins to RS, E, D4, D5, D6 and D7 pins of the LCD

// Custom characters
char unopenedCell[8] = {0x0D, 0x16, 0x1B, 0x0D, 0x16, 0x1B, 0x0D, 0x16};
char emptyCell[8] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
char flag[8] = {0x08, 0x0C, 0x0E, 0x0F, 0x08, 0x08, 0x1E, 0x00};
char mine[8] = {0x00, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x00, 0x00};

// Inverted versions of the custom characters
char unopenedCellInverted[8] = {0x12, 0x09, 0x04, 0x12, 0x09, 0x04, 0x12, 0x09};
char emptyCellInverted[8] = {0x11, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x11};
char flagInverted[8] = {0x17, 0x13, 0x11, 0x10, 0x17, 0x17, 0x01, 0x1F};
// char mineInverted[8] = {0x1F, 0x0A, 0x11, 0x00, 0x11, 0x0A, 0x1F, 0x1F};
char mineCountsInverted[8][8] = {
    {0x1B, 0x13, 0x1B, 0x1B, 0x1B, 0x1B, 0x11, 0x1F}, // 1
    {0x11, 0x0E, 0x1E, 0x1D, 0x1B, 0x17, 0x00, 0x1F}, // 2
    {0x00, 0x1D, 0x1B, 0x1D, 0x1E, 0x0E, 0x11, 0x1F}, // 3
    {0x1D, 0x19, 0x15, 0x0D, 0x00, 0x1D, 0x1D, 0x1F}, // 4
    {0x00, 0x0F, 0x01, 0x1E, 0x1E, 0x0E, 0x11, 0x1F}, // 5
    {0x19, 0x17, 0x0F, 0x01, 0x0E, 0x0E, 0x11, 0x1F}, // 6
    {0x00, 0x1E, 0x1D, 0x1B, 0x17, 0x17, 0x17, 0x1F}, // 7
    {0x11, 0x0E, 0x0E, 0x11, 0x0E, 0x0E, 0x11, 0x1F} // 8
};

int RNG(int lB, int uB) // lB inclusive, uB exclusive
{
    int r = uB - lB;
    return (rand() % r) + lB;
}

bool Contains(int* array, int size, int item)
{
    for (int i = 0; i < size; i++)
    {
        if (array[i] == item)
        {
            return true;
        }
    }

    return false;
}

bool Contains(char* array, int size, char item)
{
    for (int i = 0; i < size; i++)
    {
        if (array[i] == item)
        {
            return true;
        }
    }

    return false;
}

class Game
{
    private : class Cell
    {
        public : char _type;
        public : char _symbol;
        public : int _mineCount = 0;

        public : Cell() // Default constructor
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

    int _size;
    int _totalMines;
    Cell* _grid;
    int _pos = 0;
    bool _gameInitialized = false;
    int _start = 0;
    int _end;

    public : Game(int size, int minMines, int maxMines)
    {
        _size = size;
        // scanf("%d", &_totalMines);
        // _totalMines = _totalMines % (_size * _size);
        // printf("\nTotal Mines: %d\n", _totalMines);
        _totalMines = RNG(minMines, maxMines + 1);
        _grid = new Cell[_size * _size];
        _end = (_size * 4) - 1;
    }

    int* GetNeighbours(int pos)
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
            neighbours[i] = -1; // Dodgey fix - fills the remaining neighbour cells with positions beyond the bounds of the grid which therefore technically don't exist (should work for now)
        }

        return neighbours;
    }

    void PlaceMines(int pos)
    {
        for (int i = 0; i < _totalMines; i++)
        {
            int randCell;
            bool valid;

            do
            {
                valid = true;
                randCell = RNG(0, _size * _size);


                if (_grid[randCell]._type == 'M' || randCell == pos)
                {
                    valid = false;
                }

                else
                {
                    for (int j = 0; j < 8; j++)
                    {
                        if (randCell == GetNeighbours(pos)[j])
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

    void PlaceNumbers()
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

            _grid[pos].Open();

            // Only expand if minecount = 0
            if (_grid[pos]._mineCount != 0)
            {
                continue;
            }

            int* neighbours = GetNeighbours(pos);

            // Check neighbours
            for (int i = 0; i < 8; i++)
            {
                if (_grid[neighbours[i]]._symbol == '+' && _grid[neighbours[i]]._type != 'M' && !Contains(queue, tail, neighbours[i]) && neighbours[i] >= 0 /*dodgey workaround*/)
                {
                    queue[tail++] = neighbours[i];
                }
            }
        }
    }
    
    void Chord(int pos)
    {
        int* neighbours = GetNeighbours(pos);
        int flagCount = 0;

        for (int i = 0; i < 8; i++)
        {
            if (_grid[neighbours[i]]._symbol == 'F')
            {
                flagCount++;
            }
        }

        if (flagCount == _grid[pos]._mineCount)
        {
            for (int i = 0; i < 8; i++)
            {
                _grid[neighbours[i]].Open();
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

        return input;
    }

    void GameOver()
    {
        char gridString[_size * _size];

        for (int i = 0; i < _size * _size; i++)
        {
            gridString[i] = _grid[i]._symbol;
        }

        if (Contains(gridString, _size * _size, 'X'))
        {
            for (int j = 0; j < _size * _size; j++)
            {
                if (_grid[j]._type == 'M')
                {
                    _grid[j]._symbol = 'X';
                }
            }

            Display();
            thread_sleep_for(3000);
            lcd.cls();
            lcd.printf("Game Over!\nYou Lose...");

            while (true)
            {
                sleep();
            }
        }

        else if (!Contains(gridString, _size * _size, '+'))
        {
            Display();
            thread_sleep_for(3000);
            lcd.cls();
            lcd.printf("Congratulations!\nYou Win!");

            while (true)
            {
                sleep();
            }
        }
    }

    public : void Update()
    {
        char input = GetInput();

        switch (input)
        {
            case 'U':
                if (_pos >= _size) // if not on top row
                {
                    _pos -= _size;
                }
                break;

            case 'D':
                if (_pos < _size * (_size - 1)) // if not on bottom row
                {
                    _pos += _size;
                }
                break;

            case 'L':
                if (_pos % _size != 0) // if not on left column
                {
                    _pos--;
                }
                break;

            case 'R':
                if ((_pos + 1) % _size != 0) // if not on right column
                {
                    _pos++;
                }
                break;

            case 'A':
                if (!_gameInitialized)
                {
                    srand(HAL_GetTick()); // seed RNG
                    PlaceMines(_pos);
                    PlaceNumbers();
                    _gameInitialized = true;
                }

                if (_grid[_pos]._symbol == '+')
                {
                    FloodFill(_pos);
                }

                // else if (_grid[_pos]._mineCount > 0)
                // {
                //     Chord(_pos);
                // }
                break;

            case 'B':
                if (_grid[_pos]._symbol == '+')
                {
                    _grid[_pos].Flag();
                }

                else if (_grid[_pos]._symbol == 'F')
                {
                    _grid[_pos].UnFlag();
                }
                break;

            default:
                break;
        }

        if (_pos < _start)
        {
            _start -= _size;
            _end -= _size;
        }

        else if (_pos > _end)
        {
            _start += _size;
            _end += _size;
        }

        GameOver();
    }

    // Display on LCD
    public : void Display()
    {
        lcd.locate(10 - _size, 0);

        // char cursor = '#';
        lcd.writeCustomCharacter(unopenedCell, 1);
        lcd.writeCustomCharacter(emptyCell, 2);
        lcd.writeCustomCharacter(flag, 3);
        lcd.writeCustomCharacter(mine, 4);
        lcd.writeCustomCharacter(unopenedCellInverted, 5);
        lcd.writeCustomCharacter(emptyCellInverted, 6);
        lcd.writeCustomCharacter(flagInverted, 7);

        for (int i = _start; i <= _end; i++)
        {
            if (i == _pos)
            {
                // lcd.printf("%c ", cursor);

                if (_grid[i]._symbol == '+')
                {
                    lcd.printf("%c ", 5);
                }

                else if (_grid[i]._symbol == '0')
                {
                    lcd.printf("%c ", 6);
                }

                else if (_grid[i]._symbol == 'F')
                {
                    lcd.printf("%c ", 7);
                }

                else if (_grid[i]._symbol == 'X')
                {
                    lcd.printf("%c ", 4);
                }

                else if (_grid[i]._symbol >= 48 && _grid[i]._symbol <= 56)
                {
                    lcd.writeCustomCharacter(mineCountsInverted[_grid[i]._mineCount - 1], 8);
                    lcd.printf("%c ", 8);
                }
            }

            else
            {
                if (_grid[i]._symbol == '+')
                {
                    lcd.printf("%c ", 1);
                }

                else if (_grid[i]._symbol == '0')
                {
                    lcd.printf("%c ", 2);
                }

                else if (_grid[i]._symbol == 'F')
                {
                    lcd.printf("%c ", 3);
                }

                else if (_grid[i]._symbol == 'X')
                {
                    lcd.printf("%c ", 4);
                }

                else
                {
                    lcd.printf("%c ", _grid[i]._symbol);
                }
            }

            if ((i + 1) % _size == 0)
            {
                lcd.locate(10 - _size, (i + 1 - _start) / _size); // locates the next line (from 1 to 3) on the LCD display when a full row of the grid has been printed
            }
        }
    }

    // Display on serial port terminal (Coolterm)
    public : void Print()
    {
        // for (int i = _start; i <= _end; i++)
        for (int i = 0; i < _size * _size; i++)
        {
            if (i % _size == 0)
            {
                if (i > 0)
                {
                    printf("\n");
                }

                printf("%d. ", i / _size);
            }

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
        }

        // printf("\n\nStart: %d\nEnd: %d\nPos: %d\n", _start, _end, _pos);
        printf("\n\n");
    }
};

int main()
{
    printf("Welcome To Minesweeper!\n\n");
    Game g(7, 5, 10);

    while (true)
    {
        g.Display();
        g.Print();
        g.Update();
    }
}