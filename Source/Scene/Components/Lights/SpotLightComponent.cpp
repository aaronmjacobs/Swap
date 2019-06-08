#include "Scene/Components/Lights/SpotLightComponent.h"

#include "Core/Assert.h"
#include "Scene/Scene.h"

#include <glm/glm.hpp>

#include <algorithm>

SWAP_REGISTER_COMPONENT(SpotLightComponent)

namespace
{
   const float kMaxAngle = 170.0f;
}

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

float SpotLightComponent::getScaledRadius() const
{
   Transform localToWorld = getAbsoluteTransform();

   return radius * localToWorld.scale.z;
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
   ASSERT(newBeamAngle <= kMaxAngle);

   beamAngle = glm::clamp(newBeamAngle, 0.0f, glm::min(cutoffAngle, kMaxAngle) - MathUtils::kKindaSmallNumber);
}

void SpotLightComponent::setCutoffAngle(float newCutoffAngle)
{
   ASSERT(newCutoffAngle >= 0.0f);
   ASSERT(newCutoffAngle >= beamAngle);
   ASSERT(newCutoffAngle <= kMaxAngle);

   cutoffAngle = glm::clamp(glm::max(newCutoffAngle, beamAngle + MathUtils::kKindaSmallNumber),
                            0.0f,
                            kMaxAngle - MathUtils::kKindaSmallNumber);
}
