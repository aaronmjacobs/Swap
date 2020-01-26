#pragma once

#include "Scene/Components/Lights/LightComponent.h"

#include "Math/Bounds.h"

class DirectionalLightComponent : public LightComponent
{
protected:
   friend class ComponentRegistrar<DirectionalLightComponent>;

   DirectionalLightComponent(Entity& owningEntity);

public:
   ~DirectionalLightComponent();

   const Bounds& getShadowClipBounds() const
   {
      return shadowClipBounds;
   }

   void setShadowClipBounds(const Bounds& newShadowClipBounds)
   {
      shadowClipBounds = newShadowClipBounds;
   }

private:
   Bounds shadowClipBounds;
};

SWAP_REFERENCE_COMPONENT(DirectionalLightComponent)
