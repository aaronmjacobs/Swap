#pragma once

#include "Core/Delegate.h"
#include "Platform/InputTypes.h"

#include <GLFW/glfw3.h>

#include <array>
#include <string>
#include <unordered_map>

template<typename KeyType>
using InputMappings = std::unordered_map<KeyType, std::vector<std::string>>;

template<typename DelegateType>
using InputBindings = std::unordered_map<std::string, DelegateType>;

class InputManager
{
public:
   InputManager();

   // Raw input

   using KeyDelegate = MulticastDelegate<void, KeyChord /* keyChord */, bool /* pressed */>;
   using CursorAxisDelegate = MulticastDelegate<void, double /* xPos */, double /* yPos */>;
   using MouseButtonDelegate = MulticastDelegate<void, MouseButtonChord /* mouseButtonChord */, bool /* pressed */>;
   using GamepadButtonDelegate = MulticastDelegate<void, GamepadButtonChord /* gamepadButtonChord */, bool /* pressed */>;
   using GamepadAxisDelegate = MulticastDelegate<void, GamepadAxisChord /* gamepadAxisChord */, float /* value */>;

   DelegateHandle addKeyDelegate(KeyDelegate::FuncType&& function);
   void removeKeyDelegate(const DelegateHandle& handle);

   DelegateHandle addMouseButtonDelegate(MouseButtonDelegate::FuncType&& function);
   void removeMouseButtonDelegate(const DelegateHandle& handle);

   DelegateHandle addCursorAxisDelegate(CursorAxisDelegate::FuncType&& function);
   void removeCursorAxisDelegate(const DelegateHandle& handle);

   DelegateHandle addGamepadButtonDelegate(GamepadButtonDelegate::FuncType&& function);
   void removeGamepadButtonDelegate(const DelegateHandle& handle);

   DelegateHandle addGamepadAxisDelegate(GamepadAxisDelegate::FuncType&& function);
   void removeGamepadAxisDelegate(const DelegateHandle& handle);

   // Mapped input

   using ButtonInputDelegate = MulticastDelegate<void, bool /* pressed */>;
   using AxisInputDelegate = MulticastDelegate<void, float /* value */>;

   void createButtonMapping(const std::string& action, const KeyChord* keyChord, const MouseButtonChord* mouseButtonChord, const GamepadButtonChord* gamepadButtonChord);
   void destroyButtonMapping(const std::string& action);

   void createAxisMapping(const std::string& action, const CursorAxis* cursorAxis, const GamepadAxisChord* gamepadAxisChord);
   void destroyAxisMapping(const std::string& action);

   DelegateHandle bindButtonMapping(const std::string& action, ButtonInputDelegate::FuncType&& function);
   void unbindButtonMapping(const DelegateHandle& handle);

   DelegateHandle bindAxisMapping(const std::string& action, AxisInputDelegate::FuncType&& function);
   void unbindAxisMapping(const DelegateHandle& handle);

private:
   friend class Window;

   void onKeyEvent(int key, int scancode, int action, int mods);
   void onMouseButtonEvent(int button, int action, int mods);
   void onCursorPosChanged(double xPos, double yPos);

   void pollGamepads();
   void pollGamepad(int gamepadId);

   KeyDelegate keyDelegate;
   MouseButtonDelegate mouseButtonDelegate;
   CursorAxisDelegate cursorAxisDelegate;
   GamepadButtonDelegate gamepadButtonDelegate;
   GamepadAxisDelegate gamepadAxisDelegate;

   InputMappings<KeyChord> keyMappings;
   InputMappings<MouseButtonChord> mouseButtonMappings;
   InputMappings<CursorAxis> cursorAxisMappings;
   InputMappings<GamepadButtonChord> gamepadButtonMappings;
   InputMappings<GamepadAxisChord> gamepadAxisMappings;

   InputBindings<ButtonInputDelegate> buttonBindings;
   InputBindings<AxisInputDelegate> axisBindings;

   std::array<GLFWgamepadstate, GLFW_JOYSTICK_LAST + 1> gamepadStates;
};
