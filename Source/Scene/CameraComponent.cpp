#include "Scene/CameraComponent.h"

#include "Math/MathUtils.h"

#include "Scene/Entity.h"
#include "Scene/Scene.h"

SWAP_REGISTER_COMPONENT(CameraComponent)

CameraComponent::CameraComponent(Entity& owningEntity)
   : SceneComponent(owningEntity)
   , fieldOfView(90.0f)
{
   getScene().registerCameraComponent(this);
}

CameraComponent::~CameraComponent()
{
   getScene().unregisterCameraComponent(this);
}

void CameraComponent::makeActiveCamera()
{
   getScene().setActiveCameraComponent(this);
}

void CameraComponent::moveForward(float amount)
{
   relativeTransform.position += getForward() * amount;
}

void CameraComponent::moveRight(float amount)
{
   relativeTransform.position += getRight() * amount;
}

void CameraComponent::moveUp(float amount)
{
   relativeTransform.position += getUp() * amount;
}

void CameraComponent::rotate(float yaw, float pitch)
{
   glm::quat yawChange = glm::angleAxis(yaw, MathUtils::kUpVector);
   glm::quat pitchChange = glm::angleAxis(pitch, MathUtils::kRightVector);

   relativeTransform.orientation = glm::normalize(pitchChange * relativeTransform.orientation * yawChange);
}

glm::vec3 CameraComponent::getForward() const
{
   return MathUtils::kForwardVector * relativeTransform.orientation;
}

glm::vec3 CameraComponent::getRight() const
{
   return MathUtils::kRightVector * relativeTransform.orientation;
}

glm::vec3 CameraComponent::getUp() const
{
   return MathUtils::kUpVector * relativeTransform.orientation;
}
