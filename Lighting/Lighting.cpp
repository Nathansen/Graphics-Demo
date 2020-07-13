//------------------------------------------------------------------------------
//		Copyright(c) 2020 WarZhan zhanweilong1992@gmail.com
//		All rights reserved.
//		Use, modificationand distribution are subject to the "MIT License"
//------------------------------------------------------------------------------

#include "Angel.h"

typedef vec3 point3;

point3* sphere;			// 存放一个球的顶点坐标数据的指针

// 用于生成一个中心在原点的球的顶点坐标数据(南北极在z轴方向)
// 返回值为球的顶点数，参数为球的半径及经线和纬线数
GLsizei BuildSphere(GLfloat radius, GLsizei columns, GLsizei rows)
{
	int index = 0;	// 数组索引
	point3* vertices = new point3[(rows + 1) * (columns + 1)]; // 存放不同顶点的数组

	for (int r = 0; r <= rows; r++)
	{
		float v = (float)r / (float)rows;  // [0,1]
		float theta1 = v * (float)M_PI;	   // [0,PI]

		point3 temp(0, 0, 1);
		point3 n = temp;
		GLfloat cosTheta1 = cos(theta1);
		GLfloat sinTheta1 = sin(theta1);
		n.x = temp.x * cosTheta1 + temp.z * sinTheta1;
		n.z = -temp.x * sinTheta1 + temp.z * cosTheta1;

		for (int c = 0; c <= columns; c++)
		{
			float u = (float)c / (float)columns; // [0,1]
			float theta2 = u * (float)(M_PI * 2); // [0,2PI]
			point3 pos = n;
			temp = n;
			GLfloat cosTheta2 = cos(theta2);
			GLfloat sinTheta2 = sin(theta2);

			pos.x = temp.x * cosTheta2 - temp.y * sinTheta2;
			pos.y = temp.x * sinTheta2 + temp.y * cosTheta2;

			point3 posFull = pos;
			posFull *= radius;

			vertices[index++] = posFull;
		}
	}

	/*生成最终顶点数组数据*/
	if (sphere)
	{
		delete[] sphere;	// 如果sphere已经有数据，先回收
	}
	int numVertices = rows * columns * 6; // 顶点数
	sphere = new point3[numVertices];

	int colLength = columns + 1;
	index = 0;
	for (int r = 0; r < rows; r++)
	{
		int offset = r * colLength;

		for (int c = 0; c < columns; c++)
		{
			int ul = offset + c;						// 左上
			int ur = offset + c + 1;					// 右上
			int br = offset + (c + 1 + colLength);	// 右下
			int bl = offset + (c + 0 + colLength);	// 左下

			// 由两条经线和纬线围成的矩形
			sphere[index++] = vertices[ul];
			sphere[index++] = vertices[bl];
			sphere[index++] = vertices[br];
			sphere[index++] = vertices[ul];
			sphere[index++] = vertices[br];
			sphere[index++] = vertices[ur];
		}
	}

	delete[] vertices;

	return numVertices;
}

int main()
{
	return 0;
}
