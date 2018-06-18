#include "Platform/Window.h"

#include "Core/Assert.h"

#include <glfw/glfw3.h>

namespace
{
   void framebufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
   {
      Window *window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
      ASSERT(window);

      window->onFramebufferSizeChanged.execute(width, height);
   }

   void windowRefreshCallback(GLFWwindow* glfwWindow)
   {
      Window *window = static_cast<Window*>(glfwGetWindowUserPointer(glfwWindow));
      ASSERT(window);

      window->onWindowRefreshRequested.execute(*window);
   }
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
{
   ASSERT(glfwWindow);

   glfwSetWindowUserPointer(glfwWindow, this);
   glfwSetFramebufferSizeCallback(glfwWindow, framebufferSizeCallback);
   glfwSetWindowRefreshCallback(glfwWindow, windowRefreshCallback);
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

bool Window::shouldClose() const
{
   return glfwWindowShouldClose(glfwWindow) != GLFW_FALSE;
}
