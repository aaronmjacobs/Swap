target_sources(${PROJECT_NAME} PRIVATE
   "${SRC_DIR}/Main.cpp"

   "${SRC_DIR}/Core/Assert.h"
   "${SRC_DIR}/Core/Delegate.h"
   "${SRC_DIR}/Core/Delegate.cpp"
   "${SRC_DIR}/Core/Log.h"
   "${SRC_DIR}/Core/Pointers.h"

   "${SRC_DIR}/Platform/IOUtils.h"
   "${SRC_DIR}/Platform/IOUtils.cpp"
   "${SRC_DIR}/Platform/OSUtils.h"
   "${SRC_DIR}/Platform/OSUtils.cpp"
   "${SRC_DIR}/Platform/Window.h"
   "${SRC_DIR}/Platform/Window.cpp"
)

target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")

get_target_property(SOURCE_FILES ${PROJECT_NAME} SOURCES)
source_group(TREE "${SRC_DIR}" PREFIX Source FILES ${SOURCE_FILES})
