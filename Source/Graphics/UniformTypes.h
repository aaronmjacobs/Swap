#pragma once

#include "Core/Assert.h"
#include "Graphics/Uniform.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace
{
   bool typesMatch(const GLenum uniformType)
   {
      return false;
   }

   template<typename First, typename... Rest>
   bool typesMatch(const GLenum uniformType, First first, Rest... rest)
   {
      return uniformType == first || typesMatch(uniformType, rest...);
   }

   template<typename... Args>
   void checkTypeEnum(const GLenum uniformType, Args... args)
   {
      ASSERT(typesMatch(uniformType, args...), "Invalid uniform type: %u", uniformType);
   }
}

#define DECLARE_UNIFORM_TYPE(uniform_type, data_type, param_type, get_function, set_function, ...)\
class uniform_type##Uniform : public Uniform\
{\
public:\
   uniform_type##Uniform(const std::string& uniformName, const GLint uniformLocation, const GLenum uniformType, const GLuint program)\
      : Uniform(uniformName, uniformLocation, uniformType, program)\
      , data{}\
   {\
      checkTypeEnum(uniformType, __VA_ARGS__);\
      get_function;\
   }\
\
   UniformType getType() const override\
   {\
      return UniformType::uniform_type;\
   }\
\
protected:\
   void commitData() override\
   {\
      set_function;\
   }\
\
   bool setData(param_type value) override\
   {\
      bool valueChanged = data != value;\
      data = value;\
      return valueChanged;\
   }\
\
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
   data_type data;\
};

#define DECLARE_UNIFORM_TYPE_BASIC(uniform_type, data_type, get_function_name, set_function_name, ...)\
        DECLARE_UNIFORM_TYPE(uniform_type, data_type, data_type, get_function_name(program, uniformLocation, &data), set_function_name(getLocation(), data), __VA_ARGS__)

#define DECLARE_UNIFORM_TYPE_VECTOR(uniform_type, data_type, get_function_name, set_function_name, ...)\
        DECLARE_UNIFORM_TYPE(uniform_type, data_type, const data_type&, get_function_name(program, uniformLocation, glm::value_ptr(data)), set_function_name(getLocation(), 1, glm::value_ptr(data)), __VA_ARGS__)

#define DECLARE_UNIFORM_TYPE_MATRIX(uniform_type, data_type, get_function_name, set_function_name, ...)\
        DECLARE_UNIFORM_TYPE(uniform_type, data_type, const data_type&, get_function_name(program, uniformLocation, glm::value_ptr(data)), set_function_name(getLocation(), 1, GL_FALSE, glm::value_ptr(data)), __VA_ARGS__)

DECLARE_UNIFORM_TYPE_BASIC(Float, GLfloat, glGetUniformfv, glUniform1f, GL_FLOAT)
DECLARE_UNIFORM_TYPE_BASIC(Int, GLint, glGetUniformiv, glUniform1i, GL_INT)
DECLARE_UNIFORM_TYPE_BASIC(Uint, GLuint, glGetUniformuiv, glUniform1ui, GL_UNSIGNED_INT)
DECLARE_UNIFORM_TYPE_BASIC(Bool, GLint, glGetUniformiv, glUniform1i, GL_BOOL)

DECLARE_UNIFORM_TYPE_VECTOR(Float2, glm::fvec2, glGetUniformfv, glUniform2fv, GL_FLOAT_VEC2)
DECLARE_UNIFORM_TYPE_VECTOR(Float3, glm::fvec3, glGetUniformfv, glUniform3fv, GL_FLOAT_VEC3)
DECLARE_UNIFORM_TYPE_VECTOR(Float4, glm::fvec4, glGetUniformfv, glUniform4fv, GL_FLOAT_VEC4)

DECLARE_UNIFORM_TYPE_VECTOR(Int2, glm::ivec2, glGetUniformiv, glUniform2iv, GL_INT_VEC2)
DECLARE_UNIFORM_TYPE_VECTOR(Int3, glm::ivec3, glGetUniformiv, glUniform3iv, GL_INT_VEC3)
DECLARE_UNIFORM_TYPE_VECTOR(Int4, glm::ivec4, glGetUniformiv, glUniform4iv, GL_INT_VEC4)

DECLARE_UNIFORM_TYPE_VECTOR(Uint2, glm::uvec2, glGetUniformuiv, glUniform2uiv, GL_UNSIGNED_INT_VEC2)
DECLARE_UNIFORM_TYPE_VECTOR(Uint3, glm::uvec3, glGetUniformuiv, glUniform3uiv, GL_UNSIGNED_INT_VEC3)
DECLARE_UNIFORM_TYPE_VECTOR(Uint4, glm::uvec4, glGetUniformuiv, glUniform4uiv, GL_UNSIGNED_INT_VEC4)

