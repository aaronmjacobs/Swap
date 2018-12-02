#include "Scene/Components/SceneComponent.h"

#include "Core/Assert.h"

SWAP_REGISTER_COMPONENT(SceneComponent)

Transform SceneComponent::getAbsoluteTransform() const
{
   if (!parent)
   {
      return relativeTransform;
   }

   return relativeTransform * parent->getAbsoluteTransform();
}

void SceneComponent::setAbsoluteTransform(const Transform& newAbsoluteTransform)
{
   if (!parent)
   {
      relativeTransform = newAbsoluteTransform;
   }
   else
   {
      relativeTransform = newAbsoluteTransform * parent->getAbsoluteTransform().inverse(); // TODO Correct?
   }
}

void SceneComponent::setParent(SceneComponent* newParent)
{
   if (newParent && &getEntity() != &newParent->getEntity())
   {
      return;
   }

   if (parentDestroyDelegateHandle.isValid())
   {
      ASSERT(parent);
      parent->removeOnDestroyDelegate(parentDestroyDelegateHandle);
      parentDestroyDelegateHandle.invalidate();
   }

   if (newParent)
   {
      parentDestroyDelegateHandle = newParent->addOnDestroyDelegate([this](Component* component)
      {
         // Invalidate the handle here to prevent calling removeOnDestroyDelegate() from within setParent()
         // (Would cause concurrent modification of the multicast delegate)
         // Safe to do as we know the destroy event will only happen once
         parentDestroyDelegateHandle.invalidate();

         SceneComponent* sceneComponent = static_cast<SceneComponent*>(component);
         setParent(sceneComponent->getParent());
      });
   }

   parent = newParent;
}
