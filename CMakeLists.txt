#[[
SPDX-FileCopyrightText: 2024 basysKom GmbH
SPDX-License-Identifier: LGPL-3.0-or-later
]]

cmake_minimum_required(VERSION 3.16)

project(QtQuickComputeItemPlugin
    VERSION 0.1.0.0
    DESCRIPTION "Enable compute shader pipelines in Qt Quick"
    HOMEPAGE_URL "https://www.basyskom.com"
    LANGUAGES CXX
)

option(COMPUTE_ITEM_EXAMPLES "Build example applications." ON)

add_subdirectory(src)

if (COMPUTE_ITEM_EXAMPLES)
   add_subdirectory(examples)
endif()