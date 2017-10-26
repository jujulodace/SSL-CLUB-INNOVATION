cmake_minimum_required(VERSION 3.0)
project(viewer)

find_package(catkin REQUIRED COMPONENTS
  RhobanUtils
  )

include_directories(${catkin_INCLUDES})

# Find the QtWidgets library
find_package(Qt5Widgets)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

# QT resource file
qt5_add_resources(RESOURCES www.qrc)

# Adding mainwindow QT form
qt5_wrap_ui(ui_mainwindow.h mainwindow.ui)

# Sources
set (SOURCES
    main.cpp
    mainwindow.cpp
    API.cpp
    ui_mainwindow.h
    ${RESOURCES}
)

# Tell CMake to create the helloworld executable
add_executable(viewer ${SOURCES})

# Widgets
qt5_use_modules(viewer
    Core
    Gui
    Network
    WebKit
    WebKitWidgets
    Widgets
)

target_link_libraries(viewer ${catkin_LIBRARIES})