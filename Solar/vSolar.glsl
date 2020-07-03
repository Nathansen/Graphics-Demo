#version 140

in vec3 vPosition;		 // 输入顶点坐标
uniform mat4 MVPMatrix;  // 模视投影矩阵

void main()
{
	// 将模视投影矩阵应用于顶点坐标
	gl_Position = MVPMatrix * vec4(vPosition, 1.0);
}