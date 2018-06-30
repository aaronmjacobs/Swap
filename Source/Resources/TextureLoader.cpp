#include "Resources/TextureLoader.h"

#include "Core/Assert.h"
#include "Core/Log.h"
#include "Graphics/Texture.h"
#include "Resources/DefaultImageSource.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT ASSERT
#include <stb_image.h>

#include <functional>

namespace
{
   using PixelPtr = UPtr<unsigned char[], std::function<decltype(stbi_image_free)>>;

   struct ImageInfo
   {
      int width = 0;
      int height = 0;
      int composition = 0;
      PixelPtr pixels;
   };

   const int kMinComposition = 1;
   const int kMaxComposition = 4;

   PixelPtr createPixelPtr(unsigned char* pixels)
   {
      return PixelPtr(pixels, stbi_image_free);
   }

   bool infoMatches(const ImageInfo& first, const ImageInfo& second)
   {
      return first.width == second.width && first.height == second.height && first.composition == second.composition;
   }

   ImageInfo getDefaultImageInfo()
   {
      ImageInfo defaultInfo;
      defaultInfo.pixels = createPixelPtr(stbi_load_from_memory(kDefaultImageSource.data(),
         static_cast<int>(kDefaultImageSource.size()), &defaultInfo.width, &defaultInfo.height,
         &defaultInfo.composition, 0));

      if (!defaultInfo.pixels || defaultInfo.composition < kMinComposition || defaultInfo.composition > kMaxComposition)
      {
         LOG_ERROR("Unable to load default image");
         defaultInfo.pixels = nullptr;
      }

      return defaultInfo;
   }

   ImageInfo loadImage(const std::string& path)
   {
      ImageInfo info;
      info.pixels = createPixelPtr(stbi_load(path.c_str(), &info.width, &info.height, &info.composition, 0));

      if (!info.pixels || info.composition < kMinComposition || info.composition > kMaxComposition)
      {
         LOG_WARNING("Unable to load image from file: " << path << ", reverting to default");
         info = getDefaultImageInfo();
      }

      return info;
   }

   void setParameters(Texture& texture, Tex::Wrap wrap, Tex::MinFilter minFilter, Tex::MagFilter magFilter)
   {
      if (minFilter == Tex::MinFilter::NearestMipmapNearest || minFilter == Tex::MinFilter::LinearMipmapNearest
         || minFilter == Tex::MinFilter::NearestMipmapLinear || minFilter == Tex::MinFilter::LinearMipmapLinear)
      {
         texture.generateMipMaps();
      }

      texture.setParam(Tex::IntParam::TextureWrapS, static_cast<GLint>(wrap));
      texture.setParam(Tex::IntParam::TextureWrapT, static_cast<GLint>(wrap));
      texture.setParam(Tex::IntParam::TextureWrapR, static_cast<GLint>(wrap));

      texture.setParam(Tex::IntParam::TextureMinFilter, static_cast<GLint>(minFilter));
      texture.setParam(Tex::IntParam::TextureMagFilter, static_cast<GLint>(magFilter));
   }

   Tex::InternalFormat determineInternalFormat(int composition)
   {
      switch (composition)
      {
      case 1:
         return Tex::InternalFormat::R8;
      case 2:
         return Tex::InternalFormat::RG8;
      case 3:
         return Tex::InternalFormat::RGB8;
      case 4:
         return Tex::InternalFormat::RGBA8;
      default:
         ASSERT(false, "Invalid image composition: %d", composition);
         return Tex::InternalFormat::RGB8;
      }
   }

   Tex::ProvidedDataFormat determineProvidedDataFormat(int composition)
   {
      switch (composition)
      {
      case 1:
         return Tex::ProvidedDataFormat::Red;
      case 2:
         return Tex::ProvidedDataFormat::RG;
      case 3:
         return Tex::ProvidedDataFormat::RGB;
      case 4:
         return Tex::ProvidedDataFormat::RGBA;
      default:
         ASSERT(false, "Invalid image composition: %d", composition);
         return Tex::ProvidedDataFormat::RGB;
      }
   }