DECLARE_UNIFORM_TYPE_VECTOR(Bool2, glm::ivec2, glGetUniformiv, glUniform2iv, GL_BOOL_VEC2)
DECLARE_UNIFORM_TYPE_VECTOR(Bool3, glm::ivec3, glGetUniformiv, glUniform3iv, GL_BOOL_VEC3)
DECLARE_UNIFORM_TYPE_VECTOR(Bool4, glm::ivec4, glGetUniformiv, glUniform4iv, GL_BOOL_VEC4)

DECLARE_UNIFORM_TYPE_MATRIX(Float2x2, glm::fmat2x2, glGetUniformfv, glUniformMatrix2fv, GL_FLOAT_MAT2)
DECLARE_UNIFORM_TYPE_MATRIX(Float2x3, glm::fmat2x3, glGetUniformfv, glUniformMatrix2x3fv, GL_FLOAT_MAT2x3)
DECLARE_UNIFORM_TYPE_MATRIX(Float2x4, glm::fmat2x4, glGetUniformfv, glUniformMatrix2x4fv, GL_FLOAT_MAT2x4)
DECLARE_UNIFORM_TYPE_MATRIX(Float3x2, glm::fmat3x2, glGetUniformfv, glUniformMatrix3x2fv, GL_FLOAT_MAT3x2)
DECLARE_UNIFORM_TYPE_MATRIX(Float3x3, glm::fmat3x3, glGetUniformfv, glUniformMatrix3fv, GL_FLOAT_MAT3)
DECLARE_UNIFORM_TYPE_MATRIX(Float3x4, glm::fmat3x4, glGetUniformfv, glUniformMatrix3x4fv, GL_FLOAT_MAT3x4)
DECLARE_UNIFORM_TYPE_MATRIX(Float4x2, glm::fmat4x2, glGetUniformfv, glUniformMatrix4x2fv, GL_FLOAT_MAT4x2)
DECLARE_UNIFORM_TYPE_MATRIX(Float4x3, glm::fmat4x3, glGetUniformfv, glUniformMatrix4x3fv, GL_FLOAT_MAT4x3)
DECLARE_UNIFORM_TYPE_MATRIX(Float4x4, glm::fmat4x4, glGetUniformfv, glUniformMatrix4fv, GL_FLOAT_MAT4)

#define VALID_SAMPLER_ENUMS \
GL_SAMPLER_1D, GL_SAMPLER_2D, GL_SAMPLER_3D, GL_SAMPLER_CUBE, GL_SAMPLER_1D_SHADOW, GL_SAMPLER_2D_SHADOW,\
GL_SAMPLER_1D_ARRAY, GL_SAMPLER_2D_ARRAY, GL_SAMPLER_1D_ARRAY_SHADOW, GL_SAMPLER_2D_ARRAY_SHADOW,\
GL_SAMPLER_2D_MULTISAMPLE, GL_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_SAMPLER_CUBE_SHADOW, GL_SAMPLER_BUFFER,\
GL_SAMPLER_2D_RECT, GL_SAMPLER_2D_RECT_SHADOW, GL_INT_SAMPLER_1D, GL_INT_SAMPLER_2D, GL_INT_SAMPLER_3D,\
GL_INT_SAMPLER_CUBE, GL_INT_SAMPLER_1D_ARRAY, GL_INT_SAMPLER_2D_ARRAY, GL_INT_SAMPLER_2D_MULTISAMPLE,\
GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_INT_SAMPLER_BUFFER, GL_INT_SAMPLER_2D_RECT, GL_UNSIGNED_INT_SAMPLER_1D,\
GL_UNSIGNED_INT_SAMPLER_2D, GL_UNSIGNED_INT_SAMPLER_3D, GL_UNSIGNED_INT_SAMPLER_CUBE, GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,\
GL_UNSIGNED_INT_SAMPLER_2D_ARRAY, GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,\
GL_UNSIGNED_INT_SAMPLER_BUFFER, GL_UNSIGNED_INT_SAMPLER_2D_RECT

DECLARE_UNIFORM_TYPE_BASIC(Texture, GLint, glGetUniformiv, glUniform1i, VALID_SAMPLER_ENUMS)

#undef VALID_SAMPLER_ENUMS

#undef DECLARE_UNIFORM_TYPE_MATRIX
#undef DECLARE_UNIFORM_TYPE_VECTOR
#undef DECLARE_UNIFORM_TYPE_BASIC
#undef DECLARE_UNIFORM_TYPE
