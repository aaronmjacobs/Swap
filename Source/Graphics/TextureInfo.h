#pragma once

#include <glad/glad.h>

namespace Tex
{
   enum class Target : GLenum
   {
      // 1D
      Texture1D = GL_TEXTURE_1D,
      ProxyTexture1D = GL_PROXY_TEXTURE_1D,

      // Buffer
      TextureBuffer = GL_TEXTURE_BUFFER,

      // 2D
      Texture2D = GL_TEXTURE_2D,
      ProxyTexture2D = GL_PROXY_TEXTURE_2D,
      Texture1D_Array = GL_TEXTURE_1D_ARRAY,
      ProxyTexture1D_Array = GL_PROXY_TEXTURE_1D_ARRAY,
      TextureRectangle = GL_TEXTURE_RECTANGLE,
      ProxyTextureRectangle = GL_PROXY_TEXTURE_RECTANGLE,

      // 2D multisample
      Texture2D_Multisample = GL_TEXTURE_2D_MULTISAMPLE,
      ProxyTexture2D_Multisample = GL_PROXY_TEXTURE_2D_MULTISAMPLE,

      // Cube map
      TextureCubeMap = GL_TEXTURE_CUBE_MAP,
      ProxyTextureCubeMap = GL_PROXY_TEXTURE_CUBE_MAP,
      TextureCubeMapPositiveX = GL_TEXTURE_CUBE_MAP_POSITIVE_X,
      TextureCubeMapNegativeX = GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
      TextureCubeMapPositiveY = GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
      TextureCubeMapNegativeY = GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
      TextureCubeMapPositiveZ = GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
      TextureCubeMapNegativeZ = GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,

      // 3D
      Texture3D = GL_TEXTURE_3D,
      ProxyTexture3D = GL_PROXY_TEXTURE_3D,
      Texture2D_Array = GL_TEXTURE_2D_ARRAY,
      ProxyTexture2D_Array = GL_PROXY_TEXTURE_2D_ARRAY,

      // 3D multisample
      Texture2D_MultisampleArray = GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
      ProxyTexture2D_MultisampleArray = GL_PROXY_TEXTURE_2D_MULTISAMPLE_ARRAY
   };

   enum class InternalFormat : GLint
   {
      // R
      R8 = GL_R8,
      R8I = GL_R8I,
      R8UI = GL_R8UI,
      R8_SNorm = GL_R8_SNORM,

      R16F = GL_R16F,
      R16I = GL_R16I,
      R16UI = GL_R16UI,
      R16_SNorm = GL_R16_SNORM,

      R32F = GL_R32F,
      R32I = GL_R32I,
      R32UI = GL_R32UI,


      // RG
      RG8 = GL_RG8,
      RG8I = GL_RG8I,
      RG8UI = GL_RG8UI,
      RG8_SNorm = GL_RG8_SNORM,

      RG16 = GL_RG16,
      RG16F = GL_RG16F,
      RG16_SNorm = GL_RG16_SNORM,

      RG32F = GL_RG32F,
      RG32I = GL_RG32I,
      RG32UI = GL_RG32UI,


      // RGB
      RGB8 = GL_RGB8,
      RGB8I = GL_RGB8I,
      RGB8UI = GL_RGB8UI,
      RGB8_SNorm = GL_RGB8_SNORM,

      RGB16 = GL_RGB16,
      RGB16F = GL_RGB16F,
      RGB16I = GL_RGB16I,
      RGB16UI = GL_RGB16UI,
      RGB16_SNorm = GL_RGB16_SNORM,

      RGB32F = GL_RGB32F,
      RGB32I = GL_RGB32I,
      RGB32UI = GL_RGB32UI,


      // RGBA
      RGBA8 = GL_RGBA8,
      RGBA8UI = GL_RGBA8UI,
      RGBA8_SNorm = GL_RGBA8_SNORM,

      RGBA16 = GL_RGBA16,
      RGBA16F = GL_RGBA16F,
      RGBA16I = GL_RGBA16I,
      RGBA16UI = GL_RGBA16UI,
      RGBA16_SNorm = GL_RGBA16_SNORM,

      RGBA32F = GL_RGBA32F,
      RGBA32I = GL_RGBA32I,
      RGBA32UI = GL_RGBA32UI,


      // SRGB
      SRGB8 = GL_SRGB8,
      SRGB8_Alpha8 = GL_SRGB8_ALPHA8,


      // Compressed
      CompressedRed = GL_COMPRESSED_RED,
      CompressedRedRGTC1 = GL_COMPRESSED_RED_RGTC1,
      CompressedSignedRedRGTC1 = GL_COMPRESSED_SIGNED_RED_RGTC1,
      CompressedRG = GL_COMPRESSED_RG,
      CompressedRG_RGTC2 = GL_COMPRESSED_RG_RGTC2,
      CompressedSignedRG_RGTC2 = GL_COMPRESSED_SIGNED_RG_RGTC2,
      CompressedRGB = GL_COMPRESSED_RGB,
      CompressedRGBA = GL_COMPRESSED_RGBA,
      CompressedSRGB = GL_COMPRESSED_SRGB,
      CompressedSRGB_Alpha = GL_COMPRESSED_SRGB_ALPHA,


      // Depth / stencil
      DepthComponent16 = GL_DEPTH_COMPONENT16,
      DepthComponent24 = GL_DEPTH_COMPONENT24,
      DepthComponent32F = GL_DEPTH_COMPONENT32F,

