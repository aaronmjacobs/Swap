#include "Scene/Component.h"

#include "Scene/Entity.h"

#include <utility>

Component::~Component()
{
   onDestroyDelegate.broadcast(this);
}

void Component::destroy()
{
   bool destroyed = entity.destroyComponent(this);
   ASSERT(destroyed, "Component not destroyed by entity, possibly already distroyed?");
}

DelegateHandle Component::addOnDestroyDelegate(OnDestroyDelegate::FuncType&& function)
{
   return onDestroyDelegate.add(std::move(function));
}

void Component::removeOnDestroyDelegate(const DelegateHandle& handle)
{
   onDestroyDelegate.remove(handle);
}

Scene& Component::getScene()
{
   return getEntity().getScene();
}

const Scene& Component::getScene() const
{
   return getEntity().getScene();
}

// static
ComponentRegistry& ComponentRegistry::instance()
{
   static ComponentRegistry singleton;
   return singleton;
}
