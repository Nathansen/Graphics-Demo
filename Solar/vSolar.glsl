#version 140

in vec3 vPosition;		 // ���붥������
uniform mat4 MVPMatrix;  // ģ��ͶӰ����

void main()
{
	// ��ģ��ͶӰ����Ӧ���ڶ�������
	gl_Position = MVPMatrix * vec4(vPosition, 1.0);
}