// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#version 440

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

layout(location = 0) out vec4 vertexColor;

layout(std140, binding = 0) uniform VSUbuf
{
    mat4 mvpProjection;
    float width;
    float height;
    float pointSize;
    int flip;
} vsubuf;

out gl_PerVertex { vec4 gl_Position; float gl_PointSize; };

void main()
{
    // transform to view coords
    vec4 itemPos = (position + vec4(1.0, 1.0, 0.0, 0.0)) * vec4(0.5, 0.5, 1.0, 1.0);
    if (vsubuf.flip != 0) {
        itemPos.y = 1.0 - itemPos.y;
    }
    itemPos = itemPos * vec4(vsubuf.width, vsubuf.height, 1.0, 1.0);

    vertexColor = color;
    gl_PointSize = vsubuf.pointSize;
    gl_Position = vsubuf.mvpProjection * itemPos;
}