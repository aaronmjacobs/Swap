#include "Scene/Components/ModelComponent.h"

#include "Scene/Scene.h"

SWAP_REGISTER_COMPONENT(ModelComponent)

ModelComponent::ModelComponent(Entity& owningEntity)
   : SceneComponent(owningEntity)
{
   getScene().registerModelComponent(this);
}

ModelComponent::~ModelComponent()
{
   getScene().unregisterModelComponent(this);
}
