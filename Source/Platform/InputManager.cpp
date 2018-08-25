#include "Platform/InputManager.h"

#include <glm/glm.hpp>

#include <utility>

namespace
{
   template<typename KeyType>
   void createMapping(InputMappings<KeyType>& mappings, const KeyType* value, const std::string& action)
   {
      if (value)
      {
         auto mappingLocation = mappings.find(*value);

         if (mappingLocation == mappings.end())
         {
            auto pair = mappings.emplace(*value, std::vector<std::string>());
            mappingLocation = pair.first;
         }

         std::vector<std::string>& actions = mappingLocation->second;
         actions.push_back(action);
      }
   }

   template<typename KeyType>
   void destroyMapping(InputMappings<KeyType>& mappings, const std::string& action)
   {
      for (auto it = mappings.begin(); it != mappings.end();)
      {
         std::vector<std::string>& actions = it->second;
         actions.erase(std::remove_if(actions.begin(), actions.end(), [&action](const std::string& act)
         {
            return act == action;
         }), actions.end());

         if (actions.empty())
         {
            it = mappings.erase(it);
         }
         else
         {
            ++it;
         }
      }
   }

   template<typename DelegateType>
   DelegateHandle bindMapping(InputBindings<DelegateType>& bindings, const std::string& action, typename DelegateType::FuncType&& function)
   {
      auto pair = bindings.emplace(action, DelegateType());
      const auto& bindingLocation = pair.first;
      auto& delegate = bindingLocation->second;
      return delegate.add(std::move(function));
   }

   template<typename DelegateType>
   void unbindMapping(InputBindings<DelegateType>& bindings, const DelegateHandle& handle)
   {
      for (auto it = bindings.begin(); it != bindings.end();)
      {
         DelegateType& delegate = it->second;
         delegate.remove(handle);

         if (!delegate.isBound())
         {
            it = bindings.erase(it);
         }
         else
         {
            ++it;
         }
      }
   }

   template<typename KeyType, typename DelegateType, typename ValueType>
   void broadcastEvent(const InputMappings<KeyType>& mappings, InputBindings<DelegateType>& bindings, KeyType key, ValueType value)
   {
      auto mapingLocation = mappings.find(key);
      if (mapingLocation != mappings.end())
      {
         const auto& actions = mapingLocation->second;
         for (const std::string& action : actions)
         {
            auto bindingLocation = bindings.find(action);
            if (bindingLocation != bindings.end())
            {
               auto& delegate = bindingLocation->second;
               delegate.broadcast(value);
            }
         }
      }
   }

   float applyDeadzone(float value)
   {
      static const float kDeadzone = 0.25f;
      static const float kDeadzoneScale = 1.0f / (1.0f - kDeadzone);

      float clampedValue = glm::max(0.0f, glm::abs(value) - kDeadzone) * glm::sign(value);
      float scaledValue = clampedValue * kDeadzoneScale;

      return scaledValue;
   }
}

InputManager::InputManager()
   : lastMouseX(0.0)
   , lastMouseY(0.0)
   , gamepadStates{}
{
}

void InputManager::init(double cursorX, double cursorY)
{
   lastMouseX = cursorX;
   lastMouseY = cursorY;
}

DelegateHandle InputManager::addKeyDelegate(KeyDelegate::FuncType&& function)
{
   return keyDelegate.add(std::move(function));
}

void InputManager::removeKeyDelegate(const DelegateHandle& handle)
{
   keyDelegate.remove(handle);
}

DelegateHandle InputManager::addMouseButtonDelegate(MouseButtonDelegate::FuncType&& function)
{
   return mouseButtonDelegate.add(std::move(function));
}

void InputManager::removeMouseButtonDelegate(const DelegateHandle& handle)
{
   mouseButtonDelegate.remove(handle);
}

DelegateHandle InputManager::addCursorAxisDelegate(CursorAxisDelegate::FuncType&& function)
{
   return cursorAxisDelegate.add(std::move(function));
}

void InputManager::removeCursorAxisDelegate(const DelegateHandle& handle)
{
   cursorAxisDelegate.remove(handle);
}

DelegateHandle InputManager::addGamepadButtonDelegate(GamepadButtonDelegate::FuncType&& function)
{
   return gamepadButtonDelegate.add(std::move(function));
}

void InputManager::removeGamepadButtonDelegate(const DelegateHandle& handle)
{
   gamepadButtonDelegate.remove(handle);
}

DelegateHandle InputManager::addGamepadAxisDelegate(GamepadAxisDelegate::FuncType&& function)
{
   return gamepadAxisDelegate.add(std::move(function));
}

void InputManager::removeGamepadAxisDelegate(const DelegateHandle& handle)
{
   gamepadAxisDelegate.remove(handle);
}

void InputManager::createButtonMapping(const std::string& action, const KeyChord* keyChord, const MouseButtonChord* mouseButtonChord, const GamepadButtonChord* gamepadButtonChord)
{
   createMapping(keyMappings, keyChord, action);
   createMapping(mouseButtonMappings, mouseButtonChord, action);
   createMapping(gamepadButtonMappings, gamepadButtonChord, action);
}

