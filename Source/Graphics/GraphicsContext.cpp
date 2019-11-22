#include "GraphicsContext.h"

#include <glm/glm.hpp>

#include <tuple>

namespace
{
   using FramebufferUniforms = std::tuple<
      glm::vec4 // uFramebufferSize
   >;

   FramebufferUniforms calcFramebufferUniforms(const Viewport& viewport)
   {
      FramebufferUniforms framebufferUniforms;

      std::get<0>(framebufferUniforms) = glm::vec4(viewport.width, viewport.height, 1.0f / viewport.width, 1.0f / viewport.height);

      return framebufferUniforms;
   }

   std::size_t textureTargetIndex(Tex::Target target)
   {
      switch (target)
      {
      case Tex::Target::Texture1D:
         return 0;
      case Tex::Target::ProxyTexture1D:
         return 1;

      case Tex::Target::TextureBuffer:
         return 2;

      case Tex::Target::Texture2D:
         return 3;
      case Tex::Target::ProxyTexture2D:
         return 4;
      case Tex::Target::Texture1D_Array:
         return 5;
      case Tex::Target::ProxyTexture1D_Array:
         return 6;
      case Tex::Target::TextureRectangle:
         return 7;
      case Tex::Target::ProxyTextureRectangle:
         return 8;

      case Tex::Target::Texture2D_Multisample:
         return 9;
      case Tex::Target::ProxyTexture2D_Multisample:
         return 10;

      case Tex::Target::TextureCubeMap:
         return 11;
      case Tex::Target::ProxyTextureCubeMap:
         return 12;
      case Tex::Target::TextureCubeMapPositiveX:
         return 13;
      case Tex::Target::TextureCubeMapNegativeX:
         return 14;
      case Tex::Target::TextureCubeMapPositiveY:
         return 15;
      case Tex::Target::TextureCubeMapNegativeY:
         return 16;
      case Tex::Target::TextureCubeMapPositiveZ:
         return 17;
      case Tex::Target::TextureCubeMapNegativeZ:
         return 18;

      case Tex::Target::Texture3D:
         return 19;
      case Tex::Target::ProxyTexture3D:
         return 20;
      case Tex::Target::Texture2D_Array:
         return 21;
      case Tex::Target::ProxyTexture2D_Array:
         return 22;

      case Tex::Target::Texture2D_MultisampleArray:
         return 23;
      case Tex::Target::ProxyTexture2D_MultisampleArray:
         return 24;

      default:
         ASSERT(false);
         return 0;
      }
   }
}

GraphicsContext::~GraphicsContext()
{
   framebufferUniformBuffer = nullptr;

   onDestroy(this);
}

void GraphicsContext::makeCurrent()
{
   setCurrent(this);
}

void GraphicsContext::initialize()
{
   Viewport viewport;
   glGetIntegerv(GL_VIEWPORT, &viewport.x);
   framebufferUniformBuffer = std::make_shared<UniformBufferObject>("Framebuffer");
   framebufferUniformBuffer->setData(calcFramebufferUniforms(viewport));
   framebufferUniformBuffer->bindTo(UniformBufferObjectIndex::Framebuffer);
   framebufferUniformBuffer->setLabel("Framebuffer Uniform Buffer");
   setDefaultViewport(viewport);

   GLint currentProgram = 0;
   glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
   ASSERT(currentProgram >= 0);
   boundProgram = currentProgram;

   GLint vertexArrayBinding = 0;
   glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArrayBinding);
   ASSERT(vertexArrayBinding >= 0);
   boundVAO = vertexArrayBinding;

   GLint readFramebufferBinding = 0;
   glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebufferBinding);
   ASSERT(readFramebufferBinding >= 0);
   boundReadFramebuffer = readFramebufferBinding;

   GLint drawFramebufferBinding = 0;
   glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebufferBinding);
   ASSERT(drawFramebufferBinding >= 0);
   boundDrawFramebuffer = drawFramebufferBinding;

   glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTextureUnit);
   activeTextureUnit -= GL_TEXTURE0;
   ASSERT(activeTextureUnit >= 0);
}

void GraphicsContext::setDefaultViewport(const Viewport& viewport)
{
   ASSERT(viewport.width > 0 && viewport.height > 0);

   if (defaultViewport != viewport)
   {
      defaultViewport = viewport;

      if (boundDrawFramebuffer == 0)
      {
         setActiveViewport(viewport);
      }
   }
}

