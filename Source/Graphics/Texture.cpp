#include "Graphics/Texture.h"

#include "Core/Assert.h"

#include <glm/gtc/type_ptr.hpp>

namespace
{
   template<typename T>
   bool matchesAny(T item)
   {
      return false;
   }

   template<typename T, T First, T... Rest>
   bool matchesAny(T item)
   {
      return item == First || matchesAny<T, Rest...>(item);
   }

   void verifySpecification(const Tex::Specification& spec)
   {
#if SWAP_DEBUG
      ASSERT(spec.level >= 0);
      ASSERT(spec.samples >= 0);

      GLint maxTextureSize = 0;
      glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
      ASSERT(spec.width >= 0 && spec.height >= 0 && spec.depth >= 0 &&
         spec.width <= maxTextureSize && spec.height <= maxTextureSize && spec.depth <= maxTextureSize);

      switch (spec.target)
      {
      case Tex::Target::Texture1D:
      case Tex::Target::ProxyTexture1D:
         ASSERT((matchesAny<GLenum,
            GL_COMPRESSED_RED, GL_COMPRESSED_RG, GL_COMPRESSED_RGB, GL_COMPRESSED_RGBA, GL_COMPRESSED_SRGB,
            GL_COMPRESSED_SRGB_ALPHA, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT32,
            GL_R3_G3_B2, GL_RED, GL_RG, GL_RGB, GL_RGB4, GL_RGB5, GL_RGB8, GL_RGB10, GL_RGB12, GL_RGB16, GL_RGBA, GL_RGBA2,
            GL_RGBA4, GL_RGB5_A1, GL_RGBA8, GL_RGB10_A2, GL_RGBA12, GL_RGBA16, GL_SRGB, GL_SRGB8, GL_SRGB_ALPHA,
            GL_SRGB8_ALPHA8>(static_cast<GLenum>(spec.internalFormat)), "Invalid internal format for glTexImage1D()"));
         break;
      case Tex::Target::TextureBuffer:
         ASSERT((matchesAny<GLenum, GL_R8, GL_R16, GL_R16F, GL_R32F, GL_R8I, GL_R16I, GL_R32I, GL_R8UI, GL_R16UI, GL_R32UI,
            GL_RG8, GL_RG16, GL_RG16F, GL_RG32F, GL_RG8I, GL_RG16I, GL_RG32I, GL_RG8UI, GL_RG16UI, GL_RG32UI, GL_RGBA8,
            GL_RGBA16, GL_RGBA16F, GL_RGBA32F, GL_RGBA8I, GL_RGBA16I, GL_RGBA32I, GL_RGBA8UI, GL_RGBA16UI,
            GL_RGBA32UI>(static_cast<GLenum>(spec.internalFormat)), "Invalid internal format for glTexBuffer()"));
         break;
      case Tex::Target::Texture2D:
      case Tex::Target::ProxyTexture2D:
      case Tex::Target::Texture1D_Array:
      case Tex::Target::ProxyTexture1D_Array:
      case Tex::Target::TextureRectangle:
      case Tex::Target::ProxyTextureRectangle:
         ASSERT((matchesAny<GLenum,
            GL_RGBA32F, GL_RGBA32I, GL_RGBA32UI, GL_RGBA16, GL_RGBA16F, GL_RGBA16I, GL_RGBA16UI, GL_RGBA8, GL_RGBA8UI,
            GL_SRGB8_ALPHA8, GL_RGB10_A2, GL_RGB10_A2UI, GL_R11F_G11F_B10F, GL_RG32F, GL_RG32I, GL_RG32UI, GL_RG16,
            GL_RG16F, GL_RGB16I, GL_RGB16UI, GL_RG8, GL_RG8I, GL_RG8UI, GL_R32F, GL_R32I, GL_R32UI, GL_R16F, GL_R16I,
            GL_R16UI, GL_R8, GL_R8I, GL_R8UI, GL_RGBA16_SNORM, GL_RGBA8_SNORM, GL_RGB32F, GL_RGB32I, GL_RGB32UI,
            GL_RGB16_SNORM, GL_RGB16F, GL_RGB16I, GL_RGB16UI, GL_RGB16, GL_RGB8_SNORM, GL_RGB8, GL_RGB8I, GL_RGB8UI,
            GL_SRGB8, GL_RGB9_E5, GL_RG16_SNORM, GL_RG8_SNORM, GL_COMPRESSED_RG_RGTC2, GL_COMPRESSED_SIGNED_RG_RGTC2,
            GL_R16_SNORM, GL_R8_SNORM, GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_SIGNED_RED_RGTC1, GL_DEPTH_COMPONENT32F,
            GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT16, GL_DEPTH32F_STENCIL8, GL_DEPTH24_STENCIL8>
            (static_cast<GLenum>(spec.internalFormat)), "Invalid internal format for glTexImage2D()"));
         break;
      case Tex::Target::Texture2D_Multisample:
      case Tex::Target::ProxyTexture2D_Multisample:
         break;
      case Tex::Target::TextureCubeMap:
      case Tex::Target::ProxyTextureCubeMap:
         ASSERT((matchesAny<GLenum,
            GL_RGBA32F, GL_RGBA32I, GL_RGBA32UI, GL_RGBA16, GL_RGBA16F, GL_RGBA16I, GL_RGBA16UI, GL_RGBA8, GL_RGBA8UI,
            GL_SRGB8_ALPHA8, GL_RGB10_A2, GL_RGB10_A2UI, GL_R11F_G11F_B10F, GL_RG32F, GL_RG32I, GL_RG32UI, GL_RG16,
            GL_RG16F, GL_RGB16I, GL_RGB16UI, GL_RG8, GL_RG8I, GL_RG8UI, GL_R32F, GL_R32I, GL_R32UI, GL_R16F, GL_R16I,
            GL_R16UI, GL_R8, GL_R8I, GL_R8UI, GL_RGBA16_SNORM, GL_RGBA8_SNORM, GL_RGB32F, GL_RGB32I, GL_RGB32UI,
            GL_RGB16_SNORM, GL_RGB16F, GL_RGB16I, GL_RGB16UI, GL_RGB16, GL_RGB8_SNORM, GL_RGB8, GL_RGB8I, GL_RGB8UI,
            GL_SRGB8, GL_RGB9_E5, GL_RG16_SNORM, GL_RG8_SNORM, GL_COMPRESSED_RG_RGTC2, GL_COMPRESSED_SIGNED_RG_RGTC2,
            GL_R16_SNORM, GL_R8_SNORM, GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_SIGNED_RED_RGTC1, GL_DEPTH_COMPONENT32F,
            GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT16, GL_DEPTH32F_STENCIL8, GL_DEPTH24_STENCIL8>
            (static_cast<GLenum>(spec.internalFormat)), "Invalid internal format for glTexImage2D()"));
         break;
      case Tex::Target::Texture3D:
      case Tex::Target::ProxyTexture3D:
      case Tex::Target::Texture2D_Array:
      case Tex::Target::ProxyTexture2D_Array:
         ASSERT((matchesAny<GLenum,
            GL_RGBA32F, GL_RGBA32I, GL_RGBA32UI, GL_RGBA16, GL_RGBA16F, GL_RGBA16I, GL_RGBA16UI, GL_RGBA8, GL_RGBA8UI,
            GL_SRGB8_ALPHA8, GL_RGB10_A2, GL_RGB10_A2UI, GL_R11F_G11F_B10F, GL_RG32F, GL_RG32I, GL_RG32UI, GL_RG16,
            GL_RG16F, GL_RGB16I, GL_RGB16UI, GL_RG8, GL_RG8I, GL_RG8UI, GL_R32F, GL_R32I, GL_R32UI, GL_R16F, GL_R16I,
            GL_R16UI, GL_R8, GL_R8I, GL_R8UI, GL_RGBA16_SNORM, GL_RGBA8_SNORM, GL_RGB32F, GL_RGB32I, GL_RGB32UI,
            GL_RGB16_SNORM, GL_RGB16F, GL_RGB16I, GL_RGB16UI, GL_RGB16, GL_RGB8_SNORM, GL_RGB8, GL_RGB8I, GL_RGB8UI,
            GL_SRGB8, GL_RGB9_E5, GL_RG16_SNORM, GL_RG8_SNORM, GL_COMPRESSED_RG_RGTC2, GL_COMPRESSED_SIGNED_RG_RGTC2,
            GL_R16_SNORM, GL_R8_SNORM, GL_COMPRESSED_RED_RGTC1, GL_COMPRESSED_SIGNED_RED_RGTC1, GL_DEPTH_COMPONENT32F,
            GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT16, GL_DEPTH32F_STENCIL8, GL_DEPTH24_STENCIL8>
            (static_cast<GLenum>(spec.internalFormat)), "Invalid internal format for glTexImage3D()"));
         break;
      case Tex::Target::Texture2D_MultisampleArray:
      case Tex::Target::ProxyTexture2D_MultisampleArray:
         break;
      default:
         ASSERT(false, "Invalid texture target: %u", spec.target);
         break;
      }
#endif // SWAP_DEBUG
   }

#if SWAP_DEBUG
   void verifyParamTarget(Tex::Target target)
   {
      ASSERT(target == Tex::Target::Texture1D ||
         target == Tex::Target::Texture2D ||
         target == Tex::Target::Texture3D ||
         target == Tex::Target::Texture1D_Array ||
         target == Tex::Target::Texture2D_Array ||
         target == Tex::Target::TextureRectangle ||
         target == Tex::Target::TextureCubeMap,
         "Invalid texture target for setting texture parameters: %u", target);
   }

