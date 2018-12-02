#include "Scene/Lights/DirectionalLightComponent.h"

#include "Scene/Scene.h"

SWAP_REGISTER_COMPONENT(DirectionalLightComponent)

DirectionalLightComponent::DirectionalLightComponent(Entity& owningEntity)
   : LightComponent(owningEntity)
{
   getScene().registerDirectionalLightComponent(this);
}

DirectionalLightComponent::~DirectionalLightComponent()
{
   getScene().unregisterDirectionalLightComponent(this);
}
