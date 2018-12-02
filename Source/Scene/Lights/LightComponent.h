#pragma once

#include "Scene/SceneComponent.h"

#include <glm/glm.hpp>

class LightComponent : public SceneComponent
{
protected:
   LightComponent(Entity& owningEntity)
      : SceneComponent(owningEntity)
   {
   }

public:
   const glm::vec3& getColor() const
   {
      return color;
   }

   void setColor(const glm::vec3& newColor)
   {
      color = newColor;
   }

private:
   glm::vec3 color;
};
