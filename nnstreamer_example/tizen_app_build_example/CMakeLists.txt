CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(nnstreamer_testapp C)

OPTION(INSTALL_EXAMPLE_APP "Install example applications" ON)

IF(INSTALL_EXAMPLE_APP)
	IF(EXAMPLE_EXEC_PREFIX)
		MESSAGE("Example application will be installed to ${EXAMPLE_EXEC_PREFIX}")
	ELSE(EXAMPLE_EXEC_PREFIX)
		SET(EXAMPLE_EXEC_PREFIX /usr/lib/nnstreamer/bin/)
	ENDIF(EXAMPLE_EXEC_PREFIX)
ENDIF(INSTALL_EXAMPLE_APP)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "${PREFIX}/bin")
SET(INCLUDEDIR "${PREFIX}/include/nnstreamer")

IF(LIB_INSTALL_DIR)
ELSE(LIB_INSTALL_DIR)
    SET(LIB_INSTALL_DIR "${PREFIX}/lib")
ENDIF(LIB_INSTALL_DIR)

FIND_PACKAGE(PkgConfig REQUIRED)
SET(PKGAPP_MODULES
	nnstreamer
	gstreamer-1.0
	gstreamer-base-1.0
	gstreamer-controller-1.0
	gstreamer-video-1.0
	gstreamer-audio-1.0
	gstreamer-app-1.0
)
pkg_check_modules(apppkgs REQUIRED ${PKGAPP_MODULES})

ADD_SUBDIRECTORY(../example_sink ${CMAKE_CURRENT_BINARY_DIR}/example_sink)
ADD_SUBDIRECTORY(../example_cam ${CMAKE_CURRENT_BINARY_DIR}/example_cam)
ADD_SUBDIRECTORY(../example_filter ${CMAKE_CURRENT_BINARY_DIR}/example_filter)
