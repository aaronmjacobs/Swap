#include "Scene/Component.h"

#include "Scene/Entity.h"

#include <utility>

Component::~Component()
{
   onDestroyDelegate.broadcast(this);
}

void Component::destroy()
{
   bool destroyed = owner.destroyComponent(this);
   ASSERT(destroyed, "Component not destroyed by owner, possibly already distroyed?");
}

DelegateHandle Component::addOnDestroyDelegate(OnDestroyDelegate::FuncType&& function)
{
   return onDestroyDelegate.add(std::move(function));
}

void Component::removeOnDestroyDelegate(const DelegateHandle& handle)
{
   onDestroyDelegate.remove(handle);
}

// static
ComponentRegistry& ComponentRegistry::instance()
{
   static ComponentRegistry singleton;
   return singleton;
}