      Depth24Stencil8 = GL_DEPTH24_STENCIL8,
      Depth32FStencil8 = GL_DEPTH32F_STENCIL8,


      // Other
      RGB10_A2 = GL_RGB10_A2,
      RGB10_A2UI = GL_RGB10_A2UI,
      R11F_G11F_B10F = GL_R11F_G11F_B10F,
      RGB9_E5 = GL_RGB9_E5
   };

   enum class ProvidedDataFormat : GLenum
   {
      Red = GL_RED,
      RG = GL_RG,
      RGB = GL_RGB,
      BGR = GL_BGR,
      RGBA = GL_RGBA,
      BGRA = GL_BGRA,
      DepthComponent = GL_DEPTH_COMPONENT,
      DepthStencil = GL_DEPTH_STENCIL
   };

   enum class ProvidedDataType : GLenum
   {
      // From specification
      Byte = GL_BYTE,
      UnsignedByte = GL_UNSIGNED_BYTE,
      Short = GL_SHORT,
      UnsignedShort = GL_UNSIGNED_SHORT,
      Int = GL_INT,
      UnsignedInt = GL_UNSIGNED_INT,
      Float = GL_FLOAT,
      HalfFloat = GL_HALF_FLOAT,
      UnsignedByte_3_3_2 = GL_UNSIGNED_BYTE_3_3_2,
      UnsignedByte_2_3_3_Rev = GL_UNSIGNED_BYTE_2_3_3_REV,
      UnsignedShort_5_6_5 = GL_UNSIGNED_SHORT_5_6_5,
      UnsignedShort_5_6_5_Rev = GL_UNSIGNED_SHORT_5_6_5_REV,
      UnsignedShort_4_4_4_4 = GL_UNSIGNED_SHORT_4_4_4_4,
      UnsignedShort_4_4_4_4_Rev = GL_UNSIGNED_SHORT_4_4_4_4_REV,
      UnsignedShort_5_5_5_1 = GL_UNSIGNED_SHORT_5_5_5_1,
      UnsignedShort_1_5_5_5_Rev = GL_UNSIGNED_SHORT_1_5_5_5_REV,
      UnsignedInt_8_8_8_8 = GL_UNSIGNED_INT_8_8_8_8,
      UnsignedInt_8_8_8_8_Rev = GL_UNSIGNED_INT_8_8_8_8_REV,
      UnsignedInt_10_10_10_2 = GL_UNSIGNED_INT_10_10_10_2,
      UnsignedInt_2_10_10_10_Rev = GL_UNSIGNED_INT_2_10_10_10_REV,

      // Missing from specification, but needed?
      UnsignedInt_24_8 = GL_UNSIGNED_INT_24_8
   };

   struct Specification
   {
      Target target = Target::Texture2D;
      GLint level = 0;
      GLsizei samples = 0;
      InternalFormat internalFormat = InternalFormat::RGB8;
      GLsizei width = 256;
      GLsizei height = 256;
      GLsizei depth = 256;
      ProvidedDataFormat providedDataFormat = ProvidedDataFormat::RGB;
      ProvidedDataType providedDataType = ProvidedDataType::UnsignedByte;
      const GLvoid* providedData = nullptr;
      bool fixedSampleLocations = false;
      GLuint buffer = 0;
      const GLvoid* positiveXData = nullptr;
      const GLvoid* negativeXData = nullptr;
      const GLvoid* positiveYData = nullptr;
      const GLvoid* negativeYData = nullptr;
      const GLvoid* positiveZData = nullptr;
      const GLvoid* negativeZData = nullptr;
   };

   enum class FloatParam : GLenum
   {
      TextureLodBias = GL_TEXTURE_LOD_BIAS,
      TextureMinLod = GL_TEXTURE_MIN_LOD,
      TextureMaxLod = GL_TEXTURE_MAX_LOD,
   };

   enum class IntParam : GLenum
   {
      TextureBaseLevel = GL_TEXTURE_BASE_LEVEL,
      TextureMaxLevel = GL_TEXTURE_MAX_LEVEL,
      TextureCompareFunc = GL_TEXTURE_COMPARE_FUNC,
      TextureCompareMode = GL_TEXTURE_COMPARE_MODE,
      TextureMinFilter = GL_TEXTURE_MIN_FILTER,
      TextureMagFilter = GL_TEXTURE_MAG_FILTER,
      TextureSwizzleR = GL_TEXTURE_SWIZZLE_R,
      TextureSwizzleG = GL_TEXTURE_SWIZZLE_G,
      TextureSwizzleB = GL_TEXTURE_SWIZZLE_B,
      TextureSwizzleA = GL_TEXTURE_SWIZZLE_A,
      TextureWrapS = GL_TEXTURE_WRAP_S,
      TextureWrapT = GL_TEXTURE_WRAP_T,
      TextureWrapR = GL_TEXTURE_WRAP_R,
   };

   enum class FloatArrayParam : GLenum
   {
      TextureBorderColor = GL_TEXTURE_BORDER_COLOR,
   };

   enum class IntArrayParam : GLenum
   {
      TextureBorderColor = GL_TEXTURE_BORDER_COLOR,
      TextureSwizzleRGBA = GL_TEXTURE_SWIZZLE_RGBA,
   };

   enum class InternalIntArrayParam : GLenum
   {
      TextureBorderColor = GL_TEXTURE_BORDER_COLOR,
   };

   enum class InternalUintArrayParam : GLenum
   {
      TextureBorderColor = GL_TEXTURE_BORDER_COLOR,
   };
}
