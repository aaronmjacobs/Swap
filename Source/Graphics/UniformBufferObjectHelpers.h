#pragma once

#include <glm/glm.hpp>

#include <cstdint>
#include <cstring>
#include <tuple>
#include <utility>

namespace UniformBufferObjectHelpers
{
   // Forward declarations

   template<typename First, typename... Rest>
   constexpr std::size_t getPaddedSizeHelper();

   template<typename Type>
   constexpr std::size_t getPaddedSizeSingle();

   template<typename Tuple, std::size_t... Is>
   inline void copyDataImpl(const Tuple& tuple, uint8_t* buffer, std::size_t& offset, std::index_sequence<Is...>);

   template<typename T>
   inline void copyDataSingle(const T& data, uint8_t* buffer, std::size_t& offset);

   // Base implementations

   template<typename... Types>
   constexpr std::size_t getPaddedSize()
   {
      return getPaddedSizeHelper<Types...>();
   }

   template<>
   constexpr std::size_t getPaddedSize()
   {
      return 0;
   }

   template<typename First, typename... Rest>
   constexpr std::size_t getPaddedSizeHelper()
   {
      return getPaddedSizeSingle<First>() + getPaddedSize<Rest...>();
   }

   template<typename Type>
   constexpr std::size_t getPaddedSizeSingle()
   {
      return sizeof(Type);
   }

   template<typename... Types>
   inline void copyData(const std::tuple<Types...>& data, uint8_t* buffer, std::size_t& offset)
   {
      copyDataImpl(data, buffer, offset, std::make_index_sequence<sizeof...(Types)>{});
   }

   template<typename Tuple, std::size_t... Is>
   inline void copyDataImpl(const Tuple& tuple, uint8_t* buffer, std::size_t& offset, std::index_sequence<Is...>)
   {
      (copyDataSingle(std::get<Is>(tuple), buffer, offset), ...);
   }

   template<typename T>
   inline void copyDataSingle(const T& data, uint8_t* buffer, std::size_t& offset)
   {
      std::memcpy(buffer + offset, &data, sizeof(T));
      offset += getPaddedSizeSingle<T>();
   }

   // Specializations

   // bool padded to 4 bytes

   template<>
   constexpr std::size_t getPaddedSizeSingle<bool>()
   {
      return 4;
   }

   // vec3 padded as vec4

   template<>
   constexpr std::size_t getPaddedSizeSingle<glm::fvec3>()
   {
      return getPaddedSizeSingle<glm::fvec4>();
   }

   template<>
   constexpr std::size_t getPaddedSizeSingle<glm::ivec3>()
   {
      return getPaddedSizeSingle<glm::ivec4>();
   }

   template<>
   constexpr std::size_t getPaddedSizeSingle<glm::uvec3>()
   {
      return getPaddedSizeSingle<glm::uvec4>();
   }

   // matrices stored as column vectors (vec4)

   template<>
   constexpr std::size_t getPaddedSizeSingle<glm::mat2>()
   {
      return getPaddedSizeSingle<glm::vec4>() * 2;
   }

   template<>
   inline void copyDataSingle(const glm::mat2& data, uint8_t* buffer, std::size_t& offset)
   {
      for (int i = 0; i < 2; ++i)
      {
         std::memcpy(buffer + offset, &data[i], sizeof(data[i]));
         offset += getPaddedSizeSingle<glm::vec4>();
      }
   }

   template<>
   constexpr std::size_t getPaddedSizeSingle<glm::mat3>()
   {
      return getPaddedSizeSingle<glm::vec4>() * 3;
   }

   template<>
   inline void copyDataSingle(const glm::mat3& data, uint8_t* buffer, std::size_t& offset)
   {
      for (int i = 0; i < 3; ++i)
      {
         std::memcpy(buffer + offset, &data[i], sizeof(data[i]));
         offset += getPaddedSizeSingle<glm::vec4>();
      }
   }

   template<>
   constexpr std::size_t getPaddedSizeSingle<glm::mat4>()
   {
      return getPaddedSizeSingle<glm::vec4>() * 4;
   }

   template<>
   inline void copyDataSingle(const glm::mat4& data, uint8_t* buffer, std::size_t& offset)
   {
      for (int i = 0; i < 4; ++i)
      {
         std::memcpy(buffer + offset, &data[i], sizeof(data[i]));
         offset += getPaddedSizeSingle<glm::vec4>();
      }
   }
}
