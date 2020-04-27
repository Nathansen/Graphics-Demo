//------------------------------------------------------------------------------
//		Copyright(c) 2020 WarZhan zhanweilong1992@gmail.com
//		All rights reserved.
//		Use, modificationand distribution are subject to the "MIT License"
//------------------------------------------------------------------------------

#include "Angel.h"   // 已将GLEW库和freeglut库的头文件包含及链接放到了Angel.h中

// 以下全局变量用于控制动画的状态和速度
float RotateAngle = 0.0f;		// 绕y轴旋转的角度
float Azimuth = 0.0;			// 绕x轴旋转的角度

float AngleStepSize = 3.0f;			// 角度变化步长(3度)
const float AngleStepMax = 10.0f;	// 角度变化步长最大值
const float AngleStepMin = 0.1f;	// 角度变化步长最小值

int WireFrameOn = 0;			// 线框模式标志，当等于1时为线框模式

/*定义类的别名*/
typedef Angel::vec4 point4;
typedef Angel::vec4 color4;

// 以原点为中心八面体顶点
// 使用齐次坐标，第4个坐标都为1，表示是点而不是向量
point4 vertices[6] =
{
	point4(0.0, 1.0, 0.0, 1.0), // 顶
	point4(-1.0, 0.0, 0.0, 1.0), // 左
	point4(0.0, 0.0, 1.0, 1.0), // 前
	point4(1.0, 0.0, 0.0, 1.0), // 右
	point4(0.0, 0.0, -1.0, 1.0), // 后
	point4(0.0, -1.0, 0.0, 1.0), // 底
};

// RGBA颜色
color4 colors[10] =
{
	color4(1.0, 0.0, 0.0, 1.0),	// 红
	color4(1.0, 1.0, 0.0, 1.0),	// 黄
	color4(0.0, 1.0, 0.0, 1.0),	// 绿
	color4(0.0, 0.0, 1.0, 1.0),	// 蓝
	color4(0.3, 0.0, 0.0, 1.0),	// 浅红
	color4(0.3, 0.3, 0.0, 1.0),	// 浅黄
	color4(0.0, 0.3, 0.0, 1.0),	// 浅绿
	color4(0.0, 0.0, 0.3, 1.0),	// 浅蓝
	color4(1.0, 1.0, 1.0, 1.0), // 白
	color4(0.0, 0.0, 0.0, 1.0), // 黑
};

// 颜色枚举常量，值与colors中相应颜色的索引一致
enum { RED, YELLOW, GREEN, BLUE, HALF_RED, HALF_YELLOW, HALF_GREEN, HALF_BLUE, WHITE, BLACK};

const int NumVertices = 24;	// 8个面，每个面1个三角形，每个三角形3个顶点，共24个顶点
point4 points[NumVertices] =
{
	// 立方体顶点坐标数组
	// 上半部分
	vertices[0], vertices[1], vertices[2],
	vertices[0], vertices[2], vertices[3],
	vertices[0], vertices[3], vertices[4],
	vertices[0], vertices[4], vertices[1],
	// 下半部分
	vertices[1], vertices[5], vertices[2],
	vertices[2], vertices[5], vertices[3],
	vertices[3], vertices[5], vertices[4],
	vertices[4], vertices[5], vertices[1],
};

// 右边图形顶点颜色数组
color4 colorsRight[NumVertices] =
{
	// 上半部分
	colors[RED], colors[RED], colors[RED],
	colors[GREEN], colors[GREEN], colors[GREEN],
	colors[BLUE], colors[BLUE], colors[BLUE],
	colors[YELLOW], colors[YELLOW], colors[YELLOW],

	// 下半部分
	colors[HALF_RED], colors[HALF_RED], colors[HALF_RED],
	colors[HALF_GREEN], colors[HALF_GREEN], colors[HALF_GREEN],
	colors[HALF_BLUE], colors[HALF_BLUE], colors[HALF_BLUE],
	colors[HALF_YELLOW], colors[HALF_YELLOW], colors[HALF_YELLOW],
};

// 左边图形顶点颜色数组(注意和points中顶点序列对应)
color4 colorsLeft[NumVertices] =
{
	// 上半部分
	colors[WHITE], colors[RED], colors[GREEN],
	colors[WHITE], colors[GREEN], colors[BLUE],
	colors[WHITE], colors[BLUE], colors[YELLOW],
	colors[WHITE], colors[YELLOW], colors[RED],

	// 下半部分
	colors[RED], colors[BLACK], colors[GREEN],
	colors[GREEN], colors[BLACK], colors[BLUE],
	colors[BLUE], colors[BLACK], colors[YELLOW],
	colors[YELLOW], colors[BLACK], colors[RED],
};

GLuint vColor;	// shader中顶点属性变量"vColor"的索引
GLuint MVPMatrix;	// shader中uniform变量"MVPMatrix"的索引
GLuint bSmooth;	// shader中uniform变量"bSmooth"的索引

mat4 matProj;	// 投影矩阵

