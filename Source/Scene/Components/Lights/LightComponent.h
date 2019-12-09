#pragma once

#include "Scene/Components/SceneComponent.h"

#include <glm/glm.hpp>

class LightComponent : public SceneComponent
{
protected:
   LightComponent(Entity& owningEntity)
      : SceneComponent(owningEntity)
      , color(glm::vec3(1.0f))
      , castShadows(true)
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

   bool getCastShadows() const
   {
      return castShadows;
   }

   void setCastShadows(bool newCastShadows)
   {
      castShadows = newCastShadows;
   }

private:
   glm::vec3 color;
   bool castShadows;
};
