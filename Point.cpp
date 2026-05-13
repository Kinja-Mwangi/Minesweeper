#include "Point.h"

Point::Point(int xPos, int yPos)
{
    x = xPos;
    y = yPos;
}

Point::operator+(Point other) const
{
    return Point(x + other.x, y + other.y);
}

Point::operator-(Point other) const
{
    return Point(x - other.x, y - other.y);
}
