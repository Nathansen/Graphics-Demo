﻿#version 140

uniform mat4 PMatrix;		// 投影矩阵
uniform mat4 MVMatrix;		// 模视矩阵

in vec3 vPosition;
in vec3 vNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;

void main()
{
    FragPos = (MVMatrix * vec4(vPosition, 1.0)).xyz;
    Normal = (MVMatrix * vec4(vNormal, 0.0)).xyz;
    Color = vec3(1, 0, 0); // 默认红色

    gl_Position = PMatrix * MVMatrix * vec4(vPosition, 1.0);
}
