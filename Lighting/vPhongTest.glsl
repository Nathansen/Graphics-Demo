#version 330 core

uniform mat4 MVPMatrix;		// 模视投影矩阵
uniform mat4 ModelMatrix;		// 模型变换矩阵

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;

out VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec3 Color;
} vs_out;

void main()
{
    vs_out.FragPos = (ModelMatrix * vec4(vPosition, 1.0)).xyz;
    vs_out.Normal = (ModelMatrix * vec4(vNormal, 1.0)).xyz;
    vs_out.Color = vec3(1, 0, 0);
    gl_Position = MVPMatrix * vec4(vPosition, 1.0);
}
