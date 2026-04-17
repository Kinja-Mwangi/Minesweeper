#pragma once

class Cell
{
    public:
    char _type;
    char _symbol;
    int _mineCount = 0;

    Cell()
    {
        _type = 'E';
        _symbol = '+';
    }

    Cell(char type, char symbol, int pos)
    {
        _type = type;
        _symbol = symbol;
    }

    void Open()
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

    void Flag()
    {
        _symbol = 'F';
    }

    void UnFlag()
    {
        _symbol = '+';
    }
};