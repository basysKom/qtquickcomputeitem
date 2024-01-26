#[[
SPDX-FileCopyrightText: 2024 basysKom GmbH
SPDX-License-Identifier: BSD-3-Clause
]]

cmake_minimum_required(VERSION 3.16)

project(qmlbasicstorage
    VERSION 0.1.0.0
    DESCRIPTION "A basic example to demonstrate the QtQuickComputeShaderItem with a storage buffer"
    HOMEPAGE_URL "https://www.basyskom.com"
    LANGUAGES CXX
)

set(PROJECT_SOURCES
  main.cpp
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

qt_add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
)

qt6_add_shaders(${PROJECT_NAME} "qmlbasicstorage_shaders"
    GLSL "430"
    HLSL 50
    MSL 12
    BATCHABLE
    PRECOMPILE
    OPTIMIZED
    PREFIX
        "/"
    FILES
        "shaders/computeshader.comp"
)

set(qmlbasicstorage_RESOURCE_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/.qsb/shaders/computeshader.comp.qsb"
    "main.qml"
)

qt6_add_resources(${PROJECT_NAME} "qmlbasicstorage_resources"
    PREFIX
        "/"
    FILES
        ${qmlbasicstorage_RESOURCE_FILES}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt6::GuiPrivate
)