   void verifySwizzleValue(GLint value)
   {
      ASSERT(value == GL_RED || value == GL_GREEN || value == GL_BLUE || value == GL_ALPHA || value == GL_ZERO ||
         value == GL_ONE, "Invalid texture swizzle value: 0x%X", value);
   }
#endif // SWAP_DEBUG

   void verifyParam(Tex::Target target, Tex::FloatParam param, GLfloat value)
   {
#if SWAP_DEBUG
      verifyParamTarget(target);

      switch (param)
      {
      case Tex::FloatParam::TextureLodBias:
      case Tex::FloatParam::TextureMinLod:
      case Tex::FloatParam::TextureMaxLod:
         break;
      default:
         ASSERT(false, "Invalid texture float param: 0x%X", param);
         break;
      }
#endif // SWAP_DEBUG
   }

   void verifyParam(Tex::Target target, Tex::IntParam param, GLint value)
   {
#if SWAP_DEBUG
      verifyParamTarget(target);

      switch (param)
      {
      case Tex::IntParam::TextureBaseLevel:
      case Tex::IntParam::TextureMaxLevel:
         ASSERT(value >= 0, "Texture base / max levels should not be negative: %d", value);
         break;
      case Tex::IntParam::TextureCompareFunc:
         ASSERT(value == GL_LEQUAL || value == GL_GEQUAL || value == GL_LESS || value == GL_GREATER || value == GL_EQUAL ||
            value == GL_NOTEQUAL || value == GL_ALWAYS || value == GL_NEVER,
            "Invalid texture compare func: 0x%X", value);
         break;
      case Tex::IntParam::TextureCompareMode:
         ASSERT(value == GL_COMPARE_REF_TO_TEXTURE || value == GL_NONE, "Invalid texture compare mode: 0x%X", value);
         break;
      case Tex::IntParam::TextureMinFilter:
         ASSERT(value == GL_NEAREST || value == GL_LINEAR || value == GL_NEAREST_MIPMAP_NEAREST ||
            value == GL_LINEAR_MIPMAP_NEAREST || value == GL_NEAREST_MIPMAP_LINEAR || value == GL_LINEAR_MIPMAP_LINEAR,
            "Invalid texture min filter: 0x%X", value);
         break;
      case Tex::IntParam::TextureMagFilter:
         ASSERT(value == GL_NEAREST || value == GL_LINEAR, "Invalid texture mag filter: 0x%X", value);
         break;
      case Tex::IntParam::TextureSwizzleR:
      case Tex::IntParam::TextureSwizzleG:
      case Tex::IntParam::TextureSwizzleB:
      case Tex::IntParam::TextureSwizzleA:
         verifySwizzleValue(value);
         break;
      case Tex::IntParam::TextureWrapS:
      case Tex::IntParam::TextureWrapT:
      case Tex::IntParam::TextureWrapR:
         ASSERT(value == GL_CLAMP_TO_EDGE || value == GL_CLAMP_TO_BORDER || value == GL_MIRRORED_REPEAT ||
            value == GL_REPEAT, "Invalid texture wrap value: 0x%X", value);
         break;
      default:
         ASSERT(false, "Invalid texture int param: 0x%X", param);
         break;
      }
#endif // SWAP_DEBUG
   }

