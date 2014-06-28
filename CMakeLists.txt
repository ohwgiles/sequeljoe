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

set(QTTEST_SRCS main.cpp mainwindow.cpp sshdbconnection.cpp dbconnection.cpp sshthread.cpp mainpanel.cpp connectionwidget.cpp favourites.cpp viewtoolbar.cpp tableview.cpp tabwidget.cpp querypanel.cpp sqlhighlighter.cpp passkeywidget.cpp)


IF(APPLE)
  # set how it shows up in the Info.plist file
  SET(MACOSX_BUNDLE_ICON_FILE joe.icns)
  # set where in the bundle to put the icns file
  SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/joe.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
  # include the icns file in the target
  SET(QTTEST_SRCS ${QTTEST_SRCS} ${CMAKE_CURRENT_SOURCE_DIR}/joe.icns)
ENDIF(APPLE)

add_executable(${exe} MACOSX_BUNDLE WIN32 ${SRC_RESOURCES} ${QTTEST_SRCS} )
target_link_libraries(${exe} ssh2 )
qt5_use_modules(${exe} Widgets Sql)


#fixup_qt5_executable("${CMAKE_BINARY_DIR}/${exe}.app" "qcocoa;qsqlmysql" "" "/usr/local/lib")
INSTALL(TARGETS ${exe}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
    )
return()

SET(plugin_dest_dir bin)
SET(qtconf_dest_dir bin)
SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/sequeljoe")
IF(APPLE)
  SET(plugin_dest_dir SequelJoe.app/Contents/MacOS)
  SET(qtconf_dest_dir SequelJoe.app/Contents/Resources)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/SequelJoe.app")
ENDIF(APPLE)
IF(WIN32)
  SET(APPS "\${CMAKE_INSTALL_PREFIX}/bin/SequelJoe.exe")
ENDIF(WIN32)


foreach(plugin ${Qt5Sql_PLUGINS})

  get_target_property(_loc ${plugin} LOCATION)
  message("Plugin ${plugin} is at location ${_loc}")
endforeach()

#--------------------------------------------------------------------------------
# Install the QtTest application, on Apple, the bundle is at the root of the
# install tree, and on other platforms it'll go into the bin directory.


#--------------------------------------------------------------------------------
# Install needed Qt plugins by copying directories from the qt installation
# One can cull what gets copied by using 'REGEX "..." EXCLUDE'
# TODO
#INSTALL(DIRECTORY "${QT_PLUGINS_DIR}/imageformats" DESTINATION ${plugin_dest_dir}/plugins COMPONENT Runtime)

#--------------------------------------------------------------------------------
# install a qt.conf file
# this inserts some cmake code into the install script to write the file
INSTALL(CODE "
    file(WRITE \"\${CMAKE_INSTALL_PREFIX}/${qtconf_dest_dir}/qt.conf\" \"\")
    " COMPONENT Runtime)


#--------------------------------------------------------------------------------
# Use BundleUtilities to get all other dependencies for the application to work.
# It takes a bundle or executable along with possible plugins and inspects it
# for dependencies.  If they are not system dependencies, they are copied.

# directories to look for dependencies
SET(DIRS ${Qt5Core_DIR} ${Qt5Gui_DIR} ${Qt5Widgets_DIR})

# Now the work of copying dependencies into the bundle/package
# The quotes are escaped and variables to use at install time have their $ escaped
# An alternative is the do a configure_file() on a script and use install(SCRIPT  ...).
# Note that the image plugins depend on QtSvg and QtXml, and it got those copied
# over.
INSTALL(CODE "
    file(GLOB_RECURSE QTPLUGINS
      \"\${CMAKE_INSTALL_PREFIX}/${plugin_dest_dir}/plugins/*${CMAKE_SHARED_LIBRARY_SUFFIX}\")
    include(BundleUtilities)
    fixup_bundle(\"${APPS}\" \"\${QTPLUGINS}\" \"${DIRS}\")
    " COMPONENT Runtime)


# To Create a package, one can run "cpack -G DragNDrop CPackConfig.cmake" on Mac OS X
# where CPackConfig.cmake is created by including CPack
# And then there's ways to customize this as well
#set(CPACK_BINARY_DRAGNDROP ON)
set(CPACK_GENERATOR "DragNDrop")
#set(CPACK_BUNDLE_PLIST "Info.plist")
#set(CPACK_BUNDLE_ICON "joe.icns")
#set(CPACK_INSTALLED_DIRECTORIES "")
include(CPack)




#set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
#set(CPACK_GENERATOR "Bundle")
#set(CPACK_PACKAGE_NAME ${exe})
#set(CPACK_BUNDLE_NAME "SequelJoe")
#set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-${CMAKE_SYSTEM_PROCESSOR}")

#include(CPack)
#message("qt @ ${QT_LIBRARY_DIR}")
#set(APPS ${CMAKE_BINARY_DIR}/${exe})
#set(DIRS ${Qt5Core_DIR} ${Qt5Gui_DIR} ${Qt5Widgets_DIR})

#install(CODE "
#    include(BundleUtilities)
#    fixup_bundle(\"${APPS}\" \"\" \"${DIRS}\")
#" COMPONENT Runtime)
