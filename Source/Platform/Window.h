#pragma once

#include "Core/Delegate.h"
#include "Core/Pointers.h"

struct GLFWwindow;

class Window
{
public:
   using FramebufferSizeChangedDelegate = Delegate<void, int, int>;
   using WindowRefreshRequestedDelegate = Delegate<void, Window&>;

   static UPtr<Window> create(int width, int height, const char* title);

private:
   Window(GLFWwindow* internalWindow);

public:
   ~Window();

   void makeContextCurrent();
   void swapBuffers();
   bool shouldClose() const;

   FramebufferSizeChangedDelegate onFramebufferSizeChanged;
   WindowRefreshRequestedDelegate onWindowRefreshRequested;

private:
   GLFWwindow* glfwWindow;
};
