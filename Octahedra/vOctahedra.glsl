#version 140

in vec4 vPosition;	// 输入属性，顶点位置
in vec4 vColor;		// 输入属性，顶点颜色

uniform mat4 MVPMatrix;	// 模视投影矩阵

flat out vec4 colorFlat;		// 用于平面着色的输出颜色
smooth out vec4 colorSmooth;	// 用于平滑着色的输出颜色

void main()
{
	// 通过变换得到裁剪坐标系下顶点坐标
	gl_Position = MVPMatrix * vPosition;	
	
	/*输出颜色直接等于输入颜色*/
	colorFlat = vColor;
	colorSmooth = vColor;
}