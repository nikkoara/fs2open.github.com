
ADD_LIBRARY(glad STATIC
  "glad/include/glad/glad.h"
  "glad/include/KHR/khrplatform.h"
  "glad/src/glad.c")

TARGET_INCLUDE_DIRECTORIES(
  glad SYSTEM PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/glad/include")

SET_TARGET_PROPERTIES(glad PROPERTIES FOLDER "3rdparty")
SUPPRESS_WARNINGS(glad)

ADD_LIBRARY(OpenGL INTERFACE)
TARGET_LINK_LIBRARIES(OpenGL INTERFACE glad)