void InputManager::destroyButtonMapping(const std::string& action)
{
   destroyMapping(keyMappings, action);
   destroyMapping(mouseButtonMappings, action);
   destroyMapping(gamepadButtonMappings, action);
}

void InputManager::createAxisMapping(const std::string& action, const CursorAxis* cursorAxis, const GamepadAxisChord* gamepadAxisChord)
{
   createMapping(cursorAxisMappings, cursorAxis, action);
   createMapping(gamepadAxisMappings, gamepadAxisChord, action);
}

void InputManager::destroyAxisMapping(const std::string& action)
{
   destroyMapping(cursorAxisMappings, action);
   destroyMapping(gamepadAxisMappings, action);
}

DelegateHandle InputManager::bindButtonMapping(const std::string& action, ButtonInputDelegate::FuncType&& function)
{
   return bindMapping(buttonBindings, action, std::move(function));
}

void InputManager::unbindButtonMapping(const DelegateHandle& handle)
{
   unbindMapping(buttonBindings, handle);
}

DelegateHandle InputManager::bindAxisMapping(const std::string& action, AxisInputDelegate::FuncType&& function)
{
   return bindMapping(axisBindings, action, std::move(function));
}

void InputManager::unbindAxisMapping(const DelegateHandle& handle)
{
   unbindMapping(axisBindings, handle);
}

void InputManager::onKeyEvent(int key, int scancode, int action, int mods)
{
   if (action != GLFW_REPEAT)
   {
      KeyChord keyChord;
      keyChord.key = static_cast<Key>(key);
      keyChord.mods = static_cast<KeyMod::Enum>(mods);

      bool pressed = action == GLFW_PRESS;

      keyDelegate.broadcast(keyChord, pressed);

      broadcastEvent(keyMappings, buttonBindings, keyChord, pressed);
   }
}

void InputManager::onMouseButtonEvent(int button, int action, int mods)
{
   MouseButtonChord mouseButtonChord;
   mouseButtonChord.button = static_cast<MouseButton>(button);
   mouseButtonChord.mods = static_cast<KeyMod::Enum>(mods);

   bool pressed = action == GLFW_PRESS;

   mouseButtonDelegate.broadcast(mouseButtonChord, pressed);

   broadcastEvent(mouseButtonMappings, buttonBindings, mouseButtonChord, pressed);
}

void InputManager::onCursorPosChanged(double xPos, double yPos)
{
   static const double kMouseSensitivity = 0.1;

   cursorAxisDelegate.broadcast(xPos, yPos);

   double xDiff = (xPos - lastMouseX) * kMouseSensitivity;
   double yDiff = (yPos - lastMouseY) * kMouseSensitivity;

   broadcastEvent(cursorAxisMappings, axisBindings, CursorAxis::X, xDiff);
   broadcastEvent(cursorAxisMappings, axisBindings, CursorAxis::Y, yDiff);

   lastMouseX = xPos;
   lastMouseY = yPos;
}

void InputManager::pollGamepads()
{
   for (int gamepadId = 0; gamepadId <= GLFW_JOYSTICK_LAST; ++gamepadId)
   {
      pollGamepad(gamepadId);
   }
}

void InputManager::pollGamepad(int gamepadId)
{
   GLFWgamepadstate& currentGamepadState = gamepadStates[gamepadId];
   GLFWgamepadstate newGamepadState = {};

   if (glfwGetGamepadState(gamepadId, &newGamepadState) == GLFW_TRUE)
   {
      for (int buttonId = 0; buttonId <= GLFW_GAMEPAD_BUTTON_LAST; ++buttonId)
      {
         if (currentGamepadState.buttons[buttonId] != newGamepadState.buttons[buttonId])
         {
            GamepadButtonChord gamepadButtonChord;
            gamepadButtonChord.button = static_cast<GamepadButton>(buttonId);
            gamepadButtonChord.gamepadId = gamepadId;

            bool pressed = newGamepadState.buttons[buttonId] == GLFW_PRESS;

            gamepadButtonDelegate.broadcast(gamepadButtonChord, pressed);

            broadcastEvent(gamepadButtonMappings, buttonBindings, gamepadButtonChord, pressed);
         }
      }

      for (int axisId = 0; axisId <= GLFW_GAMEPAD_AXIS_LAST; ++axisId)
      {
         newGamepadState.axes[axisId] = applyDeadzone(newGamepadState.axes[axisId]);

         if (currentGamepadState.axes[axisId] != newGamepadState.axes[axisId])
         {
            GamepadAxisChord gamepadAxisChord;
            gamepadAxisChord.axis = static_cast<GamepadAxis>(axisId);
            gamepadAxisChord.gamepadId = gamepadId;

            float value = newGamepadState.axes[axisId];

            gamepadAxisDelegate.broadcast(gamepadAxisChord, value);

            broadcastEvent(gamepadAxisMappings, axisBindings, gamepadAxisChord, value);
         }
      }

      gamepadStates[gamepadId] = newGamepadState;
   }
}