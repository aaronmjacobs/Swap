#pragma once

#include "Core/Delegate.h"
#include "Core/Pointers.h"
#include "Platform/InputManager.h"

struct GLFWwindow;

class Window
{
public:
   using FramebufferSizeChangedDelegate = Delegate<void, int, int>;
   using WindowRefreshRequestedDelegate = Delegate<void, Window&>;
   using WindowFocusDelegate = Delegate<void, bool>;

   static UPtr<Window> create(int width, int height, const char* title);

private:
   Window(GLFWwindow* internalWindow);

public:
   ~Window();

   void makeContextCurrent();
   void swapBuffers();
   void pollEvents();
   bool shouldClose() const;

   InputManager& getInputManager()
   {
      return inputManager;
   }

   void getFramebufferSize(int& width, int& height);

   DelegateHandle bindOnFramebufferSizeChanged(FramebufferSizeChangedDelegate::FuncType&& func);
   void unbindOnFramebufferSizeChanged();

   DelegateHandle bindOnWindowRefreshRequested(WindowRefreshRequestedDelegate::FuncType&& func);
   void unbindOnWindowRefreshRequested();

   DelegateHandle bindOnWindowFocusChanged(WindowFocusDelegate::FuncType&& func);
   void unbindOnWindowFocusChanged();

private:
   friend class WindowCallbackHelper;

   void onFramebufferSizeChanged(int width, int height);
   void onWindowRefreshRequested();
   void onWindowFocusChanged(bool focused);
   void onKeyEvent(int key, int scancode, int action, int mods);
   void onCursorPosChanged(double xPos, double yPos);
   void onMouseButtonEvent(int button, int action, int mods);

   void setConsumeCursorInput(bool consume);

   GLFWwindow* glfwWindow;
   InputManager inputManager;

   FramebufferSizeChangedDelegate framebufferSizeChangedDelegate;
   WindowRefreshRequestedDelegate windowRefreshRequestedDelegate;
   WindowFocusDelegate windowFocusChangedDelegate;

   bool hasFocus;
   bool consumeCursorInput;
};
