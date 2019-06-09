#pragma once

#include "Core/Assert.h"
#if SWAP_DEBUG
#include "Core/Delegate.h"
#endif // SWAP_DEBUG
#include "Core/Pointers.h"
#include "Graphics/MaterialParameter.h"
#include "Graphics/Uniform.h"

#include <array>
#include <string>
#include <unordered_map>

struct DrawingContext;

enum class BlendMode : uint8_t
{
   Opaque,
   Translucent
};

enum class CommonMaterialParameter : uint8_t
{
   DiffuseTexture,
   SpecularTexture,
   NormalTexture
};

namespace CommonMaterialParameterNames
{
   const std::string& get(CommonMaterialParameter parameter);
}

class Material
{
public:
   Material() = default;
   Material(const Material& other);
   Material(Material&& other) = default;
   ~Material() = default;
   Material& operator=(const Material& other);
   Material& operator=(Material&& other) = default;

   void apply(DrawingContext& context) const;

   template<typename T>
   bool setParameter(const std::string& name, const T& value)
   {
      auto location = findOrCreateParameter(name, getUniformType<T>());
      ASSERT(location != parameters.end());

      return location->second->setValue(value);;
   }

   bool isParameterEnabled(const std::string& name) const;
   void setParameterEnabled(const std::string& name, bool enabled);

   bool hasParameter(const std::string& name) const
   {
      return parameters.count(name) > 0;
   }

   bool hasCommonParameter(CommonMaterialParameter parameter) const
   {
      return commonMaterialParameterUsage[static_cast<uint8_t>(parameter)];
   }

   BlendMode getBlendMode() const
   {
      return blendMode;
   }

   void setBlendMode(BlendMode newBlendMode)
   {
      blendMode = newBlendMode;
   }

private:
   using ParameterMap = std::unordered_map<std::string, UPtr<MaterialParameterBase>>;

   ParameterMap::iterator findOrCreateParameter(const std::string& name, UniformType type);

   ParameterMap parameters;
   std::array<bool, 3> commonMaterialParameterUsage = {};
   BlendMode blendMode = BlendMode::Opaque;
};
