#include "Platform/Window.h"

#include "Core/Assert.h"

#include <GLFW/glfw3.h>

#include <utility>

class WindowCallbackHelper
{
public:
   static void framebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height);
   static void windowRefreshCallback(GLFWwindow* glfwWindow);
   static void windowFocusCallback(GLFWwindow* glfwWindow, int focused);
   static void keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods);
   static void cursorPosCallback(GLFWwindow* glfwWindow, double xPos, double yPos);
   static void mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods);
};

// static
void WindowCallbackHelper::framebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
{
   Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
   ASSERT(window);

   window->onFramebufferSizeChanged(width, height);
}

// static
void WindowCallbackHelper::windowRefreshCallback(GLFWwindow* glfwWindow)
{
   Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
   ASSERT(window);

   window->onWindowRefreshRequested();
}

// static
void WindowCallbackHelper::windowFocusCallback(GLFWwindow* glfwWindow, int focused)
{
   Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
   ASSERT(window);

   window->onWindowFocusChanged(focused == GLFW_TRUE);
}

// static
void WindowCallbackHelper::keyCallback(GLFWwindow* glfwWindow, int key, int scancode, int action, int mods)
{
   Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
   ASSERT(window);

   window->onKeyEvent(key, scancode, action, mods);
}

// static
void WindowCallbackHelper::cursorPosCallback(GLFWwindow* glfwWindow, double xPos, double yPos)
{
   Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
   ASSERT(window);

   window->onCursorPosChanged(xPos, yPos);
}

void WindowCallbackHelper::mouseButtonCallback(GLFWwindow* glfwWindow, int button, int action, int mods)
{
   Window* window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
   ASSERT(window);

   window->onMouseButtonEvent(button, action, mods);
}

// static
UPtr<Window> Window::create(int width, int height, const char* title)
{
   GLFWwindow* internalWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

   UPtr<Window> window;
   if (internalWindow)
   {
      window = UPtr<Window>(new Window(internalWindow));
   }

   return window;
}

Window::Window(GLFWwindow* internalWindow)
   : glfwWindow(internalWindow)
   , hasFocus(true)
   , consumeCursorInput(true)
{
   ASSERT(glfwWindow);

   glfwSetWindowUserPointer(glfwWindow, this);

   setConsumeCursorInput(true);

   double cursorX = 0.0;
   double cursorY = 0.0;
   glfwGetCursorPos(glfwWindow, &cursorX, &cursorY);
   inputManager.init(cursorX, cursorY);

   glfwSetFramebufferSizeCallback(glfwWindow, WindowCallbackHelper::framebufferSizeCallback);
   glfwSetWindowRefreshCallback(glfwWindow, WindowCallbackHelper::windowRefreshCallback);
   glfwSetWindowFocusCallback(glfwWindow, WindowCallbackHelper::windowFocusCallback);
   glfwSetKeyCallback(glfwWindow, WindowCallbackHelper::keyCallback);
   glfwSetMouseButtonCallback(glfwWindow, WindowCallbackHelper::mouseButtonCallback);
   glfwSetCursorPosCallback(glfwWindow, WindowCallbackHelper::cursorPosCallback);
}

Window::~Window()
{
   glfwDestroyWindow(glfwWindow);
}

void Window::makeContextCurrent()
{
   glfwMakeContextCurrent(glfwWindow);
}

void Window::swapBuffers()
{
   glfwSwapBuffers(glfwWindow);
}

void Window::pollEvents()
{
   glfwPollEvents();
   inputManager.pollEvents();
}

bool Window::shouldClose() const
{
   return glfwWindowShouldClose(glfwWindow) != GLFW_FALSE;
}

void Window::setTitle(const char* title)
{
   glfwSetWindowTitle(glfwWindow, title);
}

void Window::getFramebufferSize(int& width, int& height)
{
   glfwGetFramebufferSize(glfwWindow, &width, &height);
}

DelegateHandle Window::bindOnFramebufferSizeChanged(FramebufferSizeChangedDelegate::FuncType&& func)
{
   return framebufferSizeChangedDelegate.bind(std::move(func));
}

void Window::unbindOnFramebufferSizeChanged()
{
   framebufferSizeChangedDelegate.unbind();
}

DelegateHandle Window::bindOnWindowRefreshRequested(WindowRefreshRequestedDelegate::FuncType&& func)
{
   return windowRefreshRequestedDelegate.bind(std::move(func));
}

void Window::unbindOnWindowRefreshRequested()
{
   windowRefreshRequestedDelegate.unbind();
}

DelegateHandle Window::bindOnWindowFocusChanged(WindowFocusDelegate::FuncType&& func)
{
   return windowFocusChangedDelegate.bind(std::move(func));
}

void Window::unbindOnWindowFocusChanged()
{
   windowFocusChangedDelegate.unbind();
}

void Window::onFramebufferSizeChanged(int width, int height)
{
   if (framebufferSizeChangedDelegate.isBound())
   {
      framebufferSizeChangedDelegate.execute(width, height);
   }
}

void Window::onWindowRefreshRequested()
{
   if (windowRefreshRequestedDelegate.isBound())
   {
      windowRefreshRequestedDelegate.execute(*this);
   }
}

void Window::onWindowFocusChanged(bool focused)
{
   hasFocus = focused;

   if (focused)
   {
      double cursorX = 0.0;
      double cursorY = 0.0;
      glfwGetCursorPos(glfwWindow, &cursorX, &cursorY);

      int width = 0;
      int height = 0;
      glfwGetWindowSize(glfwWindow, &width, &height);

      // Ignore the event if the cursor isn't within the bounds of the window
      if (cursorX >= 0.0 && cursorX <= width && cursorY >= 0.0 && cursorY <= height)
      {
         setConsumeCursorInput(true);
      }
   }
   else
   {
      setConsumeCursorInput(false);
   }

   if (windowFocusChangedDelegate.isBound())
   {
      windowFocusChangedDelegate.execute(focused);
   }
}

void Window::onKeyEvent(int key, int scancode, int action, int mods)
{
   inputManager.onKeyEvent(key, scancode, action, mods);
}

void Window::onCursorPosChanged(double xPos, double yPos)
{
   inputManager.onCursorPosChanged(xPos, yPos, consumeCursorInput);

   if (hasFocus && !consumeCursorInput)
   {
      // Cursor is now within the window bounds
      setConsumeCursorInput(true);
   }
}

void Window::onMouseButtonEvent(int button, int action, int mods)
{
   inputManager.onMouseButtonEvent(button, action, mods);
}

void Window::setConsumeCursorInput(bool consume)
{
   consumeCursorInput = consume;
   glfwSetInputMode(glfwWindow, GLFW_CURSOR, consume ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}
