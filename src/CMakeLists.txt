#[[
SPDX-FileCopyrightText: 2024 basysKom GmbH
SPDX-License-Identifier: LGPL-3.0-or-later
]]

cmake_minimum_required(VERSION 3.16)

project(QtQuickComputeItem VERSION 0.0.1 LANGUAGES CXX)

set(PROJECT_SOURCES
  computeitem.cpp
  computeitem.h
  imagebuffer.cpp
  imagebuffer.h
  storagebuffer.cpp
  computeshaderbuffer.h
  computeshaderbuffer.cpp
  storagebuffer.h
  storagebufferview.h
  storagebufferview.cpp
  imagebufferview.h
  imagebufferview.cpp
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Quick)
find_package(Qt6 COMPONENTS ShaderTools)

# Find Qt package
find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core Gui Qml Quick OpenGL )
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Qml Gui Quick OpenGL)

if (${QT_VERSION_MAJOR} EQUAL 6)
    # TODO use new policy
    # https://doc.qt.io/qt-6/qt-cmake-policy-qtp0001.html
    qt_policy(SET QTP0001 OLD)
endif()

qt6_add_qml_module(
        ${PROJECT_NAME}
    URI
        ${PROJECT_NAME}
    VERSION
        1.0
    SHARED
    PLUGIN_TARGET
        ${PROJECT_NAME}
    OUTPUT_DIRECTORY
       ${CMAKE_BINARY_DIR}/lib/${PROJECT_NAME}
    SOURCES
        ${PROJECT_SOURCES}
        ${QT6_PROJECT_SOURCES}
)

include_directories(${Qt6Gui_PRIVATE_INCLUDE_DIRS} ${Qt6Core_PRIVATE_INCLUDE_DIRS} "qtquickcomputeitem_includes")

qt_add_shaders(${PROJECT_NAME} "qtquickcomputeitem_shaders"
    GLSL "310es,330"
    HLSL 50
    MSL 12
    BATCHABLE
    PRECOMPILE
    OPTIMIZED
    PREFIX
        "/"
    FILES
        "shaders/pointcloud.frag"
        "shaders/pointcloud.vert"
)

set(QT_QUICK_COMPUTE_ITEM_RESOURCE_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/.qsb/shaders/pointcloud.vert.qsb"
    "${CMAKE_CURRENT_BINARY_DIR}/.qsb/shaders/pointcloud.frag.qsb"
)

qt6_add_resources(${PROJECT_NAME} "qtquickcomputeitem_resources"
    PREFIX
        "/"
    FILES
        ${QT_QUICK_COMPUTE_ITEM_RESOURCE_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt6::GuiPrivate
)
