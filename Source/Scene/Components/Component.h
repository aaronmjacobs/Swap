#pragma once

#include "Core/Assert.h"
#include "Core/Delegate.h"
#include "Core/Pointers.h"

#include <functional>
#include <string>
#include <unordered_map>

class Entity;
class Scene;

class Component
{
public:
   using OnDestroyDelegate = MulticastDelegate<void, Component*>;
   using TickFunction = std::function<void(Component*, float)>;

   virtual ~Component();

   void destroy();

   DelegateHandle addOnDestroyDelegate(OnDestroyDelegate::FuncType&& function);
   void removeOnDestroyDelegate(const DelegateHandle& handle);

   void tick(float dt);
   void setTickFunction(TickFunction newTickFunction);
   void clearTickFunction();

   Entity& getEntity()
   {
      return entity;
   }

   const Entity& getEntity() const
   {
      return entity;
   }

   Scene& getScene();
   const Scene& getScene() const;

protected:
   Component(Entity& owningEntity)
      : entity(owningEntity)
   {
   }

   virtual void onOwnerInitialized()
   {
   }

   virtual void onComponentAddedToOwner(Component* component)
   {
   }

private:
   friend class Entity;

   Entity& entity;
   OnDestroyDelegate onDestroyDelegate;
   TickFunction tickFunction;
};

template<typename T>
class ComponentRegistrar
{
public:
   ComponentRegistrar(const std::string& className);
   ~ComponentRegistrar();

private:
   friend class ComponentRegistry;

   static UPtr<T> createTypedComponent(Entity& entity)
   {
      return UPtr<T>(new T(entity));
   }

   static UPtr<Component> createComponent(Entity& entity)
   {
      return createTypedComponent(entity);
   }

   std::string componentClassName;
};

#if defined(_MSC_VER) && _MSC_VER <= 1900
#  define SWAP_SUPPRESS_UNRECOGNIZED_ATTRIBUTE __pragma(warning(suppress: 5030))
#else
#  define SWAP_SUPPRESS_UNRECOGNIZED_ATTRIBUTE
#endif

#define SWAP_REFERENCE_COMPONENT(component_name) \
namespace InitializationHack \
{ \
namespace component_name \
{ \
extern const int kDummyInteger; \
namespace \
{ \
SWAP_SUPPRESS_UNRECOGNIZED_ATTRIBUTE [[maybe_unused]] int referenceDummyInteger() { return kDummyInteger; } \
} /* namespace */ \
} /* namespace component_name */ \
} /* namespace InitializationHack */

#define SWAP_REGISTER_COMPONENT(component_name) \
namespace InitializationHack \
{ \
namespace component_name \
{ \
const int kDummyInteger = 0; \
} /* namespace component_name */ \
} /* namespace InitializationHack */\
static const ComponentRegistrar<component_name> kSwap##component_name##Registrar(#component_name);

class ComponentRegistry
{
private:
   template<typename T> friend class ComponentRegistrar;
   friend class Entity;

   using CreateComponentFunc = UPtr<Component>(*)(Entity&);

   static ComponentRegistry& instance();

   ComponentRegistry() = default;
   ComponentRegistry(const ComponentRegistry& other) = delete;
   ComponentRegistry(ComponentRegistry&& other) = delete;
   ComponentRegistry& operator=(const ComponentRegistry& other) = delete;
   ComponentRegistry& operator=(ComponentRegistry&& other) = delete;

   void registerComponent(const std::string& className, CreateComponentFunc createComponentFunc)
   {
      auto pair = componentMap.emplace(className, createComponentFunc);
      ASSERT(pair.second, "Trying to register a component that has already been registered!");
   }

   void unregisterComponent(const std::string& className)
   {
      auto itr = componentMap.find(className);
      ASSERT(itr != componentMap.end(), "Trying to unregister a component that has not been registered!");

      componentMap.erase(itr);
   }

   template<typename T>
   UPtr<T> createComponent(Entity& entity)
   {
      return ComponentRegistrar<T>::createTypedComponent(entity);
   }

   UPtr<Component> createComponent(Entity& entity, const std::string& className)
   {
      auto location = componentMap.find(className);
      if (location != componentMap.end())
      {
         CreateComponentFunc& createComponentFunc = location->second;
         return createComponentFunc(entity);
      }

      return nullptr;
   }

   std::unordered_map<std::string, CreateComponentFunc> componentMap;
};

template<typename T>
ComponentRegistrar<T>::ComponentRegistrar(const std::string& className)
   : componentClassName(className)
{
   ComponentRegistry::instance().registerComponent(componentClassName, &createComponent);
}

template<typename T>
ComponentRegistrar<T>::~ComponentRegistrar()
{
   // Because the ComponentRegistry singleton will complete construction before any of the ComponentRegistrars
   // (since it is created inside the above constructor), it will not be destroyed until after all
   // ComponentRegistrars are destroyed. Therefore, it is safe to access it in this destructor.
   ComponentRegistry::instance().unregisterComponent(componentClassName);
}
