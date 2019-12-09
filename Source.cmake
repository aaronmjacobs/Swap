target_sources(${PROJECT_NAME} PRIVATE
   "${SRC_DIR}/Main.cpp"

   "${SRC_DIR}/Core/Assert.h"
   "${SRC_DIR}/Core/Delegate.h"
   "${SRC_DIR}/Core/Delegate.cpp"
   "${SRC_DIR}/Core/Hash.h"
   "${SRC_DIR}/Core/Log.h"
   "${SRC_DIR}/Core/Log.cpp"
   "${SRC_DIR}/Core/Pointers.h"

   "${SRC_DIR}/Graphics/BufferObject.h"
   "${SRC_DIR}/Graphics/BufferObject.cpp"
   "${SRC_DIR}/Graphics/DrawingContext.h"
   "${SRC_DIR}/Graphics/ForEachUniformType.inl"
   "${SRC_DIR}/Graphics/Framebuffer.h"
   "${SRC_DIR}/Graphics/Framebuffer.cpp"
   "${SRC_DIR}/Graphics/GraphicsContext.h"
   "${SRC_DIR}/Graphics/GraphicsContext.cpp"
   "${SRC_DIR}/Graphics/GraphicsDefines.h"
   "${SRC_DIR}/Graphics/GraphicsResource.h"
   "${SRC_DIR}/Graphics/GraphicsResource.cpp"
   "${SRC_DIR}/Graphics/Material.h"
   "${SRC_DIR}/Graphics/Material.cpp"
   "${SRC_DIR}/Graphics/MaterialParameter.h"
   "${SRC_DIR}/Graphics/MaterialParameter.cpp"
   "${SRC_DIR}/Graphics/Mesh.h"
   "${SRC_DIR}/Graphics/Mesh.cpp"
   "${SRC_DIR}/Graphics/Model.h"
   "${SRC_DIR}/Graphics/Model.cpp"
   "${SRC_DIR}/Graphics/RasterizerState.h"
   "${SRC_DIR}/Graphics/ResourcePool.h"
   "${SRC_DIR}/Graphics/Shader.h"
   "${SRC_DIR}/Graphics/Shader.cpp"
   "${SRC_DIR}/Graphics/ShaderProgram.h"
   "${SRC_DIR}/Graphics/ShaderProgram.cpp"
   "${SRC_DIR}/Graphics/Texture.h"
   "${SRC_DIR}/Graphics/Texture.cpp"
   "${SRC_DIR}/Graphics/TextureInfo.h"
   "${SRC_DIR}/Graphics/Uniform.h"
   "${SRC_DIR}/Graphics/Uniform.cpp"
   "${SRC_DIR}/Graphics/UniformBufferObject.h"
   "${SRC_DIR}/Graphics/UniformBufferObject.cpp"
   "${SRC_DIR}/Graphics/UniformBufferObjectHelpers.h"
   "${SRC_DIR}/Graphics/UniformTypes.h"
   "${SRC_DIR}/Graphics/Viewport.h"

   "${SRC_DIR}/Math/Bounds.h"
   "${SRC_DIR}/Math/Bounds.cpp"
   "${SRC_DIR}/Math/MathUtils.h"
   "${SRC_DIR}/Math/Transform.h"

   "${SRC_DIR}/Platform/InputManager.h"
   "${SRC_DIR}/Platform/InputManager.cpp"
   "${SRC_DIR}/Platform/InputTypes.h"
   "${SRC_DIR}/Platform/IOUtils.h"
   "${SRC_DIR}/Platform/IOUtils.cpp"
   "${SRC_DIR}/Platform/OSUtils.h"
   "${SRC_DIR}/Platform/OSUtils.cpp"
   "${SRC_DIR}/Platform/Window.h"
   "${SRC_DIR}/Platform/Window.cpp"

   "${SRC_DIR}/Resources/DefaultImageSource.h"
   "${SRC_DIR}/Resources/ModelLoader.h"
   "${SRC_DIR}/Resources/ModelLoader.cpp"
   "${SRC_DIR}/Resources/ResourceManager.h"
   "${SRC_DIR}/Resources/ResourceManager.cpp"
   "${SRC_DIR}/Resources/ShaderLoader.h"
   "${SRC_DIR}/Resources/ShaderLoader.cpp"
   "${SRC_DIR}/Resources/TextureLoader.h"
   "${SRC_DIR}/Resources/TextureLoader.cpp"

   "${SRC_DIR}/Scene/Components/CameraComponent.h"
   "${SRC_DIR}/Scene/Components/CameraComponent.cpp"
   "${SRC_DIR}/Scene/Components/Component.h"
   "${SRC_DIR}/Scene/Components/Component.cpp"
   "${SRC_DIR}/Scene/Components/Lights/DirectionalLightComponent.h"
   "${SRC_DIR}/Scene/Components/Lights/DirectionalLightComponent.cpp"
   "${SRC_DIR}/Scene/Components/Lights/LightComponent.h"
   "${SRC_DIR}/Scene/Components/Lights/PointLightComponent.h"
   "${SRC_DIR}/Scene/Components/Lights/PointLightComponent.cpp"
   "${SRC_DIR}/Scene/Components/Lights/SpotLightComponent.h"
   "${SRC_DIR}/Scene/Components/Lights/SpotLightComponent.cpp"
   "${SRC_DIR}/Scene/Components/ModelComponent.h"
   "${SRC_DIR}/Scene/Components/ModelComponent.cpp"
   "${SRC_DIR}/Scene/Components/SceneComponent.h"
   "${SRC_DIR}/Scene/Components/SceneComponent.cpp"
   "${SRC_DIR}/Scene/Entity.h"
   "${SRC_DIR}/Scene/Entity.cpp"
   "${SRC_DIR}/Scene/Rendering/DeferredSceneRenderer.h"
   "${SRC_DIR}/Scene/Rendering/DeferredSceneRenderer.cpp"
   "${SRC_DIR}/Scene/Rendering/ForwardSceneRenderer.h"
   "${SRC_DIR}/Scene/Rendering/ForwardSceneRenderer.cpp"
   "${SRC_DIR}/Scene/Rendering/SceneRenderer.h"
   "${SRC_DIR}/Scene/Rendering/SceneRenderer.cpp"
   "${SRC_DIR}/Scene/Scene.h"
   "${SRC_DIR}/Scene/Scene.cpp"
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
   target_sources(${PROJECT_NAME} PRIVATE "${SRC_DIR}/Platform/MacOSUtils.mm")
endif()

target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")

get_target_property(SOURCE_FILES ${PROJECT_NAME} SOURCES)
source_group(TREE "${SRC_DIR}" PREFIX Source FILES ${SOURCE_FILES})
