#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"
#include "Graphics/Framebuffer.h"
#include "Graphics/RasterizerState.h"
#include "Graphics/TextureInfo.h"
#include "Graphics/UniformBufferObject.h"
#include "Graphics/Viewport.h"

#include <glad/gl.h>

#include <array>
#include <vector>

enum class PrimitiveMode : GLenum
{
   Points = GL_POINTS,
   LineStrip = GL_LINE_STRIP,
   LineLoop = GL_LINE_LOOP,
   Lines = GL_LINES,
   LineStripAdjacency = GL_LINE_STRIP_ADJACENCY,
   LinesAdjacency = GL_LINES_ADJACENCY,
   TriangleStrip = GL_TRIANGLE_STRIP,
   TriangleFan = GL_TRIANGLE_FAN,
   Triangles = GL_TRIANGLES,
   TriangleStripAdjacency = GL_TRIANGLE_STRIP_ADJACENCY,
   TrianglesAdjacency = GL_TRIANGLES_ADJACENCY,
   Patches = GL_PATCHES
};

enum class IndexType : GLenum
{
   UnsignedBye = GL_UNSIGNED_BYTE,
   UnsignedShort = GL_UNSIGNED_SHORT,
   UnsignedInt = GL_UNSIGNED_INT
};

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

   void initialize();

   const Viewport& getDefaultViewport() const
   {
      return defaultViewport;
   }

   const Viewport& getActiveViewport() const
   {
      return activeViewport;
   }

   const SPtr<UniformBufferObject>& getFramebufferUniformBuffer() const
   {
      return framebufferUniformBuffer;
   }

   void setDefaultViewport(const Viewport& viewport);
   void setActiveViewport(const Viewport& viewport);

   void useProgram(GLuint program);
   void bindVertexArray(GLuint vao);
   void bindFramebuffer(Fb::Target target, GLuint framebuffer);
   GLuint getBoundFramebuffer(Fb::Target target) const;

   void activeTexture(int textureUnit);
   void bindTexture(Tex::Target target, GLuint texture);
   void activateAndBindTexture(int textureUnit, Tex::Target target, GLuint texture);

   void drawElements(PrimitiveMode mode, GLsizei count, IndexType type, const GLvoid* indices);

   void pushRasterizerState(const RasterizerState& state);
   void popRasterizerState();

   void onProgramDestroyed(GLuint program);
   void onVertexArrayDestroyed(GLuint vao);
   void onFramebufferDestroyed(GLuint framebuffer);
   void onTextureDestroyed(Tex::Target target, GLuint texture);

private:
   using TextureBindings = std::array<GLuint, 25>;

   static void setCurrent(GraphicsContext* context);
   static void onDestroy(GraphicsContext* context);

   static GraphicsContext* currentContext;

   Viewport defaultViewport;
   Viewport activeViewport;

   GLuint boundProgram = 0;
   GLuint boundVAO = 0;
   GLuint boundReadFramebuffer = 0;
   GLuint boundDrawFramebuffer = 0;

   int activeTextureUnit = 0;
   std::array<TextureBindings, 32> textureBindings;

   SPtr<UniformBufferObject> framebufferUniformBuffer;

   std::vector<RasterizerState> rasterizerStateStack;
   RasterizerState baseRasterizerState;
   RasterizerState currentRasterizerState;
   bool rasterizerStateDirty = true;
};
