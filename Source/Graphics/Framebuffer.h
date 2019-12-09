#pragma once

#include "Core/Pointers.h"
#include "Graphics/GraphicsResource.h"
#include "Graphics/TextureInfo.h"

#include <glad/gl.h>
#include <gsl/span>

#include <vector>

class Texture;
struct Viewport;

namespace Fb
{
   enum class DepthStencilType
   {
      None,
      Depth24Stencil8,
      Depth32FStencil8
   };

   enum class Target
   {
      Framebuffer = GL_FRAMEBUFFER,
      ReadFramebuffer = GL_READ_FRAMEBUFFER,
      DrawFramebuffer = GL_DRAW_FRAMEBUFFER
   };

   enum class CubeFace
   {
      Front,
      Back,
      Top,
      Bottom,
      Left,
      Right
   };

   struct Specification
   {
      GLsizei width = 0;
      GLsizei height = 0;

      GLsizei samples = 0;
      bool cubeMap = false;

      DepthStencilType depthStencilType = DepthStencilType::Depth24Stencil8;

      gsl::span<const Tex::InternalFormat> colorAttachmentFormats;

      bool operator==(const Specification& other) const;
   };

   struct Attachments
   {
      SPtr<Texture> depthStencilAttachment;
      std::vector<SPtr<Texture>> colorAttachments;
   };

   Attachments generateAttachments(const Specification& specification);
}

namespace std
{
   template<>
   struct hash<Fb::Specification>
   {
      size_t operator()(const Fb::Specification& specification) const;
   };
}

class Framebuffer : public GraphicsResource
{
public:
   Framebuffer();
   Framebuffer(const Framebuffer& other) = delete;
   Framebuffer(Framebuffer&& other);
   ~Framebuffer();
   Framebuffer& operator=(const Framebuffer& other) = delete;
   Framebuffer& operator=(Framebuffer&& other);

private:
   void move(Framebuffer&& other);
   void release();

public:
   using SpecificationType = Fb::Specification;
   static SPtr<Framebuffer> create(const Fb::Specification& specification);
   static const char* labelSuffix();

   static void blit(Framebuffer& source, Framebuffer& destination, GLenum readBuffer, GLenum drawBuffer, GLbitfield mask, GLenum filter);
   static void bindDefault(Fb::Target target = Fb::Target::Framebuffer);

   void bind(Fb::Target target = Fb::Target::Framebuffer);

   const Fb::Attachments& getAttachments() const
   {
      return attachments;
   }

   const SPtr<Texture>& getDepthStencilAttachment() const;
   SPtr<Texture> getColorAttachment(int index) const;

   void setAttachments(Fb::Attachments newAttachments);

   bool getViewport(Viewport& viewport) const;

   bool isCubeMap() const;
   void setActiveFace(Fb::CubeFace face);

private:
   const SPtr<Texture>* getFirstValidAttachment() const;

   Fb::Attachments attachments;
};
