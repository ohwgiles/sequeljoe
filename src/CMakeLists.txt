cmake_minimum_required(VERSION 2.8)
project(sequeljoe)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)

find_path(LIBSSH_INCLUDE_DIR NAMES libssh2.h)
find_library(LIBSSH_LIBRARY NAMES ssh2 libssh2)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions("-Wall -Werror -g -std=c++11")
qt5_add_resources(SRC_RESOURCES icons.qrc)

configure_file(../osxbundle.sh.in osxbundle.sh @ONLY)

set(SOURCES main.cpp mainwindow.cpp dbconnection.cpp
sshthread.cpp mainpanel.cpp connectionwidget.cpp filteredpagedtableview.cpp
favourites.cpp viewtoolbar.cpp tablecell.cpp tableview.cpp tabwidget.cpp
querypanel.cpp sqlmodel.cpp sqlhighlighter.cpp schemamodel.cpp
tablelist.cpp passkeywidget.cpp querylog.cpp loadingoverlay.cpp
dbfilewidget.cpp schemaview.cpp
textcelleditor.cpp driver.cpp tablemodel.cpp)

if(APPLE OR WIN32)
    set(exe "SequelJoe")
else(LINUX)
    set(exe "sequeljoe")
endif(APPLE OR WIN32)


if(APPLE)
    set(MACOSX_BUNDLE_ICON_FILE joe.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/joe.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(SOURCES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/joe.icns)

    find_library(FOUNDATION_LIBRARY Foundation)
    set(SOURCES ${SOURCES} notify_osx.mm)
    set_source_files_properties(notify_osx.mm PROPERTIES COMPILE_FLAGS "-ObjC++")
    set(EXTRA_LIBS ${EXTRA_LIBS} ${FOUNDATION_LIBRARY})
elseif(UNIX)
    # linux
    # libnotify depends on glib
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB2 REQUIRED glib-2.0)
    pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
    include_directories(${GLIB2_INCLUDE_DIRS} ${LIBNOTIFY_INCLUDE_DIRS})
    set(SOURCES ${SOURCES} notify_libnotify.cpp)
    set(EXTRA_LIBS ${EXTRA_LIBS} ${LIBNOTIFY_LIBRARIES})
endif(APPLE)

add_executable(${exe} MACOSX_BUNDLE WIN32 ${SOURCES} ${SRC_RESOURCES})
target_link_libraries(${exe} ${LIBSSH_LIBRARY} ${EXTRA_LIBS})
qt5_use_modules(${exe} Widgets Sql)

install(TARGETS ${exe}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
)

# todo install plugins, set rpaths