/*glutKeyboardFunc设置下面函数用以处理“普通”按键事件)*/
void Keyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':	// 切换显示模式
		WireFrameOn = 1 - WireFrameOn;
		if (WireFrameOn)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// 仅显示线框
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		// 显示实心多边形
		}
		glutPostRedisplay();	// 刷新显示
		break;
	case 'R':	// 增加旋转速度
		AngleStepSize *= 1.5;
		if (AngleStepSize > AngleStepMax)
		{
			AngleStepSize = AngleStepMax;
		}
		break;
	case 'r':	// 降低旋转速度
		AngleStepSize /= 1.5;
		if (AngleStepSize < AngleStepMin)
		{
			AngleStepSize = AngleStepMin;
		}
		break;
	case 27:	// Esc 键退出
		exit(EXIT_SUCCESS);
	}
}

/*glutSpecialFunc设置以下函数用于处理“特殊”按键事件*/
void Special(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:
		Azimuth += AngleStepSize;
		if (Azimuth > 80.0f)
		{
			Azimuth = 80.0f;
		}
		break;
	case GLUT_KEY_DOWN:
		Azimuth -= AngleStepSize;
		if (Azimuth < -80.0f)
		{
			Azimuth = -80.0f;
		}
		break;
	case GLUT_KEY_LEFT:
		RotateAngle += AngleStepSize;
		if (RotateAngle > 180.0f)
		{
			RotateAngle -= 360.0f;
		}
		break;
	case GLUT_KEY_RIGHT:
		RotateAngle -= AngleStepSize;
		if (RotateAngle < -180.0f)
		{
			RotateAngle += 360.0f;
		}
		break;
	}
	glutPostRedisplay();
}

/*显示回调函数*/
void Display(void)
{
	// 清除颜色缓存和深度缓存内容
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 创建变换矩阵
	mat4 transform = matProj		 // 投影矩阵
		* Translate(0.0, 0.0, -25.0) // 沿z轴平移
		* RotateY(RotateAngle)	     // 绕y轴旋转
		* RotateX(Azimuth);		     // 绕x轴旋转

	/*绘制右方图形*/
	// 计算应用于右方图形的变换
	mat4 transform1 = transform * Translate(1.5, 0.0, 0.0);
	glUniformMatrix4fv(MVPMatrix, // uniform变量索引
		1,		// 矩阵个数
		true,	// 是否转置
		transform1
	);
	glUniform1f(bSmooth, 0);  // 不使用平滑着色，即使用平面着色
	// 为顶点属性数组提供数据(数据存放在之前buffer对象中)
	glVertexAttribPointer(
		vColor,				// 属性变量索引
		4,					// 每个顶点属性的分量个数
		GL_FLOAT,			// 数组数据类型
		GL_FALSE,			// 是否进行归一化处理
		0,					// 在数组中相邻属性成员间的间隔(以字节为单位)
		BUFFER_OFFSET(sizeof(points) + sizeof(colorsLeft))  // 第一个属性值在buffer中的偏移量
	);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);	// 绘制

	/*绘制左方图形*/
	// 计算应用于左方图形的变换
	mat4 transform2 = transform * Translate(-1.5, 0.0, 0.0);
	glUniformMatrix4fv(MVPMatrix, // uniform变量索引
		1,		// 矩阵个数
		true,	// 是否转置
		transform2
	);
	glUniform1f(bSmooth, 1);  // 使用平滑着色
	// 为顶点属性数组提供数据(数据存放在之前buffer对象中)
	glVertexAttribPointer(
		vColor,				// 属性变量索引
		4,					// 每个顶点属性的分量个数
		GL_FLOAT,			// 数组数据类型
		GL_FALSE,			// 是否进行归一化处理
		0,					// 在数组中相邻属性成员间的间隔(以字节为单位)
		BUFFER_OFFSET(sizeof(points))  // 第一个属性值在buffer中的偏移量
	);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);	// 绘制

	glutSwapBuffers();	// 交换前后端缓存
}


