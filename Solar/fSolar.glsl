#version 140

uniform vec3 uColor;	 // ƬԪ��ɫ
out vec4 fragColor;		 // ���ƬԪ��ɫ

void main()
{
	fragColor = vec4(uColor, 1.0);	
}