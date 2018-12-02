#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Pointers.h"
#include "Platform/Window.h"
#include "Scene/Scene.h"
#include "Scene/SceneRenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace
{
#if SWAP_DEBUG
   void glfwErrorCallback(int error, const char* description)
   {
      ASSERT(false, "GLFW error %d: %s", error, description);
   }

   void gladPostCallback(const char* name, void* funcptr, int numArgs, ...)
   {
      static const auto getGlErrorName = [](GLenum error)
      {
         switch (error)
         {
         case GL_NO_ERROR:
            return "GL_NO_ERROR";
         case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
         case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
         case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
         case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
         case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
         default:
            return "Unknown";
         }
      };

      GLenum errorCode = glad_glGetError();
      ASSERT(errorCode == GL_NO_ERROR, "OpenGL error %d (%s) in %s", errorCode, getGlErrorName(errorCode), name);
   }
#endif // SWAP_DEBUG

   UPtr<Window> createWindow()
   {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
      glfwWindowHint(GLFW_SAMPLES, 8);

      UPtr<Window> window = Window::create(1280, 720, "Swap");
      if (!window)
      {
         LOG_ERROR_MSG_BOX("Unable to create GLFW window");
         return nullptr;
      }

      window->makeContextCurrent();
      glfwSwapInterval(1);

      if (GLVersion.major == 0 && GLVersion.minor == 0)
      {
         if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
         {
            LOG_ERROR_MSG_BOX("Unable to initialize OpenGL");
            return nullptr;
         }
      }

      glEnable(GL_MULTISAMPLE);

      return window;
   }
}

int main(int argc, char* argv[])
{
#if SWAP_DEBUG
   glfwSetErrorCallback(glfwErrorCallback);
   glad_set_post_callback(gladPostCallback);
#endif // SWAP_DEBUG

   if (!glfwInit())
   {
      LOG_ERROR_MSG_BOX("Unable to initialize GLFW");
      return 1;
   }

   int returnCode = 0;
   if (UPtr<Window> window = createWindow())
   {
      int framebufferWidth = 0;
      int framebufferHeight = 0;
      window->getFramebufferSize(framebufferWidth, framebufferHeight);

      Scene scene;
      SceneRenderer sceneRenderer(framebufferWidth, framebufferHeight);

      window->bindOnFramebufferSizeChanged([&sceneRenderer](int width, int height)
      {
         sceneRenderer.onFramebufferSizeChanged(width, height);
      });

      window->bindOnWindowRefreshRequested([&scene, &sceneRenderer](Window& win)
      {
         sceneRenderer.renderScene(scene);
         win.swapBuffers();
      });

      double lastTime = glfwGetTime();
      while (!window->shouldClose())
      {
         static const double kMaxFrameTime = 0.25;

         double now = glfwGetTime();
         double frameTime = std::min(now - lastTime, kMaxFrameTime);
         lastTime = now;

         scene.tick(static_cast<float>(frameTime));

         sceneRenderer.renderScene(scene);

         window->swapBuffers();
         window->pollEvents();
      }
   }
   else
   {
      returnCode = 1;
   }

   glfwTerminate();
   return returnCode;
}
