#include "Graphics/Material.h"

#include "Core/Assert.h"
#include "Graphics/MaterialParameter.h"
#include "Graphics/ShaderProgram.h"

#include <glm/glm.hpp>

namespace
{
   UPtr<MaterialParameterBase> createParameter(const std::string& name, UniformType type)
   {
      switch (type)
      {
#define UNIFORM_TYPE_CASE(uniform_type, data_type, param_type) case UniformType::uniform_type: return std::make_unique<uniform_type##MaterialParameter>(name);

#define FOR_EACH_UNIFORM_TYPE UNIFORM_TYPE_CASE
#include "ForEachUniformType.inl"
#undef FOR_EACH_UNIFORM_TYPE

#undef UNIFORM_TYPE_CASE
      default:
         ASSERT(false, "Invalid uniform type: %u", static_cast<unsigned int>(type));
         return nullptr;
      }
   }
}

Material::Material(const SPtr<ShaderProgram>& program)
   : shaderProgram(program)
   , textureUnitCounter(0)
{
   ASSERT(shaderProgram);

   generateParameters();
#if SWAP_DEBUG
   bindOnLinkDelegate();
#endif // SWAP_DEBUG
}

Material::Material(Material&& other)
{
   move(std::move(other));
}

Material::~Material()
{
   release();
}

Material& Material::operator=(Material&& other)
{
   release();
   move(std::move(other));
   return *this;
}

void Material::move(Material&& other)
{
   other.release();

   parameters = std::move(other.parameters);
   shaderProgram = std::move(other.shaderProgram);
   textureUnitCounter = other.textureUnitCounter;
   other.textureUnitCounter = 0;

#if SWAP_DEBUG
   bindOnLinkDelegate();
#endif // SWAP_DEBUG
}

void Material::release()
{
#if SWAP_DEBUG
   if (onLinkDelegateHandle.isValid())
   {
      ASSERT(shaderProgram);

      shaderProgram->removeOnLinkDelegate(onLinkDelegateHandle);
      onLinkDelegateHandle.invalidate();
   }
#endif // SWAP_DEBUG
}

void Material::commit()
{
   ASSERT(shaderProgram);

   textureUnitCounter = 0;

   for (auto& pair : parameters)
   {
      pair.second->commit(*this);
   }

   shaderProgram->commit();
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

#if SWAP_DEBUG
void Material::generateParameters()
{
   ASSERT(shaderProgram);

   const ShaderProgram::UniformMap& uniforms = shaderProgram->getUniforms();

   // Clear parameters that no longer have an associated uniform
   for (auto it = parameters.begin(); it != parameters.end();)
   {
      if (uniforms.count(it->first) == 0)
      {
         it = parameters.erase(it);
      }
      else
      {
         ++it;
      }
   }

   // Add parameters for new uniforms
   for (const auto& pair : uniforms)
   {
      if (parameters.count(pair.first) == 0)
      {
         UPtr<MaterialParameterBase> parameter = createParameter(pair.first, pair.second->getType());
         if (parameter)
         {
            parameters.emplace(pair.first, std::move(parameter));
         }
      }
   }
}
#else // SWAP_DEBUG
void Material::generateParameters()
{
   for (const auto& pair : shaderProgram->getUniforms())
   {
      UPtr<MaterialParameterBase> parameter = createParameter(pair.first, pair.second->getType());
      if (parameter)
      {
         parameters.emplace(pair.first, std::move(parameter));
      }
   }
}
#endif // SWAP_DEBUG

#if SWAP_DEBUG
void Material::bindOnLinkDelegate()
{
   ASSERT(shaderProgram);
   ASSERT(!onLinkDelegateHandle.isValid());

   onLinkDelegateHandle = shaderProgram->addOnLinkDelegate([this](ShaderProgram& program, bool linked)
   {
      ASSERT(&program == shaderProgram.get());
      generateParameters();
   });
}
#endif // SWAP_DEBUG
