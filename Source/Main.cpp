#define SWAP_USE_WINMAIN (SWAP_PLATFORM_WINDOWS && !SWAP_DEBUG)

#if SWAP_USE_WINMAIN
#  pragma comment(linker, "/SUBSYSTEM:WINDOWS")
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#endif // SWAP_USE_WINMAIN

#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Pointers.h"
#include "Graphics/GraphicsContext.h"
#include "Graphics/GraphicsDefines.h"
#include "Graphics/Model.h"
#include "Platform/IOUtils.h"
#include "Platform/Window.h"
#include "Resources/ResourceManager.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/Lights/DirectionalLightComponent.h"
#include "Scene/Components/Lights/PointLightComponent.h"
#include "Scene/Components/Lights/SpotLightComponent.h"
#include "Scene/Components/ModelComponent.h"
#include "Scene/Entity.h"
#include "Scene/Scene.h"
#include "Scene/Rendering/DeferredSceneRenderer.h"
#include "Scene/Rendering/ForwardSceneRenderer.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace
{
#if SWAP_DEBUG
   void glfwErrorCallback(int error, const char* description)
   {
      ASSERT(false, "GLFW error %d: %s", error, description);
   }

   void gladPostCallback(void* ret, const char* name, GLADapiproc funcptr, int numArgs, ...)
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

   const int kNumSamples = 0;

   UPtr<Window> createWindow()
   {
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, SWAP_DESIRED_GL_VERSION_MAJOR);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, SWAP_DESIRED_GL_VERSION_MINOR);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
      glfwWindowHint(GLFW_SAMPLES, kNumSamples);

      UPtr<Window> window = Window::create(1280, 720, "Swap");
      if (!window)
      {
         LOG_ERROR_MSG_BOX("Unable to create GLFW window");
         return nullptr;
      }

      window->makeContextCurrent();
      glfwSwapInterval(1);

      if (!gladLoadGL(reinterpret_cast<GLADloadfunc>(glfwGetProcAddress)))
      {
         LOG_ERROR_MSG_BOX("Unable to initialize OpenGL");
         return nullptr;
      }

      GraphicsContext::current().initialize();

      return window;
   }

   void bindInputs(InputManager& inputManager, Scene& scene)
   {
      {
         KeyAxisChord moveForwardKeyAxisChord;
         moveForwardKeyAxisChord.keyChord.key = Key::W;
         GamepadAxisChord moveForwardGamepadAxisChord;
         moveForwardGamepadAxisChord.axis = GamepadAxis::LeftY;
         moveForwardGamepadAxisChord.gamepadId = 0;
         inputManager.createAxisMapping("MoveForward", &moveForwardKeyAxisChord, nullptr, &moveForwardGamepadAxisChord);

         KeyAxisChord moveBackwardKeyAxisChord;
         moveBackwardKeyAxisChord.keyChord.key = Key::S;
         moveBackwardKeyAxisChord.invert = true;
         inputManager.createAxisMapping("MoveForward", &moveBackwardKeyAxisChord, nullptr, nullptr);

         KeyAxisChord moveRightKeyAxisChord;
         moveRightKeyAxisChord.keyChord.key = Key::D;
         GamepadAxisChord moveRightGamepadAxisChord;
         moveRightGamepadAxisChord.axis = GamepadAxis::LeftX;
         moveRightGamepadAxisChord.gamepadId = 0;
         inputManager.createAxisMapping("MoveRight", &moveRightKeyAxisChord, nullptr, &moveRightGamepadAxisChord);

         KeyAxisChord moveLeftKeyAxisChord;
         moveLeftKeyAxisChord.keyChord.key = Key::A;
         moveLeftKeyAxisChord.invert = true;
         inputManager.createAxisMapping("MoveRight", &moveLeftKeyAxisChord, nullptr, nullptr);

         KeyAxisChord moveUpKeyAxisChord;
         moveUpKeyAxisChord.keyChord.key = Key::LeftShift;
         GamepadAxisChord moveUpGamepadAxisChord;
         moveUpGamepadAxisChord.axis = GamepadAxis::RightTrigger;
         moveUpGamepadAxisChord.gamepadId = 0;
         inputManager.createAxisMapping("MoveUp", &moveUpKeyAxisChord, nullptr, &moveUpGamepadAxisChord);

         KeyAxisChord moveDownKeyAxisChord;
         moveDownKeyAxisChord.keyChord.key = Key::LeftControl;
         moveDownKeyAxisChord.invert = true;
         GamepadAxisChord moveDownGamepadAxisChord;
         moveDownGamepadAxisChord.axis = GamepadAxis::LeftTrigger;
         moveDownGamepadAxisChord.gamepadId = 0;
         moveDownGamepadAxisChord.invert = true;
         inputManager.createAxisMapping("MoveUp", &moveDownKeyAxisChord, nullptr, &moveDownGamepadAxisChord);

         CursorAxisChord lookUpCursorAxisChord;
         lookUpCursorAxisChord.cursorAxis = CursorAxis::Y;
         GamepadAxisChord lookUpGamepadAxisChord;
         lookUpGamepadAxisChord.axis = GamepadAxis::RightY;
         lookUpGamepadAxisChord.gamepadId = 0;
         inputManager.createAxisMapping("LookUp", nullptr, &lookUpCursorAxisChord, &lookUpGamepadAxisChord);

         CursorAxisChord lookRightCursorAxisChord;
         lookRightCursorAxisChord.cursorAxis = CursorAxis::X;
         GamepadAxisChord lookRightGamepadAxisChord;
         lookRightGamepadAxisChord.axis = GamepadAxis::RightX;
         lookRightGamepadAxisChord.gamepadId = 0;
         inputManager.createAxisMapping("LookRight", nullptr, &lookRightCursorAxisChord, &lookRightGamepadAxisChord);
      }

      {
         static const float kLookSpeed = 3.0f;

         inputManager.bindAxisMapping("LookUp", [&scene](float value)
         {
            if (CameraComponent* activeCameraComponent = scene.getActiveCameraComponent())
            {
               activeCameraComponent->rotate(0.0f, -value * scene.getDeltaTime() * kLookSpeed);
            }
         });

         inputManager.bindAxisMapping("LookRight", [&scene](float value)
         {
            if (CameraComponent* activeCameraComponent = scene.getActiveCameraComponent())
            {
               activeCameraComponent->rotate(value * scene.getDeltaTime() * kLookSpeed, 0.0f);
            }
         });
      }

      {
         static const float kMoveSpeed = 20.0f;

         inputManager.bindAxisMapping("MoveForward", [&scene](float value)
         {
            if (CameraComponent* activeCameraComponent = scene.getActiveCameraComponent())
            {
               activeCameraComponent->moveForward(value * scene.getDeltaTime() * kMoveSpeed);
            }
         });

         inputManager.bindAxisMapping("MoveRight", [&scene](float value)
         {
            if (CameraComponent* activeCameraComponent = scene.getActiveCameraComponent())
            {
               activeCameraComponent->moveRight(value * scene.getDeltaTime() * kMoveSpeed);
            }
         });

         inputManager.bindAxisMapping("MoveUp", [&scene](float value)
         {
            if (CameraComponent* activeCameraComponent = scene.getActiveCameraComponent())
            {
               activeCameraComponent->moveUp(value * scene.getDeltaTime() * kMoveSpeed);
            }
         });
      }
   }

   void loadTestScene(ResourceManager& resourceManager, Scene& scene)
   {
      // Camera
      {
         Entity* cameraEntity = scene.createEntity<CameraComponent>();

         CameraComponent* cameraComponent = cameraEntity->getComponentByClass<CameraComponent>();
         cameraComponent->setRelativePosition(glm::vec3(0.0f, 0.0f, 2.0f));
         cameraComponent->makeActiveCamera();
      }

      // Bunnies
      {
         ModelSpecification modelSpecification;
         IOUtils::getAbsoluteResourcePath("Meshes/Bunny.obj", modelSpecification.path);
         Model bunnyModel = resourceManager.loadModel(modelSpecification);
         bunnyModel.setMaterialParameter("uMaterial.emissiveColor", glm::vec3(0.0f));
         bunnyModel.setMaterialParameter("uMaterial.diffuseColor", glm::vec3(1.0f));
         bunnyModel.setMaterialParameter("uMaterial.specularColor", glm::vec3(1.0f));
         bunnyModel.setMaterialParameter("uMaterial.shininess", 50.0f);

         Entity* bunnyEntity = scene.createEntity<ModelComponent>();

         ModelComponent* bunnyModelComponent = bunnyEntity->getComponentByClass<ModelComponent>();
         bunnyModel.setMaterialParameter("uMaterial.emissiveColor", glm::vec3(1.0f, 0.0f, 0.0f));
         bunnyModelComponent->setModel(bunnyModel);
         bunnyModelComponent->setRelativePosition(glm::vec3(0.25f, -1.0f, 0.0f));
         bunnyModelComponent->setRelativeScale(glm::vec3(10.0f));

         ModelComponent* bunnyModelComponent2 = bunnyEntity->createComponent<ModelComponent>();
         bunnyModelComponent2->setParent(bunnyModelComponent);
         bunnyModel.setMaterialParameter("uMaterial.emissiveColor", glm::vec3(0.0f, 1.0f, 0.0f));
         bunnyModelComponent2->setModel(bunnyModel);
         bunnyModelComponent2->setRelativePosition(glm::vec3(0.15f, 0.0f, 0.0f));

         ModelComponent* bunnyModelComponent3 = bunnyEntity->createComponent<ModelComponent>();
         bunnyModelComponent3->setParent(bunnyModelComponent);
         bunnyModel.setMaterialParameter("uMaterial.emissiveColor", glm::vec3(0.0f, 0.0f, 1.0f));
         bunnyModelComponent3->setModel(bunnyModel);
         bunnyModelComponent3->setRelativePosition(glm::vec3(-0.15f, 0.0f, 0.0f));
      }

      // Directional light
      {
         Entity* directionalLightEntity = scene.createEntity<DirectionalLightComponent>();

         DirectionalLightComponent* directionalLightComponent =
            directionalLightEntity->getComponentByClass<DirectionalLightComponent>();
         directionalLightComponent->setColor(glm::vec3(0.1f));
         directionalLightComponent->setRelativeOrientation(glm::angleAxis(glm::radians(-60.0f),
                                                                          MathUtils::kRightVector));
      }

      // Point light
      {
         Entity* pointLightEntity = scene.createEntity<PointLightComponent, ModelComponent>();

         PointLightComponent* pointLightComponent = pointLightEntity->getComponentByClass<PointLightComponent>();
         pointLightComponent->setColor(glm::vec3(0.12f, 0.83f, 0.91f));
         pointLightComponent->setRadius(20.0f);
         pointLightComponent->setTickFunction([](Component* component, float dt)
         {
            SceneComponent* sceneComponent = static_cast<SceneComponent*>(component);

            float time = sceneComponent->getScene().getTime();
            float r = 1.5f;
            float phi = std::fmod(time, 2.0f * glm::pi<float>());
            float theta = std::fmod(time * 0.7f, 2.0f * glm::pi<float>());
            sceneComponent->setRelativePosition(glm::vec3(r * glm::sin(phi) * glm::cos(theta),
                                                          r * glm::sin(phi) * glm::sin(theta),
                                                          r * glm::cos(phi)));
         });

         ModelSpecification sphereModelSpecification;
         IOUtils::getAbsoluteResourcePath("Meshes/Sphere.obj", sphereModelSpecification.path);
         Model sphereModel = resourceManager.loadModel(sphereModelSpecification);
         sphereModel.setMaterialParameter("uMaterial.emissiveColor", pointLightComponent->getColor());

         ModelComponent* modelComponent = pointLightEntity->getComponentByClass<ModelComponent>();
         modelComponent->setParent(pointLightComponent);
         modelComponent->setModel(std::move(sphereModel));
         modelComponent->setRelativeScale(glm::vec3(0.125f));
      }

      // Spot light
      {
         Entity* spotLightEntity = scene.createEntity<SpotLightComponent, ModelComponent>();

         SpotLightComponent* spotLightComponent = spotLightEntity->getComponentByClass<SpotLightComponent>();
         spotLightComponent->setColor(glm::vec3(0.93f, 0.22f, 0.60f));
         spotLightComponent->setRadius(30.0f);
         spotLightComponent->setRelativeOrientation(glm::angleAxis(glm::radians(35.0f), MathUtils::kRightVector));
         spotLightComponent->setRelativePosition(glm::vec3(-0.5f, -0.75f, 1.0f));
         spotLightComponent->setBeamAngle(5.0f);
         spotLightComponent->setCutoffAngle(15.0f);

         spotLightComponent->setTickFunction([](Component* component, float dt)
         {
            SceneComponent* sceneComponent = static_cast<SceneComponent*>(component);

            float pitchMultiplier = glm::sin(component->getScene().getTime()) + 1.0f;
            float yawMultiplier = glm::sin(component->getScene().getTime() * 0.7f) * 2.0f;

            glm::quat pitch = glm::angleAxis(glm::radians(35.0f) * pitchMultiplier, MathUtils::kRightVector);
            glm::quat yaw = glm::angleAxis(glm::radians(-35.0f) * yawMultiplier, MathUtils::kUpVector);
            sceneComponent->setRelativeOrientation(pitch * yaw);
         });

         ModelSpecification coneModelSpecification;
         IOUtils::getAbsoluteResourcePath("Meshes/Cone.obj", coneModelSpecification.path);
         Model coneModel = resourceManager.loadModel(coneModelSpecification);
         coneModel.setMaterialParameter("uMaterial.emissiveColor", spotLightComponent->getColor());

         ModelComponent* modelComponent = spotLightEntity->getComponentByClass<ModelComponent>();
         modelComponent->setParent(spotLightComponent);
         modelComponent->setModel(std::move(coneModel));
         modelComponent->setRelativeScale(glm::vec3(0.125f));
      }
   }
}

