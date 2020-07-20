//------------------------------------------------------------------------------
//		Copyright(c) 2020 WarZhan zhanweilong1992@gmail.com
//		All rights reserved.
//		Use, modificationand distribution are subject to the "MIT License"
//------------------------------------------------------------------------------

#include "Angel.h"

typedef vec3 point3;

MatrixStack MVPStack;		// 模视投影矩阵栈
mat4 matProj;	// 投影矩阵

/*shader中变量索引*/
GLuint vPosition;  // shader中in变量vPosition的索引
GLuint MVPMatrix;  // shader中uniform变量MVPMatrix的索引

bool bUseLine = false; // 使用线框模式

// 地面
point3* ptGround;
GLuint numVerticesGround;
GLuint vaoGround;

// 构建 y = 0 且中心为原点平面
// fExtent 地面范围
// fStep 间隔
void BuildGround(GLfloat fExtent, GLfloat fStep)
{
	numVerticesGround = (2 * fExtent / fStep + 1) * 4;
	ptGround = new point3[numVerticesGround];
	int index = 0;
	for (GLint iLine = -fExtent; iLine <= fExtent; iLine += fStep)
	{
		ptGround[index++] = point3(iLine, 0, fExtent);
		ptGround[index++] = point3(iLine, 0, -fExtent);
		ptGround[index++] = point3(fExtent, 0, iLine);
		ptGround[index++] = point3(-fExtent, 0, iLine);
	}
}

void InitGround()
{
	BuildGround(20.0, 1.0);

	glGenVertexArrays(1, &vaoGround);
	glBindVertexArray(vaoGround);

	GLuint buffGround;
	glGenBuffers(1, &buffGround);
	glBindBuffer(GL_ARRAY_BUFFER, buffGround);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(point3) * numVerticesGround,
		ptGround,
		GL_STATIC_DRAW);

	delete[] ptGround;

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
}



// 初始化函数
void Init()
{
	/*加载shader并使用所得到的shader程序*/
	// InitShader为InitShader.cpp中定义的函数，参数分别为顶点和片元shader的文件名
	// 返回值为shader程序对象的ID
	GLuint program = InitShader("vShader.glsl", "fShader.glsl");
	glUseProgram(program); // 使用该shader程序
	// 获取shader程序中属性变量的位置(索引)
	vPosition = glGetAttribLocation(program, "vPosition");
	// 获取shader中uniform变量"MVPMatrix"的索引
	MVPMatrix = glGetUniformLocation(program, "MVPMatrix");

	InitGround();

	// 蓝色背景
	glClearColor(0.0f, 0.0f, .50f, 1.0f);
	// 以线框模式绘制
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// 开启深度检测
	glEnable(GL_DEPTH_TEST);
	// 开启背面剔除
	glEnable(GL_CULL_FACE);
}

// 显示回调函数
void RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 matMVP = matProj;

	// 绘制地面
	matMVP *= Translate(0.0, -0.4, 0.0);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, matMVP);
	glDrawArrays(GL_LINES, 0, numVerticesGround);

	// 交换缓存
	glutSwapBuffers();
}

// 窗口调整回调函数
void ChangeSize(int w, int h)
{
	// 防止除0
	if (h == 0)
		h = 1;

	glViewport(0, 0, w, h);		// 视口占满整个窗口

	GLfloat fAspect = (GLfloat)w / (GLfloat)h;	// 计算窗口宽高比

	// 设置透视投影视域体
	matProj = Perspective(35.0f, fAspect, 1.0f, 50.0f);
}

//void MyKeyDown(unsigned char key, int x, int y)
//{
//	switch (key)
//	{
//	case 'w':
//	case 'W':
//		KeyDown[UP] = GL_TRUE;
//		break;
//	default:
//		break;
//	}
//}

int main(int argc, char* argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(800, 600);

	// 以下2个函数来自freeglut库，用于确认 
	// 代码是基于OpenGL 3.1版本的
	glutInitContextVersion(3, 1); // 表明使用OpenGL 3.1
	// 保持向前兼容，即不使用任何弃用的函数
	// 如程序中使用了弃用函数则注释掉本行
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutCreateWindow("Moving Camera");

	// 显卡驱动非正式发布版或者与glew库规范不兼容时加上此行
	// 如果在glGenVertexArrays处发生Access Violation则加上此行
	glewExperimental = GL_TRUE;

	GLenum err = glewInit(); // 初始化glew库，必须在glutCreateWindow之后调用
	if (err != GLEW_OK)  // 初始化不成功？
	{
		std::cout << "glewInit 失败, 退出程序." << std::endl;
		exit(EXIT_FAILURE); // 退出程序
	}

	/*注册回调函数*/
	glutReshapeFunc(ChangeSize);
	glutDisplayFunc(RenderScene);

	Init();  // 初始化函数

	glutMainLoop();

	return 0;
}