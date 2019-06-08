#pragma once

#include <glm/glm.hpp>
#include <gsl/span>

struct Bounds
{
   static Bounds fromPoints(gsl::span<glm::vec3> points);

   glm::vec3 getMin() const
   {
      return center - extent;
   }

   glm::vec3 getMax() const
   {
      return center + extent;
   }

   glm::vec3 center;
   glm::vec3 extent;
   float radius = 0.0f;
};
