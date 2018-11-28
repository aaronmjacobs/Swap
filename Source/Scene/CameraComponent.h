#pragma once

#include "Scene/SceneComponent.h"

class CameraComponent : public SceneComponent
{
protected:
   friend class ComponentRegistrar<CameraComponent>;

   CameraComponent(Entity& owningEntity);

public:
   ~CameraComponent();

   void makeActiveCamera();

   void moveForward(float amount);
   void moveRight(float amount);
   void moveUp(float amount);

   void rotate(float yaw, float pitch);

   float getFieldOfView() const
   {
      return fieldOfView;
   }

   void setFieldOfView(float newFieldOfView)
   {
      fieldOfView = newFieldOfView;
   }

protected:
   glm::vec3 getForward() const;
   glm::vec3 getRight() const;
   glm::vec3 getUp() const;

private:
   float fieldOfView;
};

SWAP_REFERENCE_COMPONENT(CameraComponent)
