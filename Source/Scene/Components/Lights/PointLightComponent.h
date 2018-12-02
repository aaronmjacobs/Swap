#pragma once

#include "Scene/Components/Lights/LightComponent.h"

class PointLightComponent : public LightComponent
{
protected:
   friend class ComponentRegistrar<PointLightComponent>;

   PointLightComponent(Entity& owningEntity);

public:
   ~PointLightComponent();

   float getRadius() const
   {
      return radius;
   }

   void setRadius(float newRadius);

private:
   float radius;
};

SWAP_REFERENCE_COMPONENT(PointLightComponent)
