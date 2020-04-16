#version 140

in vec4 vPosition; // 顶点位置属性
in vec3 vColor; // 顶点颜色属性
out vec4 color; // 输出颜色

void main()
{
	gl_Position = vPosition; 
	color = vec4(vColor, 1.0);
}