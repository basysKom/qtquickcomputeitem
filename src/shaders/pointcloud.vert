// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 vertexColor;

layout(std140, binding = 0) uniform UBuf
{
    mat4 mvpProjection;
    float width;
    float height;
    float pointSize;
    int flip;
} ubuf;

out gl_PerVertex { vec4 gl_Position; float gl_PointSize; };

void main()
{
    vec4 itemPos = (position + vec4(1.0, 1.0, 0.0, 0.0)) * vec4(ubuf.width * 0.5, ubuf.height * 0.5, 1.0, 1.0);

    vertexColor = color;
    gl_PointSize = ubuf.pointSize;
    gl_Position = ubuf.mvpProjection * itemPos;

    if (ubuf.flip != 0) {
        gl_Position.y = 1.0 - gl_Position.y;
    }
}