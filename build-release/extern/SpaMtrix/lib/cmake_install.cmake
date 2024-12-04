# Install script for directory: C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/project-qlc3d")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set path to fallback-tool for dependency-resolution.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "C:/mingw64/bin/objdump.exe")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/build-release/extern/SpaMtrix/lib/libSpaMtrix.a")
endif()

if(CMAKE_INSTALL_COMPONENT STREQUAL "Unspecified" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_cholesky.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_cholincpreconditioner.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_densematrix.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_diagpreconditioner.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_fleximatrix.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_ircmatrix.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_iterativesolvers.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_lu.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_luincpreconditioner.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_matrixmaker.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_powermethod.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_reader.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_blas.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_tdmatrix.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_vector.hpp"
    "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/extern/SpaMtrix/lib/include/spamtrix_writer.hpp"
    )
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
if(CMAKE_INSTALL_LOCAL_ONLY)
  file(WRITE "C:/Users/Public/Public_desktop/qlc3d_new/qlc3d/build-release/extern/SpaMtrix/lib/install_local_manifest.txt"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
endif()