   void verifyParam(Tex::Target target, Tex::FloatArrayParam param, const glm::vec4& value)
   {
#if SWAP_DEBUG
      verifyParamTarget(target);

      switch (param) {
      case Tex::FloatArrayParam::TextureBorderColor:
         break;
      default:
         ASSERT(false, "Invalid texture float array param: 0x%X", param);
      }
#endif // SWAP_DEBUG
   }

   void verifyParam(Tex::Target target, Tex::IntArrayParam param, const glm::ivec4& value)
   {
#if SWAP_DEBUG
      verifyParamTarget(target);

      switch (param) {
      case Tex::IntArrayParam::TextureBorderColor:
         break;
      case Tex::IntArrayParam::TextureSwizzleRGBA:
         verifySwizzleValue(value.r);
         verifySwizzleValue(value.g);
         verifySwizzleValue(value.b);
         verifySwizzleValue(value.a);
         break;
      default:
         ASSERT(false, "Invalid texture int array param: 0x%X", param);
      }
#endif // SWAP_DEBUG
   }

   void verifyParam(Tex::Target target, Tex::InternalIntArrayParam param, const glm::ivec4& value)
   {
#if SWAP_DEBUG
      verifyParamTarget(target);

      switch (param) {
      case Tex::InternalIntArrayParam::TextureBorderColor:
         break;
      default:
         ASSERT(false, "Invalid texture internal int array param: 0x%X", param);
      }
#endif // SWAP_DEBUG
   }

