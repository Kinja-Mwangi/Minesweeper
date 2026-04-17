#include "mbed.h"
#include <cstdlib>
#include <ctime>
#include <ctype.h> 
#include "TextLCD.h"

#include "Map.h"

TextLCD lcd(D0, D1, D2, D3, D4, D5, TextLCD::LCD20x4); // Connect these nucleo pins to RS, E, D4, D5, D6 and D7 pins of the LCD

int RNG(int lB, int uB) // lB inclusive, uB exclusive
{
    int r = uB - lB;
    return (rand() % r) + lB; 
}

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