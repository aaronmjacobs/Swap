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

void SceneComponent::setAbsoluteOrientation(const glm::quat& newAbsoluteOrientation)
{
   Transform absoluteTransform = getAbsoluteTransform();

   absoluteTransform.orientation = newAbsoluteOrientation;

   setAbsoluteTransform(absoluteTransform);
}

void SceneComponent::setAbsolutePosition(const glm::vec3& newAbsolutePosition)
{
   Transform absoluteTransform = getAbsoluteTransform();

   absoluteTransform.position = newAbsolutePosition;

   setAbsoluteTransform(absoluteTransform);
}

void SceneComponent::setAbsoluteScale(const glm::vec3& newAbsoluteScale)
{
   Transform absoluteTransform = getAbsoluteTransform();

   absoluteTransform.scale = newAbsoluteScale;

   setAbsoluteTransform(absoluteTransform);
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
