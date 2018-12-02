#pragma once

#include "Scene/Lights/LightComponent.h"

class DirectionalLightComponent : public LightComponent
{
protected:
   friend class ComponentRegistrar<DirectionalLightComponent>;

   DirectionalLightComponent(Entity& owningEntity);

public:
   ~DirectionalLightComponent();
};

SWAP_REFERENCE_COMPONENT(DirectionalLightComponent)
