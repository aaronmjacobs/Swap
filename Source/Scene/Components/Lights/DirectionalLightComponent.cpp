#include "Scene/Components/Lights/DirectionalLightComponent.h"

#include "Scene/Scene.h"

SWAP_REGISTER_COMPONENT(DirectionalLightComponent)

DirectionalLightComponent::DirectionalLightComponent(Entity& owningEntity)
   : LightComponent(owningEntity)
{
   shadowClipBounds.extent = glm::vec3(100.0f);

   getScene().registerDirectionalLightComponent(this);
}

DirectionalLightComponent::~DirectionalLightComponent()
{
   getScene().unregisterDirectionalLightComponent(this);
}
