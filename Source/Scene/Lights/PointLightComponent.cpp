#include "Scene/Lights/PointLightComponent.h"

#include "Core/Assert.h"
#include "Scene/Scene.h"

#include <algorithm>

SWAP_REGISTER_COMPONENT(PointLightComponent)

PointLightComponent::PointLightComponent(Entity& owningEntity)
   : LightComponent(owningEntity)
   , radius(10.0f)
{
   getScene().registerPointLightComponent(this);
}

PointLightComponent::~PointLightComponent()
{
   getScene().unregisterPointLightComponent(this);
}

void PointLightComponent::setRadius(float newRadius)
{
   ASSERT(newRadius >= 0.0f);

   radius = std::max(newRadius, 0.0f);
}
