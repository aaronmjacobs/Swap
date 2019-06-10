#include "Graphics/Framebuffer.h"

#include "Core/Assert.h"
#include "Graphics/Texture.h"

#include <utility>

Fb::Attachments Fb::generateAttachments(const Specification& specification)
{
   Attachments attachments;

   bool isMultisample = specification.samples > 0;
   GLenum target = isMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

   if (specification.depthStencilType != DepthStencilType::None)
   {
      Tex::Specification depthStencilSpecification;

      depthStencilSpecification.width = specification.width;
      depthStencilSpecification.height = specification.height;

      if (isMultisample)
      {
         depthStencilSpecification.target = Tex::Target::Texture2D_Multisample;
         depthStencilSpecification.samples = specification.samples;
      }

      bool highPrecision = specification.depthStencilType == DepthStencilType::Depth32FStencil8;
      depthStencilSpecification.internalFormat = highPrecision ? Tex::InternalFormat::Depth32FStencil8 : Tex::InternalFormat::Depth24Stencil8;
      depthStencilSpecification.providedDataFormat = Tex::ProvidedDataFormat::DepthStencil;
      depthStencilSpecification.providedDataType = Tex::ProvidedDataType::UnsignedInt_24_8;

      attachments.depthStencilAttachment = std::make_shared<Texture>(depthStencilSpecification);

      if (!isMultisample)
      {
         attachments.depthStencilAttachment->setParam(Tex::IntParam::TextureMinFilter, GL_NEAREST);
         attachments.depthStencilAttachment->setParam(Tex::IntParam::TextureMagFilter, GL_NEAREST);
         attachments.depthStencilAttachment->setParam(Tex::IntParam::TextureWrapS, GL_CLAMP_TO_EDGE);
         attachments.depthStencilAttachment->setParam(Tex::IntParam::TextureWrapT, GL_CLAMP_TO_EDGE);
      }
   }

   Tex::Specification colorSpecification;
   colorSpecification.width = specification.width;
   colorSpecification.height = specification.height;

   if (isMultisample)
   {
      colorSpecification.target = Tex::Target::Texture2D_Multisample;
      colorSpecification.samples = specification.samples;
   }

   attachments.colorAttachments.resize(specification.colorAttachmentFormats.size());
   std::size_t attachmentIndex = 0;
   for (SPtr<Texture>& colorAttachment : attachments.colorAttachments)
   {
      colorSpecification.internalFormat = specification.colorAttachmentFormats[attachmentIndex++];
      colorAttachment = std::make_shared<Texture>(colorSpecification);

      if (!isMultisample)
      {
         colorAttachment->setParam(Tex::IntParam::TextureMinFilter, GL_LINEAR);
         colorAttachment->setParam(Tex::IntParam::TextureMagFilter, GL_LINEAR);
         colorAttachment->setParam(Tex::IntParam::TextureWrapS, GL_CLAMP_TO_EDGE);
         colorAttachment->setParam(Tex::IntParam::TextureWrapT, GL_CLAMP_TO_EDGE);
      }
   }

   return attachments;
}

Framebuffer::Framebuffer()
   : id(0)
{
   glGenFramebuffers(1, &id);
}

Framebuffer::Framebuffer(Framebuffer&& other)
{
   move(std::move(other));
}

Framebuffer::~Framebuffer()
{
   release();
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void Framebuffer::move(Framebuffer&& other)
{
   ASSERT(id == 0);

   id = other.id;
   other.id = 0;

   attachments = std::move(other.attachments);
}

void Framebuffer::release()
{
   attachments = {};

   if (id != 0)
   {
      glDeleteFramebuffers(1, &id);
      id = 0;
   }
}

// static
void Framebuffer::bindDefault()
{
   glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::bind()
{
   ASSERT(id != 0);

   glBindFramebuffer(GL_FRAMEBUFFER, id);
}

void Framebuffer::setAttachments(Fb::Attachments newAttachments)
{
   bind();

   attachments = std::move(newAttachments);

   bool isMultisample = false;
   if (attachments.depthStencilAttachment)
   {
      isMultisample = attachments.depthStencilAttachment->isMultisample();
   }
   else if (attachments.colorAttachments.size() > 0)
   {
      isMultisample = attachments.colorAttachments[0]->isMultisample();
   }
   GLenum target = isMultisample ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

   if (attachments.depthStencilAttachment)
   {
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, target, attachments.depthStencilAttachment->getId(), 0);
   }

   std::vector<GLenum> drawBuffers(attachments.colorAttachments.size());
   std::size_t attachmentIndex = 0;
   for (const SPtr<Texture>& colorAttachment : attachments.colorAttachments)
   {
      drawBuffers[attachmentIndex] = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + attachmentIndex);
      glFramebufferTexture2D(GL_FRAMEBUFFER, drawBuffers[attachmentIndex], target, colorAttachment->getId(), 0);
      ++attachmentIndex;
   }
   glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());

   ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
   bindDefault();
}
