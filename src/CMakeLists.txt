cmake_minimum_required(VERSION 2.8)
project(sequeljoe)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -Werror -pedantic")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
add_definitions("-std=c++11")

set(SOURCES
    connectionwidget.cpp
    dbconnection.cpp
    dbfilewidget.cpp
    driver.cpp
    favourites.cpp
    filteredpagedtableview.cpp
    foreignkeyeditor.cpp
    loadingoverlay.cpp
    main.cpp
    mainpanel.cpp
    mainwindow.cpp
    passkeywidget.cpp
    querylog.cpp
    querypanel.cpp
    schemamodel.cpp
    schemaview.cpp
    sqlhighlighter.cpp
    sqlmodel.cpp
    sshthread.cpp
    tablecell.cpp
    tablelist.cpp
    tablemodel.cpp
    tableview.cpp
    tabwidget.cpp
    textcelleditor.cpp
    viewtoolbar.cpp
)

# Qt
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Sql REQUIRED)
set(CMAKE_AUTOMOC TRUE)
qt5_add_resources(SRC_RESOURCES icons.qrc)

# libssh2
find_path(LIBSSH2_INCLUDE_DIR NAMES libssh2.h)
find_library(LIBSSH2_LIBRARY NAMES ssh2 libssh2)

# Platform-specific
if(APPLE)
    set(exe "SequelJoe")
    configure_file(../osxbundle.sh.in osxbundle.sh @ONLY)
    set(MACOSX_BUNDLE_ICON_FILE joe.icns)
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/joe.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set(SOURCES ${SOURCES} ${CMAKE_CURRENT_SOURCE_DIR}/joe.icns)
    find_library(FOUNDATION_LIBRARY Foundation)
    set(SOURCES ${SOURCES} notify_osx.mm)
    set_source_files_properties(notify_osx.mm PROPERTIES COMPILE_FLAGS "-ObjC++")
    set(EXTRA_LIBS ${EXTRA_LIBS} ${FOUNDATION_LIBRARY})

elseif(UNIX)
    set(exe "sequeljoe")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GLIB2 REQUIRED glib-2.0)
    pkg_check_modules(LIBNOTIFY REQUIRED libnotify)
    include_directories(${GLIB2_INCLUDE_DIRS} ${LIBNOTIFY_INCLUDE_DIRS})
    set(SOURCES ${SOURCES} notify_libnotify.cpp)
    set(EXTRA_LIBS ${EXTRA_LIBS} ${LIBNOTIFY_LIBRARIES})

elseif(WIN32)
    set(exe "SequelJoe")
    set(SOURCES ${SOURCES} notify_win32.cpp)
    set(EXTRA_LIBS ${EXTRA_LIBS} ws2_32)
endif()


# Define executable
add_executable(${exe} MACOSX_BUNDLE WIN32 ${SOURCES} ${SRC_RESOURCES})
target_link_libraries(${exe} ${LIBSSH2_LIBRARY} ${EXTRA_LIBS})
qt5_use_modules(${exe} Widgets Sql)

install(TARGETS ${exe}
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION bin COMPONENT Runtime
)
