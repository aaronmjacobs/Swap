#pragma once

class ShaderProgram;

struct DrawingContext
{
   DrawingContext(ShaderProgram* shaderProgram = nullptr)
      : program(shaderProgram)
      , textureUnitCounter(0)
   {
   }

   ShaderProgram* program;
   int textureUnitCounter;
};
