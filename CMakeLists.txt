INCLUDE_DIRECTORIES("/usr/local/include")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/Dropbox/include")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/memStream/include")

ADD_LIBRARY(dropboxc
  STATIC
  Dropbox/src/dropbox.c
  Dropbox/src/dropboxJson.c
  Dropbox/src/dropboxOAuth.c
  Dropbox/src/dropboxUtils.c
  memStream/src/memStream.c
)

TARGET_LINK_LIBRARIES(dropboxc jansson oauth curl ssh2 ssl crypto z m)

SET_PROPERTY(TARGET dropboxc PROPERTY C_STANDARD 99)
