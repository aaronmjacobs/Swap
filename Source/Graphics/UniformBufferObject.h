#pragma once

#include "Core/Assert.h"
#include "Graphics/BufferObject.h"
#include "Graphics/UniformBufferObjectHelpers.h"

#include <glad/gl.h>

#include <cstdint>
#include <string>
#include <tuple>
#include <vector>

enum class UniformBufferObjectIndex : GLuint
{
   Framebuffer,
   View,

   Invalid = GL_INVALID_INDEX
};

class UniformBufferObject : public BufferObject
{
public:
   UniformBufferObject(std::string name);
   UniformBufferObject(const UniformBufferObject& other) = delete;
   UniformBufferObject(UniformBufferObject&& other);
   ~UniformBufferObject() = default;
   UniformBufferObject& operator=(const UniformBufferObject& other) = delete;
   UniformBufferObject& operator=(UniformBufferObject&& other);

protected:
   void move(UniformBufferObject&& other);

public:
   template<typename... Types>
   void setData(const std::tuple<Types...>& data);

   template<typename... Types>
   void updateData(const std::tuple<Types...>& data);

   void bindTo(UniformBufferObjectIndex index);

   const std::string& getBlockName() const
   {
      return blockName;
   }

   UniformBufferObjectIndex getBoundIndex() const
   {
      return boundIndex;
   }

private:
   template<typename... Types>
   std::vector<uint8_t> generateBuffer(const std::tuple<Types...>& data);

   std::string blockName;
   UniformBufferObjectIndex boundIndex;
};

template<typename... Types>
inline void UniformBufferObject::setData(const std::tuple<Types...>& data)
{
   std::vector<uint8_t> buffer = generateBuffer(data);
   BufferObject::setData(BufferBindingTarget::Uniform, buffer.size(), buffer.data(), BufferUsage::DynamicDraw);
}

template<typename... Types>
inline void UniformBufferObject::updateData(const std::tuple<Types...>& data)
{
   std::vector<uint8_t> buffer = generateBuffer(data);
   BufferObject::updateData(BufferBindingTarget::Uniform, 0, buffer.size(), buffer.data());
}

template<typename... Types>
std::vector<uint8_t> UniformBufferObject::generateBuffer(const std::tuple<Types...>& data)
{
   constexpr std::size_t bufferSize = UniformBufferObjectHelpers::getPaddedSize<Types...>();
   constexpr std::size_t tupleSize = sizeof(std::tuple<Types...>);
   static_assert(bufferSize >= tupleSize, "Buffer size is too small!");

   std::vector<uint8_t> buffer(bufferSize);
   std::size_t offset = 0;

   UniformBufferObjectHelpers::copyData(data, buffer.data(), offset);
   ASSERT(offset == bufferSize);

   return buffer;
}
