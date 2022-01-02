cmake_minimum_required(VERSION 3.1)

project(elam_emulator VERSION 1.0.0 LANGUAGES CXX C)

#include(C:/vcpkg/scripts/buildsystems/vcpkg.cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_DEBUG_POSTFIX d)

find_package(Qt5 COMPONENTS Widgets)

find_package(Qt5SerialPort CONFIG REQUIRED)
find_package(Qt5Qml CONFIG REQUIRED)

find_path(QSCINTILLA_INCLUDE_DIRS Qsci/qsciscintilla.h)
include_directories(${QSCINTILLA_INCLUDE_DIRS})

find_library(QSCINTILLA_LIBRARY_RELEASE qscintilla2_qt5)
find_library(QSCINTILLA_LIBRARY_DEBUG qscintilla2_qt5d)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

add_executable(elam_emulator 
    main.cpp
    mainwindow.ui
    mainwindow.cpp
    bitsmodel.cpp 
    hexitemdelegate.cpp 
    hexspinbox.cpp 
    registermodel.cpp 
    circle.c 
    crc16.c 
    modbus.c 
    addplcdialog.ui 
    addplcdialog.cpp 
    settingsdialog.ui 
    settingsdialog.cpp 
    mb_regs.cpp 
    scripteditor.ui 
    scripteditor.cpp 
    editorsettingdialog.ui
    editorsettingdialog.cpp
    elam_emulator.qrc        
)

target_link_libraries( elam_emulator PRIVATE Qt5::Widgets Qt5::SerialPort Qt5::SerialPortPrivate Qt5::Qml
   debug ${QSCINTILLA_LIBRARY_DEBUG} 
   optimized ${QSCINTILLA_LIBRARY_RELEASE} )