void GraphicsContext::setActiveViewport(const Viewport& viewport)
{
   ASSERT(viewport.width > 0 && viewport.height > 0);

   if (activeViewport != viewport)
   {
      glViewport(viewport.x, viewport.y, viewport.width, viewport.height);
      activeViewport = viewport;

      framebufferUniformBuffer->updateData(calcFramebufferUniforms(viewport));
   }
}

void GraphicsContext::useProgram(GLuint program)
{
   if (boundProgram != program)
   {
      glUseProgram(program);
      boundProgram = program;
   }
}

void GraphicsContext::bindVertexArray(GLuint vao)
{
   if (boundVAO != vao)
   {
      glBindVertexArray(vao);
      boundVAO = vao;
   }
}

void GraphicsContext::bindFramebuffer(Fb::Target target, GLuint framebuffer)
{
   bool set = false;

   switch (target)
   {
   case Fb::Target::Framebuffer:
      if (boundReadFramebuffer != framebuffer || boundDrawFramebuffer != framebuffer)
      {
         set = true;
         boundReadFramebuffer = framebuffer;
         boundDrawFramebuffer = framebuffer;
      }
      break;
   case Fb::Target::ReadFramebuffer:
      if (boundReadFramebuffer != framebuffer)
      {
         set = true;
         boundReadFramebuffer = framebuffer;
      }
      break;
   case Fb::Target::DrawFramebuffer:
      if (boundDrawFramebuffer != framebuffer)
      {
         set = true;
         boundDrawFramebuffer = framebuffer;
      }
      break;
   default:
      ASSERT(false);
      break;
   }

   if (set)
   {
      glBindFramebuffer(static_cast<GLenum>(target), framebuffer);
   }
}

void GraphicsContext::activeTexture(int textureUnit)
{
   ASSERT(textureUnit < 32);

   if (activeTextureUnit != textureUnit)
   {
      glActiveTexture(GL_TEXTURE0 + textureUnit);
      activeTextureUnit = textureUnit;
   }
}

void GraphicsContext::bindTexture(Tex::Target target, GLuint texture)
{
   TextureBindings& bindings = textureBindings[activeTextureUnit];
   std::size_t index = textureTargetIndex(target);

   if (bindings[index] != texture)
   {
      glBindTexture(static_cast<GLenum>(target), texture);
      bindings[index] = texture;
   }
}

void GraphicsContext::activateAndBindTexture(int textureUnit, Tex::Target target, GLuint texture)
{
   TextureBindings& bindings = textureBindings[textureUnit];
   std::size_t index = textureTargetIndex(target);

   if (bindings[index] != texture)
   {
      activeTexture(textureUnit);
      bindTexture(target, texture);
   }
}

void GraphicsContext::onProgramDestroyed(GLuint program)
{
   if (boundProgram == program)
   {
      useProgram(0);
   }
}

void GraphicsContext::onVertexArrayDestroyed(GLuint vao)
{
   if (boundVAO == vao)
   {
      bindVertexArray(0);
   }
}

void GraphicsContext::onFramebufferDestroyed(GLuint framebuffer)
{
   if (boundReadFramebuffer == framebuffer && boundDrawFramebuffer == framebuffer)
   {
      bindFramebuffer(Fb::Target::Framebuffer, 0);
   }
   else if (boundReadFramebuffer == framebuffer)
   {
      bindFramebuffer(Fb::Target::ReadFramebuffer, 0);
   }
   else if (boundDrawFramebuffer == framebuffer)
   {
      bindFramebuffer(Fb::Target::DrawFramebuffer, 0);
   }
}

void GraphicsContext::onTextureDestroyed(Tex::Target target, GLuint texture)
{
   std::size_t index = textureTargetIndex(target);
   GLuint cachedActiveTextureUnit = activeTextureUnit;

   for (int i = 0; i < textureBindings.size(); ++i)
   {
      if (textureBindings[i][index] == texture)
      {
         activeTexture(i);
         bindTexture(target, 0);
      }
   }

   activeTexture(cachedActiveTextureUnit);
}

// static
void GraphicsContext::setCurrent(GraphicsContext* context)
{
   ASSERT(context);
   currentContext = context;
}

// static
void GraphicsContext::onDestroy(GraphicsContext* context)
{
   if (currentContext == context)
   {
      currentContext = nullptr;
   }
}

// static
GraphicsContext* GraphicsContext::currentContext = nullptr;
