cmake_minimum_required(VERSION 2.8)
project(sequeljoe)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)

find_path(LIBSSH_INCLUDE_DIR NAMES libssh2.h)
find_library(LIBSSH_LIBRARY NAMES ssh2 libssh2)

set(CMAKE_AUTOMOC TRUE)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_definitions("-std=c++11")
qt5_add_resources(SRC_RESOURCES icons.qrc)

set(SOURCES main.cpp mainwindow.cpp sshdbconnection.cpp dbconnection.cpp
sshthread.cpp mainpanel.cpp connectionwidget.cpp filteredpagedtableview.cpp
favourites.cpp viewtoolbar.cpp tablecell.cpp tableview.cpp tabwidget.cpp
querypanel.cpp sqlcontentmodel.cpp sqlhighlighter.cpp sqlschemamodel.cpp
tablelist.cpp passkeywidget.cpp)

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
    set(EXTRA_LIBS ${EXTRA_LIBS} ${FOUNDATION})
elseif(UNIX)
    # linux
    find_library(LIBNOTIFY_LIBRARY NAMES libnotify)
    set(SOURCES ${SOURCES} notify_libnotify.cpp)
    set(EXTRA_LIBS ${EXTRA_LIBS} ${LIBNOTIFY_LIBRARY})
endif(APPLE)

add_executable(${exe} MACOSX_BUNDLE WIN32 ${SOURCES})
target_link_libraries(${exe} ${LIBSSH_LIBRARY} ${FOUNDATION_LIBRARY})
qt5_use_modules(${exe} Widgets Sql)

install(TARGETS ${exe}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
)

# todo install plugins, set rpaths
