#pragma once

#include "Core/Assert.h"
#include "Core/Delegate.h"
#include "Core/Pointers.h"
#include "Scene/Components/Component.h"

#include <gsl/span>

#include <string>
#include <utility>
#include <vector>

class Scene;

class Entity
{
public:
   using OnDestroyDelegate = MulticastDelegate<void, Entity*>;

   ~Entity();

   void destroy();

   void tick(float dt);

   template<typename T>
   T* createComponent();

   Component* createComponentByName(const std::string& className);
   bool destroyComponent(Component* componentToDestroy);

   template<typename T>
   T* getComponentByClass();

   template<typename T>
   const T* getComponentByClass() const;

   template<typename T>
   std::vector<T*> getComponentsByClass();

   template<typename T>
   std::vector<const T*> getComponentsByClass() const;

   DelegateHandle addOnDestroyDelegate(OnDestroyDelegate::FuncType&& function);
   void removeOnDestroyDelegate(const DelegateHandle& handle);

   Scene& getScene()
   {
      return scene;
   }

   const Scene& getScene() const
   {
      return scene;
   }

private:
   friend class Scene;

   template<typename... ComponentTypes>
   static UPtr<Entity> create(Scene& scene);

   static UPtr<Entity> create(gsl::span<std::string> componentClassNames, Scene& scene);

   Entity(Scene& owningScene)
      : scene(owningScene)
   {
   }

   template<typename First, typename... Rest>
   void constructComponentsHelper();

   template<typename... ComponentTypes>
   void constructComponents();

   void onInitialized();
   void onComponentCreated(Component* component);

   std::vector<UPtr<Component>> components;
   OnDestroyDelegate onDestroyDelegate;
   Scene& scene;
};

template<typename T>
T* Entity::createComponent()
{
   UPtr<T> newComponent = ComponentRegistry::instance().createComponent<T>(*this);
   ASSERT(newComponent, "Failed to create a typed component");

   T* newComponentRaw = newComponent.get();
   components.push_back(std::move(newComponent));
   onComponentCreated(newComponentRaw);
   return newComponentRaw;
}

template<typename T>
T* Entity::getComponentByClass()
{
   for (const UPtr<Component>& component : components)
   {
      if (T* castedComponent = dynamic_cast<T*>(component.get()))
      {
         return castedComponent;
      }
   }

   return nullptr;
}

template<typename T>
const T* Entity::getComponentByClass() const
{
   for (const UPtr<Component>& component : components)
   {
      if (const T* castedComponent = dynamic_cast<const T*>(component.get()))
      {
         return castedComponent;
      }
   }

   return nullptr;
}

template<typename T>
std::vector<T*> Entity::getComponentsByClass()
{
   std::vector<T*> componentsOfClass;

   for (const UPtr<Component>& component : components)
   {
      if (T* castedComponent = dynamic_cast<T*>(component.get()))
      {
         componentsOfClass.push_back(castedComponent);
      }
   }

   return componentsOfClass;
}

template<typename T>
std::vector<const T*> Entity::getComponentsByClass() const
{
   std::vector<const T*> componentsOfClass;

   for (const UPtr<Component>& component : components)
   {
      if (const T* castedComponent = dynamic_cast<const T*>(component.get()))
      {
         componentsOfClass.push_back(castedComponent);
      }
   }

   return componentsOfClass;
}

// static
template<typename... ComponentTypes>
UPtr<Entity> Entity::create(Scene& scene)
{
   UPtr<Entity> entity(new Entity(scene));

   entity->constructComponents<ComponentTypes...>();
   entity->onInitialized();

   return entity;
}

template<typename First, typename... Rest>
void Entity::constructComponentsHelper()
{
   components.push_back(ComponentRegistry::instance().createComponent<First>(*this));
   constructComponents<Rest...>();
}

template<typename... ComponentTypes>
void Entity::constructComponents()
{
   constructComponentsHelper<ComponentTypes...>();
}

template<>
inline void Entity::constructComponents()
{
}
