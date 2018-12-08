#include "Scene/Entity.h"

#include "Scene/Scene.h"

#include <algorithm>

Entity::~Entity()
{
   onDestroyDelegate.broadcast(this);
}

void Entity::destroy()
{
   bool destroyed = scene.destroyEntity(this);
   ASSERT(destroyed, "Entity not destroyed by scene, possibly already distroyed?");
}

void Entity::tick(float dt)
{
   for (const UPtr<Component>& component : components)
   {
      component->tick(dt);
   }
}

Component* Entity::createComponentByName(const std::string& className)
{
   UPtr<Component> newComponent = ComponentRegistry::instance().createComponent(*this, className);
   if (!newComponent)
   {
      return nullptr;
   }

   Component* newComponentRaw = newComponent.get();
   components.push_back(std::move(newComponent));
   onComponentCreated(newComponentRaw);
   return newComponentRaw;
}

bool Entity::destroyComponent(Component* componentToDestroy)
{
   auto location = std::find_if(components.begin(), components.end(), [componentToDestroy](const UPtr<Component>& component)
   {
      return component.get() == componentToDestroy;
   });

   if (location != components.end())
   {
      components.erase(location);
      return true;
   }

   return false;
}

DelegateHandle Entity::addOnDestroyDelegate(OnDestroyDelegate::FuncType&& function)
{
   return onDestroyDelegate.add(std::move(function));
}

void Entity::removeOnDestroyDelegate(const DelegateHandle &handle)
{
   onDestroyDelegate.remove(handle);
}

// static
UPtr<Entity> Entity::create(gsl::span<std::string> componentClassNames, Scene& scene)
{
   UPtr<Entity> entity(new Entity(scene));

   entity->components.reserve(componentClassNames.size());
   for (const std::string& className : componentClassNames)
   {
      if (UPtr<Component> newComponent = ComponentRegistry::instance().createComponent(*entity, className))
      {
         entity->components.push_back(std::move(newComponent));
      }
   }
   entity->onInitialized();

   return entity;
}

void Entity::onInitialized()
{
   for (const UPtr<Component>& component : components)
   {
      component->onOwnerInitialized();
   }
}

void Entity::onComponentCreated(Component* component)
{
   for (const UPtr<Component>& component : components)
   {
      component->onComponentAddedToOwner(component.get());
   }
}
