#pragma once

#include "Core/Assert.h"
#include "Core/Pointers.h"

#include <glm/glm.hpp>

#include <string>

class Material;
class Texture;
struct DrawingContext;

class MaterialParameterBase
{
public:
   MaterialParameterBase(const std::string& paramName)
      : name(paramName)
      , enabled(true)
   {
   }

   virtual ~MaterialParameterBase() = default;

   virtual UPtr<MaterialParameterBase> clone() const = 0;

   bool isEnabled() const
   {
      return enabled;
   }

   void setEnabled(bool newEnabled)
   {
      enabled = newEnabled;
   }

   virtual void apply(DrawingContext& context) const = 0;

#define DECLARE_SET_VALUE(uniform_type, data_type, param_type)\
   virtual bool setValue(param_type newValue) { return typeError(#data_type); }

#define FOR_EACH_UNIFORM_TYPE DECLARE_SET_VALUE
#include "ForEachUniformType.inl"
#undef FOR_EACH_UNIFORM_TYPE

#undef DECLARE_SET_VALUE

protected:
   virtual const char* getUniformTypeName() const = 0;
   virtual const char* getDataTypeName() const = 0;

   const std::string name;
   bool enabled;

private:
   bool typeError(const char* dataTypeName);
};

#define DECLARE_MATERIAL_PARAM_TYPE(uniform_type, data_type, param_type)\
class uniform_type##MaterialParameter : public MaterialParameterBase\
{\
public:\
   uniform_type##MaterialParameter(const std::string& paramName)\
      : MaterialParameterBase(paramName)\
   {\
   }\
\
   UPtr<MaterialParameterBase> clone() const override\
   {\
      return std::make_unique<uniform_type##MaterialParameter>(*this);\
   }\
\
   void apply(DrawingContext& context) const override;\
\
   bool setValue(param_type newValue) override\
   {\
      value = newValue;\
      return true;\
   }\
\
protected:\
   const char* getUniformTypeName() const override\
   {\
      return #uniform_type;\
   }\
\
   const char* getDataTypeName() const override\
   {\
      return #data_type;\
   }\
\
private:\
   data_type value;\
};

#define FOR_EACH_UNIFORM_TYPE DECLARE_MATERIAL_PARAM_TYPE
#include "ForEachUniformType.inl"
#undef FOR_EACH_UNIFORM_TYPE

#undef DECLARE_MATERIAL_PARAM_TYPE
