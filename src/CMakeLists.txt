cmake_minimum_required(VERSION 2.8)
project(sequeljoe)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

#include(DeployQt5.cmake)

if(APPLE)
set(exe "SequelJoe")
else()
set(exe "sequeljoe")
endif()

add_definitions("-std=c++11")
qt5_add_resources(SRC_RESOURCES icons.qrc)

set(QTTEST_SRCS main.cpp mainwindow.cpp sshdbconnection.cpp dbconnection.cpp sshthread.cpp mainpanel.cpp connectionwidget.cpp favourites.cpp viewtoolbar.cpp tablecell.cpp tableview.cpp tabwidget.cpp querypanel.cpp sqlhighlighter.cpp sqlschemamodel.cpp passkeywidget.cpp)

IF(APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE joe.icns)
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/joe.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  SET(QTTEST_SRCS ${QTTEST_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/joe.icns)

find_library(FOUNDATION Foundation)
#find_path(FOUNDATION_HEADERS Foundation/Foundation.h)
SET(QTTEST_SRCS ${QTTEST_SRCS} notify_osx.mm)
set_source_files_properties(notify_osx.mm PROPERTIES COMPILE_FLAGS "-ObjC++ -I${FOUNDATION}/Headers")
set(EXTRA_LIBS ${FOUNDATION})
#message(${FOUNDATION_HEADERS})
ENDIF(APPLE)

if(UNIX)
    set(QTTEST_SRCS ${QTTEST_SRCS} notify_libnotify.cpp)
endif(UNIX)

add_executable(${exe} MACOSX_BUNDLE WIN32 ${SRC_RESOURCES} ${QTTEST_SRCS} )
target_link_libraries(${exe} ssh2 ${FOUNDATION})
qt5_use_modules(${exe} Widgets Sql)


#fixup_qt5_executable("${CMAKE_BINARY_DIR}/${exe}.app" "qcocoa;qsqlmysql" "" "/usr/local/lib")
INSTALL(TARGETS ${exe}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )

# TODO: bundle plugins!
#SET(plugin_dest_dir bin)
#SET(qtconf_dest_dir bin)
#SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/sequeljoe")
#IF(APPLE)
#  SET(plugin_dest_dir SequelJoe.app/Contents/MacOS)
#  SET(qtconf_dest_dir SequelJoe.app/Contents/Resources)
#  SET(APPS "\${CMAKE_INSTALL_PREFIX}/SequelJoe.app")
#ENDIF(APPLE)
#IF(WIN32)
#  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/SequelJoe.exe")
#ENDIF(WIN32)

