target_sources(${PROJECT_NAME} PUBLIC
   "${SRC_DIR}/Assert.h"
   "${SRC_DIR}/Log.h"
   "${SRC_DIR}/Main.cpp"
   "${SRC_DIR}/Pointers.h"
)

target_include_directories(${PROJECT_NAME} PUBLIC "${SRC_DIR}")

get_target_property(SOURCE_FILES ${PROJECT_NAME} SOURCES)
source_group(TREE "${SRC_DIR}" PREFIX Source FILES ${SOURCE_FILES})
