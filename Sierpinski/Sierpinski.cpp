//------------------------------------------------------------------------------
//		Copyright(c) 2020 WarZhan zhanweilong1992@gmail.com
//		All rights reserved.
//		Use, modificationand distribution are subject to the "MIT License"
//------------------------------------------------------------------------------

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
void Triangle(point3 a, point3 b, point3 c, int colorIndex)
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
void Tetra(point3 a, point3 b, point3 c, point3 d)
{
	Triangle(a, b, c, 0);
	Triangle(a, c, d, 1);
	Triangle(a, d, b, 2);
	Triangle(b, d, c, 3);
}

// deep 迭代次数
void DivideTetra(point3 a, point3 b, point3 c, point3 d, int deep)
{
	if (deep > 0)
	{
		point3 mid[6];

		mid[0] = (a + b) / 2.0;
		mid[1] = (a + c) / 2.0;
		mid[2] = (a + d) / 2.0;
		mid[3] = (b + c) / 2.0;
		mid[4] = (c + d) / 2.0;
		mid[5] = (b + d) / 2.0;

		DivideTetra(a, mid[0], mid[1], mid[2], deep - 1);
		DivideTetra(mid[0], b, mid[3], mid[5], deep - 1);
		DivideTetra(mid[1], mid[3], c, mid[4], deep - 1);
		DivideTetra(mid[2], mid[5], mid[4], d, deep - 1);
	}
	else
	{
		// 记录最终的数据
		Tetra(a, b, c, d);
	}
}

void Init()
{
	// 初始四面体
	point3 vertices[4] =
	{
		point3(0.0, 0.0, -1.0),
		point3(0.0, 0.942809, 0.333333),
		point3(-0.816497, -0.471405, 0.333333),
		point3(0.816497, -0.471405, 0.333333)
	};

	// 细分
	DivideTetra(vertices[0], vertices[1], vertices[2], vertices[3], NumTimesToSubdivide);

	// 创建一个顶点数组对象 VAO vertex_array_object
	GLuint vao;
	glGenVertexArrays(1, &vao); // 生成一个未用的 VAO ID，存于 vao 中
	glBindVertexArray(vao);		// 创建 id 为 vao 的 VAO，并绑定为当前 VAO

	// 创建并初始化一个缓冲区对象（Buffer Object）
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer); // 创建 id 为 buffer 的 Array Buffer 对象，并绑定为单前 Array Buffer 对象

	// 为 Buffer 对象在 GPU 端申请空间，并提供数据
	glBufferData(GL_ARRAY_BUFFER,	// Buffer 类型
		sizeof(points) + sizeof(colors), // 申请空间大小（以字节为单位）位置 + 颜色
		NULL,					//  数据指针
		GL_STATIC_DRAW);			// 表明将如何使用 Buffer 的标志（GL_STATIC_DRAW ：一次提供数据，多次绘制）


	// 存入顶点数据
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);

	// 存入颜色数据
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);


	// 初始化 shader
	GLuint program = InitShader("vSierpinski.glsl", "fSierpinski.glsl");
	glUseProgram(program);

	// 获取 shader 程序中变量地址
	GLuint loc = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(loc); // 启用顶点属性数组

	// 为顶点属性数组提供数据（数据存放在之前 buffer 对象中）
	glVertexAttribPointer(
		loc,		// 属性变量索引
		3,			// 每个顶点属性的分量个数
		GL_FLOAT,	// 数组数据类型
		GL_FALSE,	// 是否进行归一化
		0,			// 在数组中相邻属性成员间的间隔（以字节为单位）
		BUFFER_OFFSET(0)); // 第一个属性值在 buffer 中的偏移量

	// 颜色
	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);

	glVertexAttribPointer(
		vColor,		// 属性变量索引
		3,			// 每个顶点属性的分量个数
		GL_FLOAT,	// 数组数据类型
		GL_FALSE,	// 是否进行归一化
		0,			// 在数组中相邻属性成员间的间隔（以字节为单位）
		BUFFER_OFFSET(sizeof(points))); // 第一个属性值在 buffer 中的偏移量

	glEnable(GL_DEPTH_TEST); // 启用深度检测

	glClearColor(1.0, 1.0, 1.0, 1.0); // 指定背景刷新颜色

}

void Display()
{
	// 将帧缓存的深度值刷新为初始深度值
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	glFlush();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	// 以下窗口初始化 属性 和 上下文（Context）的函数必须在 glutCreateWindow 前调用
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(0, 0);

	glutInitContextVersion(3, 1); // 指定 OpengGL 3.1

	// 保持先前兼容，即不使用任何废弃的
	// 如程序中使用了弃用函数则注释本行
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	// OpenGL 3.2 以上版本才有效果，作用和 glutInitContextFlags 类似
	//glutInitContextProfile(GLUT_CORE_PROFILE);

	glutCreateWindow("Simple Sierpinski");

	// 显卡驱动非正式发布版 或者 与 glew 库 规范不兼容时加上此行
	// 如果在 glGenVertexArrays 处发生 Access Violation 则加上此行
	glewExperimental = GL_TRUE;

	GLenum err = glewInit(); // 初始化 glew 库，必须在 glutCreateWindow 之后调用
	if (err != GLEW_OK)
	{
		std::cout << "glewInit fault !! " << std::endl;
		exit(EXIT_FAILURE);
	}

	Init();

	// 注册显示回调函数 必须有
	glutDisplayFunc(Display);

	glutMainLoop();

	return 0;
}