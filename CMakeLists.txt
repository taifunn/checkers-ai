cmake_minimum_required(VERSION 3.16)

project(checkers VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

set(PROJECT_SOURCES
    gui/main.cpp

    gui/mainwindow.cpp
    gui/mainwindow.h
    gui/mainwindow.ui

    gui/boardwidget.cpp
    gui/boardwidget.h

    engine/board.cpp
    engine/board.hpp

    engine/move_generator.cpp

    engine/ai.cpp
    engine/ai.hpp

    engine/types.hpp

    resources.qrc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(checkers
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(checkers
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(checkers PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_include_directories(checkers PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/gui
    ${CMAKE_CURRENT_SOURCE_DIR}/engine
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(checkers)
endif()