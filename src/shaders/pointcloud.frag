// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#version 440

layout(location = 0) in vec4 vertexColor;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform UBuf
{
    float pointSize;
} ubuf;

void main()
{
    fragColor = vertexColor;
}