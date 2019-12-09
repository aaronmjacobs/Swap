#pragma once

#include <glad/gl.h>

enum class FaceCullMode : GLenum
{
   Front = GL_FRONT,
   Back = GL_BACK,
   FrontAndBack = GL_FRONT_AND_BACK
};

enum class DepthFunc : GLenum
{
   Never = GL_NEVER,
   Always = GL_ALWAYS,
   Equal = GL_EQUAL,
   NotEqual = GL_NOTEQUAL,
   Less = GL_LESS,
   LessEqual = GL_LEQUAL,
   Greater = GL_GREATER,
   GreaterEqual = GL_GEQUAL
};

enum class BlendFactor : GLenum
{
   Zero = GL_ZERO,
   One = GL_ONE,
   SourceColor = GL_SRC_COLOR,
   OneMinusSourceColor = GL_ONE_MINUS_SRC_COLOR,
   DestinationColor = GL_DST_COLOR,
   OneMinusDestinationColor = GL_ONE_MINUS_DST_COLOR,
   SourceAlpha = GL_SRC_ALPHA,
   OneMinusSourceAlpha = GL_ONE_MINUS_SRC_ALPHA,
   DestinationAlpha = GL_DST_ALPHA,
   OneMinusDestinationAlpha = GL_ONE_MINUS_DST_ALPHA,
   ConstantColor = GL_CONSTANT_COLOR,
   OneMinusConstantColor = GL_ONE_MINUS_CONSTANT_COLOR,
   ConstantAlpha = GL_CONSTANT_ALPHA,
   OneMinusConstantAlpha = GL_ONE_MINUS_CONSTANT_ALPHA,
   SourceAlphaSaturate = GL_SRC_ALPHA_SATURATE,
   Source1Color = GL_SRC1_COLOR,
   OneMinusSource1Color = GL_ONE_MINUS_SRC1_COLOR,
   Source1Alpha = GL_SRC1_ALPHA,
   OneMinusSource1Alpha = GL_ONE_MINUS_SRC1_ALPHA,
};

struct RasterizerState
{
   bool enableFaceCulling = true;
   FaceCullMode faceCullMode = FaceCullMode::Back;

   bool enableDepthTest = true;
   bool enableDepthWriting = true;
   DepthFunc depthFunc = DepthFunc::Less;

   bool enableBlending = false;
   BlendFactor sourceBlendFactor = BlendFactor::One;
   BlendFactor destinationBlendFactor = BlendFactor::Zero;
};
