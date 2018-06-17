#pragma once

#include "Assert.h"

#include <algorithm>
#include <cstdint>
#include <functional>
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

   uint64_t id;
};

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

   ReturnType execute(Params... params)
   {
      ASSERT(isBound());
      return function(params...);
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

   void execute(Params... params)
   {
      for (DelegateType& delegate : delegates)
      {
         delegate.execute(params...);
      }
   }

   std::vector<ReturnType> executeWithReturn(Params... params)
   {
      std::vector<ReturnType> returnValues(delegates.size());

      for (std::size_t i = 0; i < delegates.size(); ++i)
      {
         returnValues[i] = delegates[i].execute(params...);
      }

      return returnValues;
   }

private:
   std::vector<DelegateType> delegates;
};