int main(int argc, char* argv[])
{
#if SWAP_DEBUG
   glfwSetErrorCallback(glfwErrorCallback);
   gladSetGLPostCallback(gladPostCallback);
#endif // SWAP_DEBUG

   if (!glfwInit())
   {
      LOG_ERROR_MSG_BOX("Unable to initialize GLFW");
      return 1;
   }

   int returnCode = 0;
   if (UPtr<Window> window = createWindow())
   {
      SPtr<ResourceManager> resourceManager = std::make_shared<ResourceManager>();
      Scene scene;

      auto createSceneRenderer = [&](bool deferred) -> UPtr<SceneRenderer>
      {
         if (deferred)
         {
            return std::make_unique<DeferredSceneRenderer>(resourceManager);
         }
         else
         {
            return std::make_unique<ForwardSceneRenderer>(kNumSamples, resourceManager);
         }
      };

      bool rendererIsDeferred = false;
      UPtr<SceneRenderer> sceneRenderer = createSceneRenderer(rendererIsDeferred);
      window->setTitle("Swap: Forward");

      KeyChord swapRendererKeyChord;
      swapRendererKeyChord.key = Key::Space;
      window->getInputManager().createButtonMapping("SwapRenderer", &swapRendererKeyChord, nullptr, nullptr);
      window->getInputManager().bindButtonMapping("SwapRenderer", [&](bool pressed)
      {
         if (pressed)
         {
            rendererIsDeferred = !rendererIsDeferred;
            sceneRenderer = createSceneRenderer(rendererIsDeferred);

            window->setTitle(rendererIsDeferred ? "Swap: Deferred" : "Swap: Forward");
         }
      });

      window->bindOnFramebufferSizeChanged([&sceneRenderer](int width, int height)
      {
         sceneRenderer->onFramebufferSizeChanged(width, height);
      });

      window->bindOnWindowRefreshRequested([&scene, &sceneRenderer](Window& win)
      {
         sceneRenderer->renderScene(scene);
         win.swapBuffers();
      });

#if SWAP_DEBUG
      window->bindOnWindowFocusChanged([resourceManager](bool focused)
      {
         if (focused)
         {
            resourceManager->reloadShaders();
         }
      });
#endif // SWAP_DEBUG

      bindInputs(window->getInputManager(), scene);
      loadTestScene(*resourceManager, scene);

      double lastTime = glfwGetTime();
      while (!window->shouldClose())
      {
         static const double kMaxFrameTime = 0.25;

         double now = glfwGetTime();
         double frameTime = std::min(now - lastTime, kMaxFrameTime);
         lastTime = now;

         scene.tick(static_cast<float>(frameTime));

         sceneRenderer->renderScene(scene);

         window->swapBuffers();
         window->pollEvents();
      }

#if SWAP_DEBUG
      window->bindOnWindowFocusChanged(nullptr);
#endif // SWAP_DEBUG
   }
   else
   {
      returnCode = 1;
   }

   glfwTerminate();
   return returnCode;
}

#if SWAP_USE_WINMAIN
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
   return main(__argc, __argv);
}
#endif // SWAP_USE_WINMAIN
