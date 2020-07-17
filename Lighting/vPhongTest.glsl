#version 330 core

uniform mat4 PMatrix;		// 投影矩阵
uniform mat4 MVMatrix;		// 模视矩阵

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
} vs_out;

void main()
{
    vs_out.FragPos = (MVMatrix * vec4(vPosition, 1.0)).xyz;
    vs_out.Normal = (MVMatrix * vec4(vNormal, 1.0)).xyz;
    vs_out.Color = vec3(1, 0, 0);
    gl_Position = PMatrix * MVMatrix * vec4(vPosition, 1.0);
}
