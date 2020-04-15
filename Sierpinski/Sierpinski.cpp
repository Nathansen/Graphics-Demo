// Sierpinski.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "Angel.h"

typedef vec3 point3;
typedef vec3 color3;

const int NumTimesToSubdivide = 4; // 迭代次数
const int NumTetrahedrons = 256; // 生成四面体个数 4^4
const int NumTriangles = 4 * NumTetrahedrons; // 产生三角形个数 每个四面体 4 个三角形
const int NumVertices = 3 * NumTriangles; // 顶点数

point3 points[NumVertices]; // 顶点位置数组
color3 colors[NumVertices]; // 顶点颜色数组

// 将三角形顶点坐标 和 颜色 加入数组中
void triangle(point3 a, point3 b, point3 c, int colorIndex)
{
	static color3 base_color[] =
	{
		color3(1.0, 0.0, 0.0),
		color3(0.0, 1.0, 0.0),
		color3(0.0, 0.0, 1.0),
		color3(0.0, 0.0, 0.0),
	};

	static int index = 0;

	points[index] = a;
	colors[index] = base_color[colorIndex];
	index++;

	points[index] = b;
	colors[index] = base_color[colorIndex];
	index++;

	points[index] = c;
	colors[index] = base_color[colorIndex];
	index++;

}

// 生成一个四面体
void tetra(point3 a, point3 b, point3 c, point3 d)
{
	triangle(a, b, c, 0);
	triangle(a, c, d, 1);
	triangle(a, d, b, 2);
	triangle(b, d, c, 3);
}

int main()
{

}