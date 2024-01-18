// SPDX-FileCopyrightText: 2024 basysKom GmbH
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#version 440

layout(location = 0) in vec2 qt_TexCoord0;

layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    float contentWidth;
    float contentHeight;
} ubuf;

layout(binding = 1) uniform sampler2D source;

#define texRGB(p)  texture(source, p).rgb

void main()
{
    vec2 pixelSize = 1.0 / vec2(ubuf.contentWidth, ubuf.contentHeight);

    vec4 color   = texture(source, qt_TexCoord0) ;
    vec3 center  = texRGB(qt_TexCoord0);
    vec3 right   = texRGB(qt_TexCoord0 + vec2( pixelSize.x,          0.0));
    vec3 top     = texRGB(qt_TexCoord0 + vec2(         0.0,  pixelSize.y));
    vec3 left    = texRGB(qt_TexCoord0 + vec2(-pixelSize.x,          0.0));
    vec3 bottom  = texRGB(qt_TexCoord0 + vec2(         0.0, -pixelSize.y));

    vec3 vertical = ((right - left) + vec3(1.0)) * 0.5;
    vec3 horizontal = ((bottom - top) + vec3(1.0)) * 0.5;
    vec3 combined = (vertical + horizontal) * 0.5;

    vec2 normalTangent = vec2(length(vertical), length(horizontal));

    float g = dot(combined, vec3(0.344, 0.5, 0.156));

    fragColor = vec4(g, g, g, color.a) * ubuf.qt_Opacity;
}