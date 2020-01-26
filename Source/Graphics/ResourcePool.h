#pragma once

#include "Core/Delegate.h"
#include "Core/Pointers.h"

#include <algorithm>
#include <map>
#include <string>

// Type needs to be a subclass of GraphicsResource, and must implement the following:
// using SpecificationType = <specification type>;
// static SPtr<T> create(const SpecificationType& specification);
// static const char* labelSuffix();

template<typename T>
class ResourcePool
{
public:
   using Spec = typename T::SpecificationType;
   using ResourceCreatedDelegate = Delegate<void, T&>;

   void clear()
   {
      pool.clear();
   }

   void clearUnreferenced()
   {
      pool.erase(std::remove_if(pool.begin(), pool.end(), [](const SPtr<T>& element) { return element.use_count() == 1; }), pool.end());
   }

   SPtr<T> obtain(const Spec& spec)
   {
      auto range = pool.equal_range(spec);
      for (auto it = range.first; it != range.second; ++it)
      {
         SPtr<T>& resource = it->second;

         if (resource.use_count() == 1)
         {
            return resource;
         }
      }

      SPtr<T> resource = T::create(spec);
      resource->setLabel("Pooled " + std::string(T::labelSuffix()) + " | " + std::to_string(count++));

      pool.insert({ { spec, resource } });

      if (resourceCreatedDelegate.isBound())
      {
         resourceCreatedDelegate.execute(*resource);
      }

      return resource;
   }

   DelegateHandle bindOnResourceCreated(typename ResourceCreatedDelegate::FuncType&& func)
   {
      return resourceCreatedDelegate.bind(std::move(func));
   }

   void unbindOnResourceCreated()
   {
      resourceCreatedDelegate.unbind();
   }

private:
   std::unordered_multimap<Spec, SPtr<T>> pool;
   int count = 0;

   ResourceCreatedDelegate resourceCreatedDelegate;
};
