#version 330 core

uniform mat4 MVPMatrix;		// 模视投影矩阵

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
} vs_out;

void main()
{
    vs_out.FragPos = vPosition;
    vs_out.Normal = vNormal;
    vs_out.Color = vec3(1, 1, 1);
    gl_Position = MVPMatrix * vec4(vPosition, 1.0);
}
