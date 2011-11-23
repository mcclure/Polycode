# Build a local version of zlib and libpng
INCLUDE(ExternalProject)

SET(libpng_PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libpng)

SET(libpng_CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR> 
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DPNG_SHARED=OFF
    -DBUILD_SHARED_LIBS=FALSE
    -DSKIP_INSTALL_FILES=1
)

EXTERNALPROJECT_ADD(zlib
    PREFIX ${libpng_PREFIX}

    DOWNLOAD_DIR ${POLYCODE_DEPS_DOWNLOAD_DIR}
    URL http://zlib.net/zlib-1.2.5.tar.gz
    URL_MD5 c735eab2d659a96e5a594c9e8541ad63

    PATCH_COMMAND ${CMAKE_COMMAND} -E remove <SOURCE_DIR>/zconf.h

    INSTALL_DIR ${POLYCODE_DEPS_CORE_PREFIX}
    CMAKE_ARGS ${libpng_CMAKE_ARGS}
)

ExternalProject_Get_Property(zlib install_dir)

EXTERNALPROJECT_ADD(libpng
    DEPENDS zlib
    PREFIX ${libpng_PREFIX}

    DOWNLOAD_DIR ${POLYCODE_DEPS_DOWNLOAD_DIR}
    URL ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng-1.5.6.tar.gz
    URL_MD5 8b0c05ed12637ee1f060ddfbbf526ea3

    INSTALL_DIR ${POLYCODE_DEPS_CORE_PREFIX}
    CMAKE_ARGS ${libpng_CMAKE_ARGS} -DCMAKE_PREFIX_PATH=${install_dir} # to find zlib
)
