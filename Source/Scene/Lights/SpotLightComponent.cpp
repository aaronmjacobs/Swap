#include "Scene/Lights/SpotLightComponent.h"

#include "Core/Assert.h"
#include "Scene/Scene.h"

#include <algorithm>

SWAP_REGISTER_COMPONENT(SpotLightComponent)

SpotLightComponent::SpotLightComponent(Entity& owningEntity)
   : LightComponent(owningEntity)
   , radius(10.0f)
   , beamAngle(30.0f)
   , cutoffAngle(45.0f)
{
   getScene().registerSpotLightComponent(this);
}

SpotLightComponent::~SpotLightComponent()
{
   getScene().unregisterSpotLightComponent(this);
}

void SpotLightComponent::setRadius(float newRadius)
{
   ASSERT(newRadius >= 0.0f);

   radius = std::max(newRadius, 0.0f);
}

void SpotLightComponent::setBeamAngle(float newBeamAngle)
{
   ASSERT(newBeamAngle >= 0.0f);
   ASSERT(newBeamAngle <= cutoffAngle);

   beamAngle = std::max(std::min(newBeamAngle, cutoffAngle), 0.0f);
}

void SpotLightComponent::setGetCutoffAngle(float newCutoffAngle)
{
   ASSERT(newCutoffAngle >= 0.0f);
   ASSERT(newCutoffAngle >= beamAngle);

   cutoffAngle = std::max(std::max(newCutoffAngle, beamAngle), 0.0f);
}
