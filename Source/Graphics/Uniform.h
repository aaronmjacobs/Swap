#pragma once

#include "Core/Pointers.h"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <string>

class Texture;

enum class UniformType : uint8_t
{
   Invalid,

   Float,
   Int,
   Uint,
   Bool,

   Float2,
   Float3,
   Float4,

   Int2,
   Int3,
   Int4,

   Uint2,
   Uint3,
   Uint4,

   Bool2,
   Bool3,
   Bool4,

   Float2x2,
   Float2x3,
   Float2x4,
   Float3x2,
   Float3x3,
   Float3x4,
   Float4x2,
   Float4x3,
   Float4x4,

   Texture
};

template<typename T>
constexpr inline UniformType getUniformType();

#define UNIFORM_TYPE_SPECIALIZATION(uniform_type, data_type, param_type) template<> constexpr inline UniformType getUniformType<data_type>() { return UniformType::uniform_type; }

#define FOR_EACH_UNIFORM_TYPE UNIFORM_TYPE_SPECIALIZATION
#include "ForEachUniformType.inl"
#undef FOR_EACH_UNIFORM_TYPE

#undef UNIFORM_TYPE_SPECIALIZATION

class Uniform
{
public:
   Uniform(const std::string& uniformName, const GLint uniformLocation, const GLuint program);
   Uniform(const Uniform& other) = delete;
   Uniform(Uniform&& other) = delete;
   virtual ~Uniform() = default;
   Uniform& operator=(const Uniform& other) = delete;
   Uniform& operator=(Uniform&& other) = delete;

   void commit();

   template<typename T>
   void setValue(const T& value)
   {
      bool valueChanged = setData(value);
      dirty = dirty || valueChanged;
   }

   virtual UniformType getType() const = 0;

protected:
   GLint getLocation() const
   {
      return location;
   }

   virtual void commitData() = 0;

#define DECLARE_SET_DATA(uniform_type, data_type, param_type) virtual bool setData(param_type value) { return typeError(#data_type); }

#define FOR_EACH_UNIFORM_TYPE DECLARE_SET_DATA
#define FOR_EACH_UNIFORM_TYPE_NO_BOOL
#define FOR_EACH_UNIFORM_TYPE_NO_COMPLEX
#include "ForEachUniformType.inl"
#undef FOR_EACH_UNIFORM_TYPE_NO_COMPLEX
#undef FOR_EACH_UNIFORM_TYPE_NO_BOOL
#undef FOR_EACH_UNIFORM_TYPE

#undef DECLARE_SET_DATA

   // OpenGL treats booleans as integers when setting / getting uniform values, so we provide wrapper functions
   bool setData(bool value)
   {
      return setData(static_cast<GLint>(value));
   }
   bool setData(const glm::bvec2& value)
   {
      return setData(glm::ivec2(value.x, value.y));
   }
   bool setData(const glm::bvec3& value)
   {
      return setData(glm::ivec3(value.x, value.y, value.z));
   }
   bool setData(const glm::bvec4& value)
   {
      return setData(glm::ivec4(value.x, value.y, value.z, value.w));
   }

   virtual const char* getUniformTypeName() const = 0;
   virtual const char* getDataTypeName() const = 0;

private:
   bool typeError(const char* dataTypeName);

   const std::string name;
   const GLint location;
   bool dirty;
};
