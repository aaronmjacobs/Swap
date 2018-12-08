#pragma once

#include "Scene/Components/SceneComponent.h"

#include <glm/glm.hpp>

class LightComponent : public SceneComponent
{
protected:
   LightComponent(Entity& owningEntity)
      : SceneComponent(owningEntity)
      , color(glm::vec3(1.0f))
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