   SPtr<Texture> createTexture(const ImageInfo& info)
   {
      Tex::Specification specification;

      specification.internalFormat = determineInternalFormat(info.composition);
      specification.width = info.width;
      specification.height = info.height;
      specification.providedDataFormat = determineProvidedDataFormat(info.composition);
      specification.providedDataType = Tex::ProvidedDataType::UnsignedByte;
      specification.providedData = info.pixels.get();

      // stb_image provides tightly packed data, so make sure OpenGL isn't expecting any padding between rows
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      // Don't use make_shared so that the texture can be freed even if we still hold a weak reference
      return SPtr<Texture>(new Texture(specification));
   }

   SPtr<Texture> createCubemap(const std::array<ImageInfo, 6>& infos)
   {
      Tex::Specification specification;

      specification.target = Tex::Target::TextureCubeMap;
      specification.internalFormat = determineInternalFormat(infos[0].composition);
      specification.width = infos[0].width;
      specification.height = infos[0].height;
      specification.providedDataFormat = determineProvidedDataFormat(infos[0].composition);
      specification.providedDataType = Tex::ProvidedDataType::UnsignedByte;
      specification.positiveXData = infos[0].pixels.get();
      specification.negativeXData = infos[1].pixels.get();
      specification.positiveYData = infos[2].pixels.get();
      specification.negativeYData = infos[3].pixels.get();
      specification.positiveZData = infos[4].pixels.get();
      specification.negativeZData = infos[5].pixels.get();

      // stb_image provides tightly packed data, so make sure OpenGL isn't expecting any padding between rows
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      // Don't use make_shared so that the texture can be freed even if we still hold a weak reference
      return SPtr<Texture>(new Texture(specification));
   }
}

SPtr<Texture> TextureLoader::loadTexture(const std::string& path, Tex::Wrap wrap /*= Tex::Wrap::ClampToBorder*/,
   Tex::MinFilter minFilter /*= Tex::MinFilter::LinearMipmapLinear*/,
   Tex::MagFilter magFilter /*= Tex::MagFilter::Linear*/)
{
   auto location = textureMap.find(path);
   if (location != textureMap.end())
   {
      SPtr<Texture> texture = location->second.lock();
      if (texture)
      {
         return texture;
      }
   }

   // Load images bottom-to-top (since that is how OpenGL expects textures)
   stbi_set_flip_vertically_on_load(true);

   ImageInfo info = loadImage(path);

   SPtr<Texture> texture = createTexture(info);
   setParameters(*texture, wrap, minFilter, magFilter);

   textureMap.emplace(path, WPtr<Texture>(texture));
   return texture;
}

SPtr<Texture> TextureLoader::loadCubemap(const std::array<std::string, 6>& paths,
   Tex::Wrap wrap /*= Tex::Wrap::ClampToEdge*/, Tex::MinFilter minFilter /*= Tex::MinFilter::LinearMipmapLinear*/,
   Tex::MagFilter magFilter /*= Tex::MagFilter::Linear*/)
{
   auto location = cubemapMap.find(paths[0]);
   if (location != cubemapMap.end())
   {
      SPtr<Texture> cubemap = location->second.lock();
      if (cubemap)
      {
         return cubemap;
      }
   }

   // Load images top-to-bottom because cubemaps are weird
   stbi_set_flip_vertically_on_load(false);

   std::array<ImageInfo, 6> infos;
   for (int i = 0; i < infos.size(); ++i)
   {
      infos[i] = loadImage(paths[i]);
   }

   bool allMatch = true;
   for (int i = 1; i < infos.size(); ++i)
   {
      if (!infoMatches(infos[0], infos[i]))
      {
         allMatch = false;
         break;
      }
   }
   if (!allMatch)
   {
      LOG_WARNING("Not all cubemap faces share image resolution, composition, or format, reverting to default");
      for (ImageInfo& info : infos)
      {
         info = getDefaultImageInfo();
      }
   }

   SPtr<Texture> cubemap = createCubemap(infos);
   setParameters(*cubemap, wrap, minFilter, magFilter);

   cubemapMap.emplace(paths[0], WPtr<Texture>(cubemap));
   return cubemap;
}
