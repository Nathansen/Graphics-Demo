#version 140

uniform bool bSmooth;	// 平滑着色标志

flat in vec4 colorFlat;		// 未经过插值的颜色
smooth in vec4 colorSmooth;	// 经过插值得到的颜色

out vec4 FragColor;		// 输出片元最终颜色

void main()
{
	FragColor = bSmooth ? colorSmooth : colorFlat;
}