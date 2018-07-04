#pragma once

#include <gsl/span>

#include <utility>
#include <vector>

namespace Hash
{
   template<typename T>
   inline void combine(std::size_t& seed, const T& value)
   {
      std::hash<T> hasher;
      seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
   }
}

namespace std
{
   template<typename T>
   struct hash<gsl::span<T>>
   {
      std::size_t operator()(const gsl::span<T>& values) const
      {
         std::size_t seed = 0;

         for (const auto& value : values)
         {
            Hash::combine(seed, value);
         }

         return seed;
      }
   };

   template<typename T>
   struct hash<std::vector<T>>
   {
      std::size_t operator()(const std::vector<T>& values) const
      {
         return hash<gsl::span<const T>>()(values);
      }
   };
}
