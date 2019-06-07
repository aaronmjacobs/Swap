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
   void apply(DrawingContext& context);

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

private:
   using ParameterMap = std::unordered_map<std::string, UPtr<MaterialParameterBase>>;

   ParameterMap::iterator findOrCreateParameter(const std::string& name, UniformType type);

   ParameterMap parameters;
   std::array<bool, 3> commonMaterialParameterUsage = {};
};
