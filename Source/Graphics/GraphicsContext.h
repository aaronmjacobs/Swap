#pragma once

#include "Core/Assert.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/TextureInfo.h"

#include <glad/glad.h>

#include <array>

class GraphicsContext
{
public:
   static GraphicsContext& current()
   {
      ASSERT(currentContext);
      return *currentContext;
   }

   ~GraphicsContext();

   void makeCurrent();

   void useProgram(GLuint program);
   void bindVertexArray(GLuint vao);
   void bindFramebuffer(Fb::Target target, GLuint framebuffer);

   void activeTexture(int textureUnit);
   void bindTexture(Tex::Target target, GLuint texture);
   void activateAndBindTexture(int textureUnit, Tex::Target target, GLuint texture);

   void onProgramDestroyed(GLuint program);
   void onVertexArrayDestroyed(GLuint vao);
   void onFramebufferDestroyed(GLuint framebuffer);
   void onTextureDestroyed(Tex::Target target, GLuint texture);

private:
   using TextureBindings = std::array<GLuint, 25>;

   static void setCurrent(GraphicsContext* context);
   static void onDestroy(GraphicsContext* context);

   static GraphicsContext* currentContext;

   GLuint boundProgram = 0;
   GLuint boundVAO = 0;
   GLuint boundReadFramebuffer = 0;
   GLuint boundDrawFramebuffer = 0;
   
   int activeTextureUnit = 0;
   std::array<TextureBindings, 32> textureBindings;
};