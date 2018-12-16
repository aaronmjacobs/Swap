#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Pointers.h"
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
#include "Scene/ForwardSceneRenderer.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

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

   void loadTestScene(ResourceManager& resourceManager, Scene& scene)
   {
      std::vector<ShaderSpecification> shaderSpecifications;
      shaderSpecifications.resize(2);
      shaderSpecifications[0].type = ShaderType::Vertex;
      shaderSpecifications[1].type = ShaderType::Fragment;
      IOUtils::getAbsoluteResourcePath("Forward.vert", shaderSpecifications[0].path);
      IOUtils::getAbsoluteResourcePath("Forward.frag", shaderSpecifications[1].path);

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
         IOUtils::getAbsoluteResourcePath("Bunny.obj", modelSpecification.path);
         modelSpecification.shaderSpecifications = shaderSpecifications;
         SPtr<Model> bunnyModel = resourceManager.loadModel(modelSpecification);
         bunnyModel->setMaterialParameter("uMaterial.diffuseColor", glm::vec3(1.0f), false);
         bunnyModel->setMaterialParameter("uMaterial.specularColor", glm::vec3(1.0f), false);
         bunnyModel->setMaterialParameter("uMaterial.shininess", 50.0f, false);

         Entity* bunnyEntity = scene.createEntity<ModelComponent>();

         ModelComponent* bunnyModelComponent = bunnyEntity->getComponentByClass<ModelComponent>();
         bunnyModelComponent->setModel(bunnyModel);
         bunnyModelComponent->setRelativePosition(glm::vec3(0.25f, -1.0f, 0.0f));
         bunnyModelComponent->setRelativeScale(glm::vec3(10.0f));

         ModelComponent* bunnyModelComponent2 = bunnyEntity->createComponent<ModelComponent>();
         bunnyModelComponent2->setParent(bunnyModelComponent);
         bunnyModelComponent2->setModel(bunnyModel);
         bunnyModelComponent2->setRelativePosition(glm::vec3(0.15f, 0.0f, 0.0f));

         ModelComponent* bunnyModelComponent3 = bunnyEntity->createComponent<ModelComponent>();
         bunnyModelComponent3->setParent(bunnyModelComponent);
         bunnyModelComponent3->setModel(bunnyModel);
         bunnyModelComponent3->setRelativePosition(glm::vec3(-0.15f, 0.0f, 0.0f));
      }

      // Directional light
      {
         Entity* directionalLightEntity = scene.createEntity<DirectionalLightComponent>();

         DirectionalLightComponent* directionalLightComponent = directionalLightEntity->getComponentByClass<DirectionalLightComponent>();
         directionalLightComponent->setColor(glm::vec3(0.25f));
         directionalLightComponent->setRelativeOrientation(glm::angleAxis(glm::radians(-60.0f), MathUtils::kRightVector));
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
            sceneComponent->setRelativePosition(glm::vec3(r * glm::sin(phi) * glm::cos(theta), r * glm::sin(phi) * glm::sin(theta), r * glm::cos(phi)));
         });

         ModelSpecification sphereModelSpecification;
         IOUtils::getAbsoluteResourcePath("Sphere.obj", sphereModelSpecification.path);
         sphereModelSpecification.shaderSpecifications = shaderSpecifications;
         SPtr<Model> sphereModel = resourceManager.loadModel(sphereModelSpecification);
         sphereModel->setMaterialParameter("uMaterial.emissiveColor", pointLightComponent->getColor(), false);

         ModelComponent* modelComponent = pointLightEntity->getComponentByClass<ModelComponent>();
         modelComponent->setParent(pointLightComponent);
         modelComponent->setModel(sphereModel);
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
         IOUtils::getAbsoluteResourcePath("Cone.obj", coneModelSpecification.path);
         coneModelSpecification.shaderSpecifications = shaderSpecifications;
         SPtr<Model> coneModel = resourceManager.loadModel(coneModelSpecification);
         coneModel->setMaterialParameter("uMaterial.emissiveColor", spotLightComponent->getColor(), false);

         ModelComponent* modelComponent = spotLightEntity->getComponentByClass<ModelComponent>();
         modelComponent->setParent(spotLightComponent);
         modelComponent->setModel(coneModel);
         modelComponent->setRelativeScale(glm::vec3(0.125f));
      }
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

      ResourceManager resourceManager;
      Scene scene;
      UPtr<SceneRenderer> sceneRenderer = std::make_unique<ForwardSceneRenderer>(framebufferWidth, framebufferHeight);

      window->bindOnFramebufferSizeChanged([&sceneRenderer](int width, int height)
      {
         sceneRenderer->onFramebufferSizeChanged(width, height);
      });

      window->bindOnWindowRefreshRequested([&scene, &sceneRenderer](Window& win)
      {
         sceneRenderer->renderScene(scene);
         win.swapBuffers();
      });

      window->bindOnWindowFocusChanged([&resourceManager](bool focused)
      {
         if (focused)
         {
            resourceManager.reloadShaders();
         }
      });

      loadTestScene(resourceManager, scene);

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
   }
   else
   {
      returnCode = 1;
   }

   glfwTerminate();
   return returnCode;
}
