target_sources(${PROJECT_NAME} PUBLIC
   "${SRC_DIR}/Assert.h"
   "${SRC_DIR}/Delegate.h"
   "${SRC_DIR}/Delegate.cpp"
   "${SRC_DIR}/Log.h"
   "${SRC_DIR}/Main.cpp"
   "${SRC_DIR}/Pointers.h"

   "${SRC_DIR}/Platform/IOUtils.h"
   "${SRC_DIR}/Platform/IOUtils.cpp"
   "${SRC_DIR}/Platform/OSUtils.h"
   "${SRC_DIR}/Platform/OSUtils.cpp"
)

target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")

get_target_property(SOURCE_FILES ${PROJECT_NAME} SOURCES)
source_group(TREE "${SRC_DIR}" PREFIX Source FILES ${SOURCE_FILES})
