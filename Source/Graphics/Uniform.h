#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class Uniform
{
public:
   Uniform(const std::string& uniformName, const GLint uniformLocation, const GLenum uniformType, const GLuint program);
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

protected:
   GLint getLocation() const
   {
      return location;
   }

   virtual void commitData() = 0;

#define DECLARE_SET_DATA(type) virtual bool setData(type value) { return typeError(#type); }
#define DECLARE_SET_DATA_REF(type) virtual bool setData(const type& value) { return typeError(#type); }

   DECLARE_SET_DATA(GLfloat)
   DECLARE_SET_DATA(GLint)
   DECLARE_SET_DATA(GLuint)

   DECLARE_SET_DATA_REF(glm::fvec2)
   DECLARE_SET_DATA_REF(glm::fvec3)
   DECLARE_SET_DATA_REF(glm::fvec4)

   DECLARE_SET_DATA_REF(glm::ivec2)
   DECLARE_SET_DATA_REF(glm::ivec3)
   DECLARE_SET_DATA_REF(glm::ivec4)

   DECLARE_SET_DATA_REF(glm::uvec2)
   DECLARE_SET_DATA_REF(glm::uvec3)
   DECLARE_SET_DATA_REF(glm::uvec4)

   DECLARE_SET_DATA_REF(glm::fmat2x2)
   DECLARE_SET_DATA_REF(glm::fmat2x3)
   DECLARE_SET_DATA_REF(glm::fmat2x4)
   DECLARE_SET_DATA_REF(glm::fmat3x2)
   DECLARE_SET_DATA_REF(glm::fmat3x3)
   DECLARE_SET_DATA_REF(glm::fmat3x4)
   DECLARE_SET_DATA_REF(glm::fmat4x2)
   DECLARE_SET_DATA_REF(glm::fmat4x3)
   DECLARE_SET_DATA_REF(glm::fmat4x4)

#undef DECLARE_SET_DATA
#undef DECLARE_SET_DATA_REF

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

   virtual const char* getTypeName() const = 0;

private:
   bool typeError(const char* typeName);

   const std::string name;
   const GLint location;
   const GLenum type;
   bool dirty;
};