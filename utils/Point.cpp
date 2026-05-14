#include "Point.h"

Point::Point(int xPos, int yPos)
{
    x = xPos;
    y = yPos;
}

Point Point::operator+(Point other) const
{
    return Point(x + other.x, y + other.y);
}

Point Point::operator-(Point other) const
{
    return Point(x - other.x, y - other.y);
}
