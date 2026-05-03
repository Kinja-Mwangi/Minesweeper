#include "mbed.h"
#include "TextLCD.h"
#include <ctype.h>
#include <time.h>

TextLCD lcd(D0, D1, D2, D3, D4, D5, TextLCD::LCD20x4); // Connect these nucleo pins to RS, E, D4, D5, D6 and D7 pins of the LCD

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
        public : bool _opened;
        public : bool _flagged;
        public : bool _mine;
        public : int _mineCount;

        public : Cell() // Default constructor
        {
            _opened = false;
            _flagged = false;
            _mine = false;
            _mineCount = 0;
        }

        public : void Open()
        {
            _opened = true;
        }

        public : void Flag()
        {
            _flagged = true;
        }

        public : void UnFlag()
        {
            _flagged = false;
        }
    };

    int _size;
    public : int _totalMines;
    Cell* _grid;
    int _pos = 0;
    bool _gameInitialized = false;
    
    // 2D Camera Viewport Variables
    int _displayStartX = 0;
    int _displayStartY = 0;
    int _displayWidth;
    int _displayHeight;

    // Custom characters
    char unopenedCell[8] = {0x0D, 0x16, 0x1B, 0x0D, 0x16, 0x1B, 0x0D, 0x16};
    char emptyCell[8] = {0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E};
    char flag[8] = {0x08, 0x0C, 0x0E, 0x0F, 0x08, 0x08, 0x1E, 0x00};
    char mine[8] = {0x00, 0x15, 0x0E, 0x1F, 0x0E, 0x15, 0x00, 0x00};
    char flower[8] = {0x0E, 0x11, 0x15, 0x11, 0x0E, 0x06, 0x04, 0x0E};

    // Inverted versions of the custom characters
    char unopenedCellInverted[8] = {0x12, 0x09, 0x04, 0x12, 0x09, 0x04, 0x12, 0x09};
    char emptyCellInverted[8] = {0x11, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x11};
    char flagInverted[8] = {0x17, 0x13, 0x11, 0x10, 0x17, 0x17, 0x01, 0x1F};
    char mineCountsInverted[8][8] = 
        {{0x1B, 0x13, 0x1B, 0x1B, 0x1B, 0x1B, 0x11, 0x1F}, // 1
        {0x11, 0x0E, 0x1E, 0x1D, 0x1B, 0x17, 0x00, 0x1F}, // 2
        {0x00, 0x1D, 0x1B, 0x1D, 0x1E, 0x0E, 0x11, 0x1F}, // 3
        {0x1D, 0x19, 0x15, 0x0D, 0x00, 0x1D, 0x1D, 0x1F}, // 4
        {0x00, 0x0F, 0x01, 0x1E, 0x1E, 0x0E, 0x11, 0x1F}, // 5
        {0x19, 0x17, 0x0F, 0x01, 0x0E, 0x0E, 0x11, 0x1F}, // 6
        {0x00, 0x1E, 0x1D, 0x1B, 0x17, 0x17, 0x17, 0x1F}, // 7
        {0x11, 0x0E, 0x0E, 0x11, 0x0E, 0x0E, 0x11, 0x1F}}; // 8

    public : Game(int size, int minMines, int maxMines)
    {
        _size = size;
        _totalMines = RNG(minMines, maxMines + 1);
        _grid = new Cell[_size * _size];
        
        // LCD is 20x4, meaning max 10 cells wide (2 chars per cell) and 4 cells high
        _displayWidth = _size > 10 ? 10 : _size;
        _displayHeight = _size > 4 ? 4 : _size;

        lcd.writeCustomCharacter(unopenedCell, 1);
        lcd.writeCustomCharacter(emptyCell, 2);
        lcd.writeCustomCharacter(flag, 3);
        lcd.writeCustomCharacter(mine, 4);
        lcd.writeCustomCharacter(unopenedCellInverted, 5);
        lcd.writeCustomCharacter(emptyCellInverted, 6);
        lcd.writeCustomCharacter(flagInverted, 7);
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

                if (_grid[randCell]._mine || randCell == pos)
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

            _grid[randCell]._mine = true;
        }
    }

    void PlaceNumbers()
    {
        for (int i = 0; i < _size * _size; i++)
        {
            // Place numbers using GetNeighbours
            for (int j = 0; j < 8; j++)
            {
                if (_grid[GetNeighbours(i)[j]]._mine)
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
                if (!_grid[neighbours[i]]._opened && !_grid[neighbours[i]]._flagged && !_grid[neighbours[i]]._mine && !Contains(queue, tail, neighbours[i]) && neighbours[i] >= 0)
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
            if (_grid[neighbours[i]]._flagged)
            {
                flagCount++;
            }
        }

        if (flagCount == _grid[pos]._mineCount)
        {
            for (int i = 0; i < 8; i++)
            {
                if (!_grid[neighbours[i]]._opened && !_grid[neighbours[i]]._flagged)
                {
                    FloodFill(neighbours[i]);
                }
            }
        }
    }

    char GetInput()
    {
        DigitalIn A(D8, PullDown);
        DigitalIn B(D9, PullDown);
        DigitalIn U(D10, PullDown);
        DigitalIn D(D11, PullDown);
        DigitalIn L(D12, PullDown);
        DigitalIn R(D13, PullDown);

        while (true)
        {
            if (A)
            {
                return 'A';
            }

            else if (B)
            {
                return  'B';
            }

            else if (U)
            {
                return 'U';
            }

            else if (D)
            {
                return 'D';
            }

            else if (L)
            {
                return 'L';
            }

            else if (R)
            {
                return 'R';
            }
        }
    }

    void GameOver()
    {
        bool win = true;
        bool lose = false;

        for (int i = 0; i < _size * _size; i++)
        {
            if (!_grid[i]._mine && !_grid[i]._opened)
            {
                win = false;
            }

            else if (_grid[i]._mine && _grid[i]._opened)
            {
                win = false;
                lose = true;
                break;
            }
        }

        if (win)
        {
            lcd.writeCustomCharacter(flower, 4);

            for (int i = 0; i < _size * _size; i++) // Reveal all mines
            {
                if (_grid[i]._mine)
                {
                    _grid[i].Open();
                    Display();
                }
            }

            thread_sleep_for(3000);
            lcd.cls();
            lcd.printf("Congratulations!\nYou Win!");

            while (true)
            {
                sleep();
            }
        }

        else if (lose)
        {
            for (int i = 0; i < _size * _size; i++) // Reveal all mines
            {
                if (_grid[i]._mine)
                {
                    _grid[i].Open();
                    Display();
                }
            }

            thread_sleep_for(2000);
            lcd.cls();
            lcd.printf("Game Over!\nYou Lose...");

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

                if (!_grid[_pos]._flagged)
                {
                    if (!_grid[_pos]._opened)
                    {
                        FloodFill(_pos);
                    }
                    
                    else
                    {
                        Chord(_pos);
                    }
                }

                GameOver();
                break;

            case 'B':
                if (!_grid[_pos]._opened && !_grid[_pos]._flagged && _gameInitialized)
                {
                    _grid[_pos].Flag();
                }

                else if (_grid[_pos]._flagged)
                {
                    _grid[_pos].UnFlag();
                }
                break;

            default:
                break;
        }

        // Camera Handling for both X and Y
        int posX = _pos % _size;
        int posY = _pos / _size;

        // Horizontal Scrolling Checks
        if (posX < _displayStartX)
        {
            _displayStartX = posX;
        }

        else if (posX - _displayStartX >= _displayWidth)
        {
            _displayStartX = posX - _displayWidth + 1;
        }

        // Vertical Scrolling Checks
        if (posY < _displayStartY)
        {
            _displayStartY = posY;
        }
        
        else if (posY - _displayStartY >= _displayHeight)
        {
            _displayStartY = posY - _displayHeight + 1;
        }
    }

    // Display on LCD
    public : void Display()
    {
        for (int y = 0; y < _displayHeight; y++)
        {
            // Center the display logically based on visible width 
            lcd.locate(10 - _displayWidth, y); 
            
            for (int x = 0; x < _displayWidth; x++)
            {
                // Translate camera coordinates to absolute grid index
                int i = ((y + _displayStartY) * _size) + x + _displayStartX;

                if (i == _pos)
                {
                    if (_grid[i]._opened)
                    {
                        if (_grid[i]._mine)
                        {
                            lcd.printf("%c ", 4);
                        }
                        
                        else if (_grid[i]._mineCount == 0)
                        {
                            lcd.printf("%c ", 6);
                        }

                        else if (_grid[i]._mineCount > 0 && _grid[i]._mineCount <= 8)
                        {
                            lcd.writeCustomCharacter(mineCountsInverted[_grid[i]._mineCount - 1], 8);
                            lcd.printf("%c ", 8);
                        }
                    }

                    else
                    {
                        if (_grid[i]._flagged)
                        {
                            lcd.printf("%c ", 7);
                        }

                        else
                        {
                            lcd.printf("%c ", 5);
                        }
                    }
                }

                else
                {
                    if (_grid[i]._opened)
                    {
                        if (_grid[i]._mine)
                        {
                            lcd.printf("%c ", 4);
                        }

                        else if (_grid[i]._mineCount == 0)
                        {
                            lcd.printf("%c ", 2);
                        }

                        else if (_grid[i]._mineCount > 0 && _grid[i]._mineCount <= 8)
                        {
                            lcd.printf("%d ", _grid[i]._mineCount);
                        }
                    }

                    else
                    {
                        if (_grid[i]._flagged)
                        {
                            lcd.printf("%c ", 3);
                        }

                        else
                        {
                            lcd.printf("%c ", 1);
                        }
                    }
                }
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

            else if (_grid[i]._opened)
            {
                if (_grid[i]._mine)
                {
                    printf("X ");
                }

                else if (_grid[i]._mineCount == 0)
                {
                    printf("0 ");
                }

                else if (_grid[i]._mineCount > 0 && _grid[i]._mineCount <= 8)
                {
                    printf("%d ", _grid[i]._mineCount);
                }
            }

            else
            {
                if (_grid[i]._flagged)
                {
                    printf("F ");
                }

                else
                {
                    printf("+ ");
                }
            }

            if ((i + 1) % _size == 0)
            {
                printf("\n");
            }
        }

        printf("\n\n");
    }
};

int main()
{
    printf("Welcome To Minesweeper!\n\n");
    Game g(14, 10, 18); // Size > 10 horizontally or > 4 vertically will trigger scrolling

    while (true)
    {
        g.Display();
        g.Print();
        g.Update();
    }
}