#include "Graphics/Material.h"

#include "Core/Assert.h"
#include "Graphics/MaterialParameter.h"
#include "Graphics/ShaderProgram.h"

#include <glm/glm.hpp>

namespace
{
   const std::array<CommonMaterialParameter, 3> kCommonMaterialParameters =
   {
      {
         CommonMaterialParameter::DiffuseTexture,
         CommonMaterialParameter::SpecularTexture,
         CommonMaterialParameter::NormalTexture
      }
   };
}

namespace CommonMaterialParameterNames
{
   const std::string& get(CommonMaterialParameter parameter)
   {
      static const std::array<std::string, 3> kCommonMaterialParameterNames =
      {
         {
            "uMaterial.diffuseTexture",
            "uMaterial.specularTexture",
            "uMaterial.normalTexture"
         }
      };

      return kCommonMaterialParameterNames[static_cast<uint8_t>(parameter)];
   }
}

Material::Material(const Material& other)
{
   *this = other;
}

Material& Material::operator=(const Material& other)
{
   parameters.reserve(other.parameters.size());
   for (const auto& pair : other.parameters)
   {
      parameters.emplace(pair.first, pair.second->clone());
   }

   commonMaterialParameterUsage = other.commonMaterialParameterUsage;
   blendMode = other.blendMode;

   return *this;
}

void Material::apply(DrawingContext& context) const
{
   for (auto& pair : parameters)
   {
      pair.second->apply(context);
   }
}

bool Material::isParameterEnabled(const std::string& name) const
{
   auto location = parameters.find(name);
   if (location != parameters.end())
   {
      return location->second->isEnabled();
   }
   else
   {
      ASSERT(false, "Material parameter with given name doesn't exist: %s", name.c_str());
      return false;
   }
}

void Material::setParameterEnabled(const std::string& name, bool enabled)
{
   auto location = parameters.find(name);
   if (location != parameters.end())
   {
      location->second->setEnabled(enabled);
   }
   else
   {
      ASSERT(false, "Material parameter with given name doesn't exist: %s", name.c_str());
   }
}

Material::ParameterMap::iterator Material::findOrCreateParameter(const std::string& name, UniformType type)
{
   ParameterMap::iterator location = parameters.find(name);
   if (location != parameters.end())
   {
      return location;
   }

   UPtr<MaterialParameterBase> parameter;
   switch (type)
   {
#define UNIFORM_TYPE_CASE(uniform_type, data_type, param_type) case UniformType::uniform_type: parameter = std::make_unique<uniform_type##MaterialParameter>(name); break;

#define FOR_EACH_UNIFORM_TYPE UNIFORM_TYPE_CASE
#include "ForEachUniformType.inl"
#undef FOR_EACH_UNIFORM_TYPE

#undef UNIFORM_TYPE_CASE
   default:
      ASSERT(false, "Invalid uniform type: %u", static_cast<unsigned int>(type));
      break;
   }

   if (parameter)
   {
      auto pair = parameters.emplace(name, std::move(parameter));
      location = pair.first;

      for (uint8_t i = 0; i < kCommonMaterialParameters.size(); ++i)
      {
         commonMaterialParameterUsage[i] = commonMaterialParameterUsage[i] || name == CommonMaterialParameterNames::get(kCommonMaterialParameters[i]);
      }
   }

   return location;
}
