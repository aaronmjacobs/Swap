#pragma once

#include "Scene/Components/Component.h"

#include "Math/Transform.h"

class SceneComponent : public Component
{
protected:
   friend class ComponentRegistrar<SceneComponent>;

   SceneComponent(Entity& owningEntity)
      : Component(owningEntity)
      , parent(nullptr)
   {
   }

public:
   Transform& getRelativeTransform()
   {
      return relativeTransform;
   }

   const Transform& getRelativeTransform() const
   {
      return relativeTransform;
   }

   void setRelativeTransform(const Transform& newRelativeTransform)
   {
      relativeTransform = newRelativeTransform;
   }

   glm::quat& getRelativeOrientation()
   {
      return relativeTransform.orientation;
   }

   const glm::quat& getRelativeOrientation() const
   {
      return relativeTransform.orientation;
   }

   void setRelativeOrientation(const glm::quat& newRelativeOrientation)
   {
      relativeTransform.orientation = newRelativeOrientation;
   }

   glm::vec3& getRelativePosition()
   {
      return relativeTransform.position;
   }

   const glm::vec3& getRelativePosition() const
   {
      return relativeTransform.position;
   }

   void setRelativePosition(const glm::vec3& newRelativePosition)
   {
      relativeTransform.position = newRelativePosition;
   }

   glm::vec3& getRelativeScale()
   {
      return relativeTransform.scale;
   }

   const glm::vec3& getRelativeScale() const
   {
      return relativeTransform.scale;
   }

   void setRelativeScale(const glm::vec3& newRelativeScale)
   {
      relativeTransform.scale = newRelativeScale;
   }

   Transform getAbsoluteTransform() const;
   void setAbsoluteTransform(const Transform& newAbsoluteTransform);

   glm::quat getAbsoluteOrientation() const
   {
      return getAbsoluteTransform().orientation;
   }

   void setAbsoluteOrientation(const glm::quat& newAbsoluteOrientation);

   glm::vec3 getAbsolutePosition() const
   {
      return getAbsoluteTransform().position;
   }

   void setAbsolutePosition(const glm::vec3& newAbsolutePosition);

   glm::vec3 getAbsoluteScale() const
   {
      return getAbsoluteTransform().scale;
   }

   void setAbsoluteScale(const glm::vec3& newAbsoluteScale);

   SceneComponent* getParent()
   {
      return parent;
   }

   const SceneComponent* getParent() const
   {
      return parent;
   }

   void setParent(SceneComponent* newParent);

protected:
   Transform relativeTransform;

private:
   SceneComponent* parent;
   DelegateHandle parentDestroyDelegateHandle;
};

SWAP_REFERENCE_COMPONENT(SceneComponent)
