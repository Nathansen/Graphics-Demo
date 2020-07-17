#version 140

uniform vec3 uColor = vec3(1.0, 1.0, 1.0);	 // 片元颜色
out vec4 fragColor;		 // 输出片元颜色

void main()
{
	fragColor = vec4(uColor, 1.0);	
}