   void verifyParam(Tex::Target target, Tex::InternalUintArrayParam param, const glm::uvec4& value)
   {
#if SWAP_DEBUG
      verifyParamTarget(target);

      switch (param)
      {
      case Tex::InternalUintArrayParam::TextureBorderColor:
         break;
      default:
         ASSERT(false, "Invalid texture internal uint array param: 0x%X", param);
      }
#endif // SWAP_DEBUG
   }
}

Texture::Texture(const Tex::Specification& textureSpecification)
   : specification(textureSpecification)
   , id(0)
{
   glGenTextures(1, &id);
   updateSpecification(textureSpecification);
}

Texture::Texture(Texture&& other)
{
   move(std::move(other));
}

Texture& Texture::operator=(Texture &&other)
{
   release();
   move(std::move(other));
   return *this;
}

Texture::~Texture()
{
   release();
}

void Texture::move(Texture&& other)
{
   id = other.id;
   specification = other.specification;

   other.id = 0;
}

void Texture::release()
{
   if (id != 0)
   {
      glDeleteTextures(1, &id);
   }
}

void Texture::bind()
{
   glBindTexture(static_cast<GLenum>(specification.target), id);
}

void Texture::updateSpecification(const Tex::Specification& textureSpecification)
{
   ASSERT(textureSpecification.target == specification.target, "Can not change texture target after it is initially set");
   verifySpecification(textureSpecification);

   specification = textureSpecification;
   bind();

   GLenum target = static_cast<GLenum>(specification.target);
   GLint level = specification.level;
   GLsizei samples = specification.samples;
   GLint internalFormat = static_cast<GLint>(specification.internalFormat);
   GLsizei width = specification.width;
   GLsizei height = specification.height;
   GLsizei depth = specification.depth;
   GLint border = 0;
   GLenum format = static_cast<GLenum>(specification.providedDataFormat);
   GLenum type = static_cast<GLenum>(specification.providedDataType);
   const GLvoid* data = specification.providedData;
   bool fixedSampleLocations = specification.fixedSampleLocations;
   GLuint buffer = specification.buffer;

   switch (specification.target)
   {
   case Tex::Target::Texture1D:
   case Tex::Target::ProxyTexture1D:
      glTexImage1D(target, level, internalFormat, width, border, format, type, data);
      break;
   case Tex::Target::TextureBuffer:
      glTexBuffer(target, internalFormat, buffer);
      break;
   case Tex::Target::Texture2D:
   case Tex::Target::ProxyTexture2D:
   case Tex::Target::Texture1D_Array:
   case Tex::Target::ProxyTexture1D_Array:
   case Tex::Target::TextureRectangle:
   case Tex::Target::ProxyTextureRectangle:
      glTexImage2D(target, level, internalFormat, width, height, border, format, type, data);
      break;
   case Tex::Target::Texture2D_Multisample:
   case Tex::Target::ProxyTexture2D_Multisample:
      glTexImage2DMultisample(target, samples, internalFormat, width, height, fixedSampleLocations);
      break;
   case Tex::Target::TextureCubeMap:
   case Tex::Target::ProxyTextureCubeMap:
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, level, internalFormat, width, height, border, format, type, specification.positiveXData);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, level, internalFormat, width, height, border, format, type, specification.negativeXData);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, level, internalFormat, width, height, border, format, type, specification.positiveYData);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, level, internalFormat, width, height, border, format, type, specification.negativeYData);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, level, internalFormat, width, height, border, format, type, specification.positiveZData);
      glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, level, internalFormat, width, height, border, format, type, specification.negativeZData);
      break;
   case Tex::Target::Texture3D:
   case Tex::Target::ProxyTexture3D:
   case Tex::Target::Texture2D_Array:
   case Tex::Target::ProxyTexture2D_Array:
      glTexImage3D(target, level, internalFormat, width, height, depth, border, format, type, data);
      break;
   case Tex::Target::Texture2D_MultisampleArray:
   case Tex::Target::ProxyTexture2D_MultisampleArray:
      glTexImage3DMultisample(target, samples, internalFormat, width, height, depth, fixedSampleLocations);
      break;
   default:
      ASSERT(false, "Invalid texture target: %u", specification.target);
      break;
   }
}

