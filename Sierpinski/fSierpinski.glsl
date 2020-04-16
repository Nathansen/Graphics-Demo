#version 140

in vec4 color; // 输入颜色
out vec4 fColor; // 输出颜色

void main()
{
	fColor = color; // 不修改颜色，直接使用光栅化阶段插值得到的颜色
}