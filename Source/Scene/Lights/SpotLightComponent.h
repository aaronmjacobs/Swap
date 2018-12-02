#pragma once

#include "Scene/Lights/LightComponent.h"

class SpotLightComponent : public LightComponent
{
protected:
   friend class ComponentRegistrar<SpotLightComponent>;

   SpotLightComponent(Entity& owningEntity);

public:
   ~SpotLightComponent();

   float getRadius() const
   {
      return radius;
   }

   void setRadius(float newRadius);

   float getBeamAngle() const
   {
      return beamAngle;
   }

   void setBeamAngle(float newBeamAngle);

   float getCutoffAngle() const
   {
      return cutoffAngle;
   }

   void setGetCutoffAngle(float newCutoffAngle);

private:
   float radius;
   float beamAngle;
   float cutoffAngle;
};

SWAP_REFERENCE_COMPONENT(SpotLightComponent)
