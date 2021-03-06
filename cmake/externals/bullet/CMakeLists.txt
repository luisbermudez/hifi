set(EXTERNAL_NAME bullet)

if (WIN32)
  set(PLATFORM_CMAKE_ARGS "-DUSE_MSVC_RUNTIME_LIBRARY_DLL=1")
else ()
  set(PLATFORM_CMAKE_ARGS "-DBUILD_SHARED_LIBS=1")
  
  if (ANDROID)
    list(APPEND PLATFORM_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}" "-DANDROID_NATIVE_API_LEVEL=19")
  elseif (APPLE)
    list(APPEND PLATFORM_CMAKE_ARGS "-DCMAKE_INSTALL_NAME_DIR=<INSTALL_DIR>/lib")
  endif()
endif ()

include(ExternalProject)

if (WIN32)  
  ExternalProject_Add(
    ${EXTERNAL_NAME}
    # URL https://bullet.googlecode.com/files/bullet-2.82-r2704.zip
    URL http://hifi-public.s3.amazonaws.com/dependencies/bullet-2.82-ccd-fix.zip
    URL_MD5 d95b07eb120de7dd7786361c0b5a8d9f
    CMAKE_ARGS ${PLATFORM_CMAKE_ARGS} -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_EXTRAS=0 -DINSTALL_LIBS=1 -DBUILD_DEMOS=0 -DUSE_GLUT=0 -DUSE_DX11=0
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    BINARY_DIR ${EXTERNAL_PROJECT_PREFIX}/build
  )  
else ()
  ExternalProject_Add(
    ${EXTERNAL_NAME}
    #URL http://bullet.googlecode.com/files/bullet-2.82-r2704.tgz
    URL http://hifi-public.s3.amazonaws.com/dependencies/bullet-2.82-ccd-fix.tgz
    URL_MD5 fb140a4983b4109aa1c825a162aa8d64
    CMAKE_ARGS ${PLATFORM_CMAKE_ARGS} -DCMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> -DBUILD_EXTRAS=0 -DINSTALL_LIBS=1 -DBUILD_DEMOS=0 -DUSE_GLUT=0
    LOG_DOWNLOAD 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
    BINARY_DIR ${EXTERNAL_PROJECT_PREFIX}/build
  )  
endif ()

# Hide this external target (for ide users)
set_target_properties(${EXTERNAL_NAME} PROPERTIES FOLDER "hidden/externals")

ExternalProject_Get_Property(${EXTERNAL_NAME} INSTALL_DIR)

string(TOUPPER ${EXTERNAL_NAME} EXTERNAL_NAME_UPPER)

set(BULLET_LIB_DIR "${INSTALL_DIR}/lib")

if (APPLE OR UNIX OR ANDROID)
  if (APPLE)
    set(BULLET_LIB_EXT "dylib")
  else ()
    set(BULLET_LIB_EXT "so")
  endif ()
  
  set(LIB_PREFIX "lib")
elseif (WIN32)
  set(BULLET_LIB_EXT "lib")
endif ()

if (DEFINED BULLET_LIB_EXT)
  set(_BULLET_LIB_PAIRS "DYNAMICS_LIBRARY\;BulletDynamics" "COLLISION_LIBRARY\;BulletCollision" "MATH_LIBRARY\;LinearMath" "SOFTBODY_LIBRARY\;BulletSoftBody")
  
  foreach(_LIB_PAIR ${_BULLET_LIB_PAIRS})
    list(GET _LIB_PAIR 0 _LIB_VAR_NAME)
    list(GET _LIB_PAIR 1 _LIB_NAME)
    
    set(${EXTERNAL_NAME_UPPER}_${_LIB_VAR_NAME}_RELEASE ${BULLET_LIB_DIR}/${LIB_PREFIX}${_LIB_NAME}.${BULLET_LIB_EXT} CACHE FILEPATH "${_LIB_NAME} release library location")
    
    if (WIN32)
      set(${EXTERNAL_NAME_UPPER}_${_LIB_VAR_NAME}_DEBUG ${BULLET_LIB_DIR}/${LIB_PREFIX}${_LIB_NAME}_Debug.${BULLET_LIB_EXT} CACHE FILEPATH "${_LIB_NAME} debug library location")
    else ()
      set(${EXTERNAL_NAME_UPPER}_${_LIB_VAR_NAME}_DEBUG "" CACHE FILEPATH "${_LIB_NAME} debug library location")
    endif ()
  endforeach()
endif ()

if (DEFINED ${EXTERNAL_NAME_UPPER}_DYNAMICS_LIBRARY_RELEASE)
  set(${EXTERNAL_NAME_UPPER}_INCLUDE_DIR ${INSTALL_DIR}/include/bullet CACHE PATH "Path to bullet include directory")
endif ()
