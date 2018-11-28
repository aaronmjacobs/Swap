#pragma once

#include "Scene/Component.h"

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
   const Transform& getRelativeTransform() const
   {
      return relativeTransform;
   }

   void setRelativeTransform(const Transform& newRelativeTransform)
   {
      relativeTransform = newRelativeTransform;
   }

   Transform getAbsoluteTransform() const;
   void setAbsoluteTransform(const Transform& newAbsoluteTransform);

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
