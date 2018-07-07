target_sources(${PROJECT_NAME} PRIVATE
   "${SRC_DIR}/Main.cpp"

   "${SRC_DIR}/Core/Assert.h"
   "${SRC_DIR}/Core/Delegate.h"
   "${SRC_DIR}/Core/Delegate.cpp"
   "${SRC_DIR}/Core/Hash.h"
   "${SRC_DIR}/Core/Log.h"
   "${SRC_DIR}/Core/Pointers.h"

   "${SRC_DIR}/Graphics/BufferObject.h"
   "${SRC_DIR}/Graphics/BufferObject.cpp"
   "${SRC_DIR}/Graphics/ForEachUniformType.inl"
   "${SRC_DIR}/Graphics/Material.h"
   "${SRC_DIR}/Graphics/Material.cpp"
   "${SRC_DIR}/Graphics/MaterialParameter.h"
   "${SRC_DIR}/Graphics/MaterialParameter.cpp"
   "${SRC_DIR}/Graphics/Mesh.h"
   "${SRC_DIR}/Graphics/Mesh.cpp"
   "${SRC_DIR}/Graphics/Model.h"
   "${SRC_DIR}/Graphics/Model.cpp"
   "${SRC_DIR}/Graphics/Shader.h"
   "${SRC_DIR}/Graphics/Shader.cpp"
   "${SRC_DIR}/Graphics/ShaderProgram.h"
   "${SRC_DIR}/Graphics/ShaderProgram.cpp"
   "${SRC_DIR}/Graphics/Texture.h"
   "${SRC_DIR}/Graphics/Texture.cpp"
   "${SRC_DIR}/Graphics/TextureInfo.h"
   "${SRC_DIR}/Graphics/Uniform.h"
   "${SRC_DIR}/Graphics/Uniform.cpp"
   "${SRC_DIR}/Graphics/UniformTypes.h"

   "${SRC_DIR}/Platform/IOUtils.h"
   "${SRC_DIR}/Platform/IOUtils.cpp"
   "${SRC_DIR}/Platform/OSUtils.h"
   "${SRC_DIR}/Platform/OSUtils.cpp"
   "${SRC_DIR}/Platform/Window.h"
   "${SRC_DIR}/Platform/Window.cpp"

   "${SRC_DIR}/Resources/DefaultImageSource.h"
   "${SRC_DIR}/Resources/ModelLoader.h"
   "${SRC_DIR}/Resources/ModelLoader.cpp"
   "${SRC_DIR}/Resources/ShaderLoader.h"
   "${SRC_DIR}/Resources/ShaderLoader.cpp"
   "${SRC_DIR}/Resources/TextureLoader.h"
   "${SRC_DIR}/Resources/TextureLoader.cpp"
)

target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")

get_target_property(SOURCE_FILES ${PROJECT_NAME} SOURCES)
source_group(TREE "${SRC_DIR}" PREFIX Source FILES ${SOURCE_FILES})
