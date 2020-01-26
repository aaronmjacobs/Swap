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
      , shadowBias(0.0001f)
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

   float getShadowBias() const
   {
      return shadowBias;
   }

   void setShadowBias(float newShadowBias)
   {
      shadowBias = newShadowBias;
   }

private:
   glm::vec3 color;
   bool castShadows;
   float shadowBias;
};