// 初始化函数
void Init()
{
	/*创建一个顶点数组对象(VAO)*/
	GLuint vao;
	glGenVertexArrays(1, &vao);  // 生成一个未用的VAO ID，存于变量vao中
	glBindVertexArray(vao);      // 创建id为vao的VAO，并绑定为当前VAO

	/*创建并初始化一个缓冲区对象(Buffer Object)*/
	GLuint buffer;
	glGenBuffers(1, &buffer); // 生成一个未用的缓冲区对象ID，存于变量buffer中
	// 创建id为buffer的Array Buffer对象，并绑定为当前Array Buffer对象
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// 为Buffer对象在GPU端申请空间，并提供数据
	glBufferData(GL_ARRAY_BUFFER,	// Buffer类型
		sizeof(points) + sizeof(colorsLeft) + sizeof(colorsRight),  // 申请空间大小(存坐标和颜色)
		NULL,			// 暂不提供数据
		GL_STATIC_DRAW	// 表明将如何使用Buffer的标志(GL_STATIC_DRAW含义是一次提供数据，多遍绘制)
	);
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(points), points);			// 加载顶点位置数据
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points),
		sizeof(colorsLeft), colorsLeft);	// 加载颜色数据，注意偏移量的变化
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colorsLeft),
		sizeof(colorsRight), colorsRight);	// 加载颜色数据，注意偏移量的变化

	/*加载shader并使用所得到的shader程序*/
	// InitShader为InitShader.cpp中定义的函数，参数分别为顶点和片元shader的文件名
	// 返回值为shader程序对象的ID
	GLuint program = InitShader("vOctahedra.glsl", "fOctahedra.glsl");
	glUseProgram(program); // 使用该shader程序

	/*初始化顶点着色器中的顶点位置属性*/
	// 获取shader程序中属性变量的位置(索引)
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);	// 启用顶点属性数组
	// 为顶点属性数组提供数据(数据存放在之前buffer对象中)
	glVertexAttribPointer(
		vPosition,			// 属性变量索引
		4,					// 每个顶点属性的分量个数
		GL_FLOAT,			// 数组数据类型
		GL_FALSE,			// 是否进行归一化处理
		0,					// 在数组中相邻属性成员间的间隔(以字节为单位)
		BUFFER_OFFSET(0)    // 第一个属性值在buffer中的偏移量
	);

	/*初始化顶点着色器中的顶点颜色属性*/
	// 获取shader程序中属性变量的位置(索引)
	vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);	// 启用顶点属性数组

	// 获取uniform变量索引
	MVPMatrix = glGetUniformLocation(program, "MVPMatrix");
	bSmooth = glGetUniformLocation(program, "bSmooth");

	glEnable(GL_DEPTH_TEST);	// 深度检测必须打开

	/*这两行代码将导致背面不会显示*/
	glCullFace(GL_BACK);		// 剔除背面	
	glEnable(GL_CULL_FACE);	// 开启面剔除
}

// 当窗口大小改变时调用)
//w,h - 窗口的宽度和高度(以像素为单位)
void Reshape(int w, int h)
{
	GLfloat aspectRatio;

	// 定义窗口中用于OpenGL渲染的部分
	glViewport(0, 0, w, h);	// 视口使用整个窗口

	/*设置投影观察矩阵：透视投影
	  复杂的地方是窗口的宽高比可能与我们希望观察到的场景的宽高比可能不同*/
	  // 防止除0
	w = (w == 0) ? 1 : w;
	h = (h == 0) ? 1 : h;
	aspectRatio = (double)w / (double)h;	// 宽高比
	matProj = Perspective(15.0, aspectRatio, 15.0, 35.0);	// 透视投影变换
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);	// 初始化GLUT库，必须在其他glut函数前执行

	/*以下初始化窗口属性和上下文(Context)的函数必须在glutCreateWindow之前调用*/
	// 初始化显示模式，这里使用了双缓存和深度缓存
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512); // 初始化窗口尺寸(宽度和高度，单位为像素)

	// 以下2个函数来自freeglut库，用于确认 
	// 代码是基于OpenGL 3.1版本的
	glutInitContextVersion(3, 1); // 表明使用OpenGL 3.1
	// 保持向前兼容，即不使用任何弃用的函数
	// 如程序中使用了弃用函数则注释掉本行
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);
	// 请求opengl 3.2以上版本才有效果，作用和glutInitContextFlags类似
	//glutInitContextProfile( GLUT_CORE_PROFILE ); 

	glutCreateWindow("Color Cube");	// 创建窗口，标题为"Color Cube",并初始化Context

	// 显卡驱动非正式发布版或者与glew库规范不兼容时加上此行
	// 如果在glGenVertexArrays处发生Access Violation则加上此行
	glewExperimental = GL_TRUE;

	GLenum err = glewInit(); // 初始化glew库，必须在glutCreateWindow之后调用
	if (err != GLEW_OK)  // 初始化不成功？
	{
		std::cout << "glewInit 失败, 退出程序." << std::endl;
		exit(EXIT_FAILURE); // 退出程序
	}

	// 注册回调函数
	glutDisplayFunc(Display);   // 指定你管显示回调函数，必须要有
	glutKeyboardFunc(Keyboard);	// 指定普通按键回调函数
	glutSpecialFunc(Special); // 指定特殊键回调函数
	glutReshapeFunc(Reshape);	// 设置处理窗口大小调整的回调函数

	// 自定义的初始化函数
	// 通常将shader初始化和在程序运行过程中保持不变的属性的设置放在此函数中
	Init();

	/*输出帮助信息*/
	fprintf(stdout, "方向键控制视点.\n");
	fprintf(stdout, "按 \"w\" 切换线框模式.\n");
	fprintf(stdout, "按 \"R\" 或 \"r\" 来(分别)增加或降低移动速度.\n");

	glutMainLoop(); // 进入主循环(无限循环：不断检测事件处理事件)，此时窗口才会显示
}

