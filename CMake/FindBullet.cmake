
SET(BULLET_SEARCH_PATHS
	${POLYCODE_RELEASE_DIR}/Framework/Core/Dependencies/lib
	${POLYCODE_RELEASE_DIR}/Framework/Modules/Dependencies/lib
	${POLYCODE_RELEASE_DIR}/Framework/Modules/Dependencies/include/bullet
	${POLYCODE_RELEASE_DIR}/Framework/Tools/Dependencies/lib
)

# - Try to find Bullet
# Once done this will define
#
#  BULLET_FOUND - system has bullet
#  BULLET_INCLUDE_DIR - the bullet include directory
#  BULLET_LIBRARIES - Link these to use Collada DOM
#

  SET(BULLETDYNAMICS "BulletDynamics")
  SET(BULLETCOLLISION "BulletCollision")
  SET(BULLETMATH "LinearMath")
  SET(BULLETSOFTBODY "BulletSoftBody")
  SET(BULLETMULTITHREADED "BulletMultiThreaded")

FIND_PATH(BULLET_INCLUDE_DIR NAMES btBulletCollisionCommon.h
	HINTS
	NO_DEFAULT_PATH
	NO_CMAKE_ENVIRONMENT_PATH
	NO_CMAKE_SYSTEM_PATH
	NO_SYSTEM_ENVIRONMENT_PATH
	NO_CMAKE_PATH
	CMAKE_FIND_FRAMEWORK NEVER
	PATHS ${BULLET_SEARCH_PATHS}
)

FIND_LIBRARY(LIBBULLETDYNAMICS
  NAMES 
  ${BULLETDYNAMICS}
	HINTS
	NO_DEFAULT_PATH
	NO_CMAKE_ENVIRONMENT_PATH
	NO_CMAKE_SYSTEM_PATH
	NO_SYSTEM_ENVIRONMENT_PATH
	NO_CMAKE_PATH
	CMAKE_FIND_FRAMEWORK NEVER
	$ENV{BULLETDIR}
	$ENV{BULLET_PATH}
	PATH_SUFFIXES lib lib64 win32/Dynamic_Release "Win32/${MSVC_YEAR_NAME}/x64/Release" "Win32/${MSVC_YEAR_NAME}/Win32/Release"
	PATHS ${BULLET_SEARCH_PATHS}
)

IF(NOT LIBBULLETDYNAMICS)
    MESSAGE ("WARNING: Could not find Bullet Dynamics - depending targets will be disabled.")
ENDIF(NOT LIBBULLETDYNAMICS)


FIND_LIBRARY(LIBBULLETCOLLISION
  NAMES 
  ${BULLETCOLLISION}
	HINTS
	NO_DEFAULT_PATH
	NO_CMAKE_ENVIRONMENT_PATH
	NO_CMAKE_SYSTEM_PATH
	NO_SYSTEM_ENVIRONMENT_PATH
	NO_CMAKE_PATH
	CMAKE_FIND_FRAMEWORK NEVER
	$ENV{BULLETDIR}
	$ENV{BULLET_PATH}
	PATH_SUFFIXES lib lib64 win32/Dynamic_Release "Win32/${MSVC_YEAR_NAME}/x64/Release" "Win32/${MSVC_YEAR_NAME}/Win32/Release"
	PATHS ${BULLET_SEARCH_PATHS}
)

IF(NOT LIBBULLETCOLLISION)
    MESSAGE ("WARNING: Could not find Bullet Collision - depending targets will be disabled.")
ENDIF(NOT LIBBULLETCOLLISION)

FIND_LIBRARY(LIBBULLETMATH
  NAMES 
  ${BULLETMATH}
	HINTS
	NO_DEFAULT_PATH
	NO_CMAKE_ENVIRONMENT_PATH
	NO_CMAKE_SYSTEM_PATH
	NO_SYSTEM_ENVIRONMENT_PATH
	NO_CMAKE_PATH
	CMAKE_FIND_FRAMEWORK NEVER
	$ENV{BULLETDIR}
	$ENV{BULLET_PATH}
	PATH_SUFFIXES lib lib64 win32/Dynamic_Release "Win32/${MSVC_YEAR_NAME}/x64/Release" "Win32/${MSVC_YEAR_NAME}/Win32/Release"
	PATHS ${BULLET_SEARCH_PATHS}
)

IF(NOT LIBBULLETMATH)
    MESSAGE ("WARNING: Could not find Bullet Math - depending targets will be disabled.")
ENDIF(NOT LIBBULLETMATH)

FIND_LIBRARY(LIBBULLETSOFTBODY
  NAMES 
  ${BULLETSOFTBODY}
	HINTS
	NO_DEFAULT_PATH
	NO_CMAKE_ENVIRONMENT_PATH
	NO_CMAKE_SYSTEM_PATH
	NO_SYSTEM_ENVIRONMENT_PATH
	NO_CMAKE_PATH
	CMAKE_FIND_FRAMEWORK NEVER
	$ENV{BULLETDIR}
	$ENV{BULLET_PATH}
	PATH_SUFFIXES lib lib64 win32/Dynamic_Release "Win32/${MSVC_YEAR_NAME}/x64/Release" "Win32/${MSVC_YEAR_NAME}/Win32/Release"
	PATHS ${BULLET_SEARCH_PATHS}
)

IF(NOT LIBBULLETSOFTBODY)
    MESSAGE ("WARNING: Could not find Bullet SoftBody - depending targets will be disabled.")
ENDIF(NOT LIBBULLETSOFTBODY)


FIND_LIBRARY(LIBBULLETMULTITHREADED
  NAMES 
  ${BULLETMULTITHREADED}
	HINTS
	NO_DEFAULT_PATH
	NO_CMAKE_ENVIRONMENT_PATH
	NO_CMAKE_SYSTEM_PATH
	NO_SYSTEM_ENVIRONMENT_PATH
	NO_CMAKE_PATH
	CMAKE_FIND_FRAMEWORK NEVER
	$ENV{BULLETDIR}
	$ENV{BULLET_PATH}
	PATH_SUFFIXES lib lib64 win32/Dynamic_Release "Win32/${MSVC_YEAR_NAME}/x64/Release" "Win32/${MSVC_YEAR_NAME}/Win32/Release"
	PATHS ${BULLET_SEARCH_PATHS}
)

IF(NOT LIBBULLETMULTITHREADED)
    MESSAGE ("WARNING: Could not find Bullet MultiThreaded - depending targets will be disabled.")
ENDIF(NOT LIBBULLETMULTITHREADED)

SET(BULLET_LIBRARIES ${LIBBULLETMULTITHREADED} ${LIBBULLETSOFTBODY} ${LIBBULLETDYNAMICS} ${LIBBULLETCOLLISION} ${LIBBULLETMATH})

IF(BULLET_INCLUDE_DIR AND BULLET_LIBRARIES)
  SET(BULLET_FOUND TRUE)
ENDIF(BULLET_INCLUDE_DIR AND BULLET_LIBRARIES)

# show the BULLET_INCLUDE_DIR and BULLET_LIBRARIES variables only in the advanced view
IF(BULLET_FOUND)
  MESSAGE ("Bullet found (${BULLET_INCLUDE_DIR}) (${BULLET_LIBRARIES})")
  MARK_AS_ADVANCED(BULLET_INCLUDE_DIR BULLET_LIBRARIES )
ENDIF(BULLET_FOUND)
