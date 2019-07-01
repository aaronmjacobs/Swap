#pragma once

struct Viewport
{
   Viewport(int xValue, int yValue, int widthValue, int heightValue)
      : x(xValue)
      , y(yValue)
      , width(widthValue)
      , height(heightValue)
   {
   }

   Viewport(int widthValue, int heightValue)
      : Viewport(0, 0, widthValue, heightValue)
   {
   }

   Viewport()
      : Viewport(0, 0, 0, 0)
   {
   }

   bool operator==(const Viewport& other) const
   {
      return x == other.x && y == other.y && width == other.width && height == other.height;
   }

   bool operator!=(const Viewport& other) const
   {
      return !(*this == other);
   }

   int x;
   int y;
   int width;
   int height;
};
