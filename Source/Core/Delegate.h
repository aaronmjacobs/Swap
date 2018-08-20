#pragma once

#include "Core/Assert.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

class DelegateHandle
{
public:
   static DelegateHandle create();

   DelegateHandle()
      : id(0)
   {
   }

   bool isValid() const
   {
      return id != 0;
   }

   void invalidate()
   {
      id = 0;
   }

   friend bool operator==(const DelegateHandle& first, const DelegateHandle& second)
   {
      return first.id == second.id;
   }

private:
   static uint64_t counter;

   friend struct std::hash<DelegateHandle>;

   uint64_t id;
};

namespace std
{
   template<>
   struct hash<DelegateHandle>
   {
      size_t operator()(const DelegateHandle& delegateHandle) const
      {
         std::hash<uint64_t> hasher;
         return hasher(delegateHandle.id);
      }
   };
}

template<typename RetType, typename... Params>
class Delegate
{
public:
   using ReturnType = RetType;
   using FuncType = std::function<ReturnType(Params...)>;

   DelegateHandle bind(FuncType&& func)
   {
      handle = DelegateHandle::create();
      function = std::move(func);

      return handle;
   }

   void unbind()
   {
      handle.invalidate();
      function = nullptr;
   }

   bool isBound() const
   {
      return !!function;
   }

   ReturnType execute(Params... params) const
   {
      ASSERT(isBound());
      return function(std::forward<Params>(params)...);
   }

   const DelegateHandle& getHandle() const
   {
      return handle;
   }

private:
   FuncType function;
   DelegateHandle handle;
};

template<typename RetType, typename... Params>
class MulticastDelegate
{
public:
   using ReturnType = RetType;
   using FuncType = std::function<ReturnType(Params...)>;
   using DelegateType = Delegate<ReturnType, Params...>;

   DelegateHandle add(FuncType&& function)
   {
      Delegate<RetType, Params...> newDelegate;
      DelegateHandle handle = newDelegate.bind(std::move(function));

      delegates.push_back(std::move(newDelegate));

      return handle;
   }

   void remove(const DelegateHandle& handle)
   {
      delegates.erase(std::remove_if(delegates.begin(), delegates.end(), [&handle](const DelegateType& delegate) { return delegate.getHandle() == handle; }), delegates.end());
   }

   void clear()
   {
      delegates.clear();
   }

   bool isBound() const
   {
      return !delegates.empty();
   }

   void broadcast(Params... params) const
   {
      for (const DelegateType& delegate : delegates)
      {
         delegate.execute(std::forward<Params>(params)...);
      }
   }

   std::vector<ReturnType> broadcastWithReturn(Params... params) const
   {
      std::vector<ReturnType> returnValues(delegates.size());

      for (std::size_t i = 0; i < delegates.size(); ++i)
      {
         returnValues[i] = delegates[i].execute(std::forward<Params>(params)...);
      }

      return returnValues;
   }

private:
   std::vector<DelegateType> delegates;
};