void Texture::setParam(Tex::FloatParam param, GLfloat value)
{
   assertBound();
   verifyParam(specification.target, param, value);

   glTexParameterf(static_cast<GLenum>(specification.target), static_cast<GLenum>(param), value);
}

void Texture::setParam(Tex::IntParam param, GLint value)
{
   assertBound();
   verifyParam(specification.target, param, value);

   glTexParameteri(static_cast<GLenum>(specification.target), static_cast<GLenum>(param), value);
}

void Texture::setParam(Tex::FloatArrayParam param, const glm::vec4& value)
{
   assertBound();
   verifyParam(specification.target, param, value);

   glTexParameterfv(static_cast<GLenum>(specification.target), static_cast<GLenum>(param), glm::value_ptr(value));
}

void Texture::setParam(Tex::IntArrayParam param, const glm::ivec4& value)
{
   assertBound();
   verifyParam(specification.target, param, value);

   glTexParameteriv(static_cast<GLenum>(specification.target), static_cast<GLenum>(param), glm::value_ptr(value));
}

void Texture::setParam(Tex::InternalIntArrayParam param, const glm::ivec4& value)
{
   assertBound();
   verifyParam(specification.target, param, value);

   glTexParameterIiv(static_cast<GLenum>(specification.target), static_cast<GLenum>(param), glm::value_ptr(value));
}

void Texture::setParam(Tex::InternalUintArrayParam param, const glm::uvec4& value)
{
   assertBound();
   verifyParam(specification.target, param, value);

   glTexParameterIuiv(static_cast<GLenum>(specification.target), static_cast<GLenum>(param), glm::value_ptr(value));
}

void Texture::generateMipMaps()
{
   assertBound();
   ASSERT(specification.target == Tex::Target::Texture1D ||
      specification.target == Tex::Target::Texture2D ||
      specification.target == Tex::Target::Texture3D ||
      specification.target == Tex::Target::Texture1D_Array ||
      specification.target == Tex::Target::Texture2D_Array ||
      specification.target == Tex::Target::TextureCubeMap,
      "Invalid texture target for generating mip maps: %u", specification.target);

   glGenerateMipmap(static_cast<GLenum>(specification.target));
}

void Texture::assertBound() const
{
#if SWAP_DEBUG
   static const auto getBindingQueryEnum = [](Tex::Target target) -> GLenum
   {
      switch (target)
      {
      case Tex::Target::Texture1D:
      case Tex::Target::ProxyTexture1D:
         return GL_TEXTURE_BINDING_1D;
      case Tex::Target::TextureBuffer:
         return GL_TEXTURE_BINDING_BUFFER;
      case Tex::Target::Texture2D:
      case Tex::Target::ProxyTexture2D:
         return GL_TEXTURE_BINDING_2D;
      case Tex::Target::Texture2D_Multisample:
      case Tex::Target::ProxyTexture2D_Multisample:
         return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
      case Tex::Target::Texture1D_Array:
      case Tex::Target::ProxyTexture1D_Array:
         return GL_TEXTURE_BINDING_1D_ARRAY;
      case Tex::Target::TextureRectangle:
      case Tex::Target::ProxyTextureRectangle:
         return GL_TEXTURE_BINDING_RECTANGLE;
      case Tex::Target::TextureCubeMap:
      case Tex::Target::ProxyTextureCubeMap:
         return GL_TEXTURE_BINDING_CUBE_MAP;
      case Tex::Target::Texture3D:
      case Tex::Target::ProxyTexture3D:
         return GL_TEXTURE_BINDING_3D;
      case Tex::Target::Texture2D_Array:
      case Tex::Target::ProxyTexture2D_Array:
         return GL_TEXTURE_BINDING_2D_ARRAY;
      case Tex::Target::Texture2D_MultisampleArray:
      case Tex::Target::ProxyTexture2D_MultisampleArray:
         return GL_TEXTURE_BINDING_2D_MULTISAMPLE;
      default:
         ASSERT(false, "Invalid texture target: %u", target);
         return GL_TEXTURE_BINDING_2D;
      }
   };

   ASSERT(id != 0);

   GLint textureBinding = 0;
   glGetIntegerv(getBindingQueryEnum(specification.target), &textureBinding);

   ASSERT(static_cast<GLuint>(textureBinding) == id);
#endif // SWAP_DEBUG
}
