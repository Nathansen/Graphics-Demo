//------------------------------------------------------------------------------
//		Copyright(c) 2020 WarZhan zhanweilong1992@gmail.com
//		All rights reserved.
//		Use, modificationand distribution are subject to the "MIT License"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	用于演示相机变换
//	使用说明：
//	按 方向键 控制镜头逐步移动
//	按 W、S、A、D 键 控制镜头移动
//	按 ESC 键 退出
//------------------------------------------------------------------------------

#include "Angel.h"

const int NUM_SPHERES = 50;
const float MIN_POS = -20.f;
const float MAX_POS = 20.f;

typedef vec3 point3;

MatrixStack MVPStack;		// 模视投影矩阵栈
mat4 matProj;	// 投影矩阵
mat4 matCamera; // 相机变换矩阵

// 在每次 matCamera 左乘一个最新变换的时候
// matReverse都右乘一个最新变换的逆变换
// 这样可保证matReverse始终都是matCamera的逆矩阵
// 当前照相机在世界坐标系下的位置就是(matReverse[0][3], matReverse[1][3], matReverse[2][3])
mat4 matReverse; // 相机变换逆矩阵 用于计算相机位置

/*shader中变量索引*/
GLuint vPosition;  // shader中in变量vPosition的索引
GLuint MVPMatrix;  // shader中uniform变量MVPMatrix的索引

bool bUseLine = false; // 使用线框模式

// 地面
point3* ptGround;
GLuint numVerticesGround;
GLuint vaoGround;

// 球
point3* ptSphere;
GLuint numVerticesSphere;
GLuint vaoSphere;
point3 spheres[NUM_SPHERES];

GLfloat yRot = 0.0f;

// 圆环
point3* ptTorus;
GLuint numVerticesTorus;
GLuint vaoTorus;

enum { UP, DOWN, LEFT, RIGHT, NUM_KEY };
bool KeyDown[NUM_KEY];

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
	BuildGround(MAX_POS, 1.0);

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

// 创建球体函数：
// 用于生成一个中心在原点的球的顶点坐标数据(南北极在z轴方向)
// 参数为球的半径及经线和纬线数
void BuildSphere(GLfloat radius, GLsizei columns, GLsizei rows)
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
	if (ptSphere)
	{
		delete[] ptSphere;	// 如果sphere已经有数据，先回收

	}

	numVerticesSphere = rows * columns * 6; // 顶点数
	ptSphere = new point3[numVerticesSphere];

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
			ptSphere[index++] = vertices[ul];
			ptSphere[index++] = vertices[bl];
			ptSphere[index++] = vertices[br];
			ptSphere[index++] = vertices[ul];
			ptSphere[index++] = vertices[br];
			ptSphere[index++] = vertices[ur];
		}
	}

	delete[]vertices;
}

void InitSphere()
{
	for (int iSphere = 0; iSphere < NUM_SPHERES; iSphere++)
	{
		// 在 -20 到 20 以一米 为步长随机生成一个位置
		float x = (float)((rand() % 400) - 200) * 0.1f;
		float z = (float)((rand() % 400) - 200) * 0.1f;
		spheres[iSphere].x = x;
		spheres[iSphere].z = z;
	}

	BuildSphere(0.2, 15, 15);

	glGenVertexArrays(1, &vaoSphere);
	glBindVertexArray(vaoSphere);
	GLuint buffSphere;
	glGenBuffers(1, &buffSphere);
	glBindBuffer(GL_ARRAY_BUFFER, buffSphere);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(point3) * numVerticesSphere,
		ptSphere,
		GL_STATIC_DRAW);

	delete[] ptSphere;

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		BUFFER_OFFSET(0));
}

// 构建中心在原点的圆环(由三角形构建)
// 参数分别为圆环的主半径(决定环的大小)，圆环截面圆的半径(决定环的粗细)，
// numMajor和numMinor决定模型精细程度
void BuildTorus(GLfloat majorRadius, GLfloat minorRadius, GLint numMajor, GLint numMinor)
{
	if (ptTorus)
		delete[] ptTorus;	// 如果ptTorus已经有数据，先回收
	numVerticesTorus = numMajor * numMinor * 6; // 顶点数
	ptTorus = new point3[numVerticesTorus];

	double majorStep = 2.0f * M_PI / numMajor;
	double minorStep = 2.0f * M_PI / numMinor;

	int index = 0;
	for (int i = 0; i < numMajor; ++i)
	{
		double a0 = i * majorStep;
		double a1 = a0 + majorStep;
		GLfloat x0 = (GLfloat)cos(a0);
		GLfloat y0 = (GLfloat)sin(a0);
		GLfloat x1 = (GLfloat)cos(a1);
		GLfloat y1 = (GLfloat)sin(a1);

		for (int j = 0; j < numMinor; ++j)
		{
			double b0 = j * minorStep;
			double b1 = b0 + minorStep;
			GLfloat c0 = (GLfloat)cos(b0);
			GLfloat r0 = minorRadius * c0 + majorRadius;
			GLfloat z0 = minorRadius * (GLfloat)sin(b0);
			GLfloat c1 = (GLfloat)cos(b1);
			GLfloat r1 = minorRadius * c1 + majorRadius;
			GLfloat z1 = minorRadius * (GLfloat)sin(b1);

			point3 left0 = point3(x0 * r0, y0 * r0, z0);
			point3 right0 = point3(x1 * r0, y1 * r0, z0);
			point3 left1 = point3(x0 * r1, y0 * r1, z1);
			point3 right1 = point3(x1 * r1, y1 * r1, z1);
			ptTorus[index++] = left0;
			ptTorus[index++] = right0;
			ptTorus[index++] = left1;
			ptTorus[index++] = left1;
			ptTorus[index++] = right0;
			ptTorus[index++] = right1;
		}
	}
}

void InitTorus()
{
	BuildTorus(0.35, 0.15, 40, 20);

	glGenVertexArrays(1, &vaoTorus);
	glBindVertexArray(vaoTorus);

	GLuint buffTorus;
	glGenBuffers(1, &buffTorus);
	glBindBuffer(GL_ARRAY_BUFFER, buffTorus);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point3) * numVerticesTorus, ptTorus, GL_STATIC_DRAW);

	delete[] ptTorus;

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
	InitSphere();
	InitTorus();

	// 蓝色背景
	glClearColor(0.0f, 0.0f, .50f, 1.0f);
	// 以线框模式绘制
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// 开启深度检测
	glEnable(GL_DEPTH_TEST);
	// 开启背面剔除
	glEnable(GL_CULL_FACE);

	// 绘制光滑的直线
	glEnable(GL_LINE_SMOOTH); // 开启平滑线条
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST); // 设置为最佳效果
	glEnable(GL_BLEND); // 开启混合
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA); // 设置为混合模式
}

void SetLegalPos()
{
	// 相机移动范围 [-20, 20]
	if (matReverse[0][3] < MIN_POS)
	{
		float deltaX = MIN_POS - matReverse[0][3];
		matCamera = Translate(-deltaX, 0.0, 0.0) * matCamera;
		matReverse *= Translate(deltaX, 0.0, 0.0);
	}

	if (matReverse[0][3] > MAX_POS)
	{
		float deltaX = MAX_POS - matReverse[0][3];
		matCamera = Translate(-deltaX, 0.0, 0.0) * matCamera;
		matReverse *= Translate(deltaX, 0.0, 0.0);
	}

	if (matReverse[2][3] < MIN_POS)
	{
		float deltaX = MIN_POS - matReverse[2][3];
		matCamera = Translate(0.0, 0.0, -deltaX) * matCamera;
		matReverse *= Translate(0.0, 0.0, deltaX);
	}

	if (matReverse[2][3] > MAX_POS)
	{
		float deltaX = MAX_POS - matReverse[2][3];
		matCamera = Translate(0.0, 0.0, -deltaX) * matCamera;
		matReverse *= Translate(0.0, 0.0, deltaX);
	}
}

void UpdateCamera()
{
	// 在每次 matCamera 左乘一个最新变换的时候
	// matReverse 都右乘一个最新变换的逆变换
	// 这样可保证matReverse始终都是 matCamera 的逆矩阵
	// 当前照相机在世界坐标系下的位置就是(matReverse[0][3], matReverse[1][3], matReverse[2][3])
	if (KeyDown[UP])
	{
		matCamera = Translate(0.0, 0.0, 0.1) * matCamera;
		matReverse *= Translate(0.0, 0.0, -0.1);
	}
	if (KeyDown[DOWN])
	{
		matCamera = Translate(0.0, 0.0, -0.1) * matCamera;
		matReverse *= Translate(0.0, 0.0, 0.1);
	}
	if (KeyDown[LEFT])
	{
		matCamera = Translate(0.1, 0.0, 0.0) * matCamera;
		matReverse *= Translate(-0.1, 0.0, 0.0);
	}
	if (KeyDown[RIGHT])
	{
		matCamera = Translate(-0.1, 0.0, 0.0) * matCamera;
		matReverse *= Translate(0.1, 0.0, 0.0);
	}

	SetLegalPos();

	glutPostRedisplay();
}

// 显示回调函数
void RenderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	UpdateCamera();

	mat4 matMVP = matProj * matCamera;

	// 绘制地面
	MVPStack.push(matMVP);
	matMVP *= Translate(0.0, -0.4, 0.0);
	glBindVertexArray(vaoGround);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, matMVP);
	glDrawArrays(GL_LINES, 0, numVerticesGround);
	matMVP = MVPStack.pop();

	// 绘制球
	glBindVertexArray(vaoSphere);
	for (int iSphere = 0; iSphere < NUM_SPHERES; iSphere++)
	{
		MVPStack.push(matMVP);
		matMVP *= Translate(spheres[iSphere].x, 0.0, spheres[iSphere].z);
		matMVP *= Rotate(90, 1.0, 0.0, 0.0);
		glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, matMVP);
		glDrawArrays(GL_TRIANGLES, 0, numVerticesSphere);
		matMVP = MVPStack.pop();
	}

	matMVP *= Translate(0.0, 0.0, -2.5f);
	// 旋转的球
	MVPStack.push(matMVP);
	matMVP *= Rotate(yRot, 0.0f, 1.0f, 0.0f);
	matMVP *= Translate(1.0, 0.0f, 0.0f);
	matMVP *= Rotate(90, 1.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, matMVP);
	glDrawArrays(GL_TRIANGLES, 0, numVerticesSphere);
	matMVP = MVPStack.pop();

	// 圆环
	matMVP *= Rotate(-yRot, 0.0f, 1.0f, 0.0f);
	glBindVertexArray(vaoTorus);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, matMVP);
	glDrawArrays(GL_TRIANGLES, 0, numVerticesTorus);

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

void MyKeyDown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 'W':
		KeyDown[UP] = GL_TRUE;
		break;
	case 's':
	case 'S':
		KeyDown[DOWN] = GL_TRUE;
		break;
	case 'a':
	case 'A':
		KeyDown[LEFT] = GL_TRUE;
		break;
	case 'd':
	case 'D':
		KeyDown[RIGHT] = GL_TRUE;
		break;
	case 27:	// Esc键
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}

	SetLegalPos();

	glutPostRedisplay();
}

void MyKeyUp(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
	case 'W':
		KeyDown[UP] = GL_FALSE;
		break;
	case 's':
	case 'S':
		KeyDown[DOWN] = GL_FALSE;
		break;
	case 'a':
	case 'A':
		KeyDown[LEFT] = GL_FALSE;
		break;
	case 'd':
	case 'D':
		KeyDown[RIGHT] = GL_FALSE;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y)
{
	// 在每次 matCamera 左乘一个最新变换的时候
	// matReverse都右乘一个最新变换的逆变换
	// 这样可保证 matReverse 始终都是 matCamera 的逆矩阵
	// 当前照相机在世界坐标系下的位置就是(matReverse[0][3], matReverse[1][3], matReverse[2][3])
	switch (key)
	{
	case GLUT_KEY_UP:
		matCamera = Translate(0.0, 0.0, 0.1) * matCamera;
		matReverse *= Translate(0.0, 0.0, -0.1);
		break;
	case GLUT_KEY_DOWN:
		matCamera = Translate(0.0, 0.0, -0.1) * matCamera;
		matReverse *= Translate(0.0, 0.0, 0.1);
		break;
	case GLUT_KEY_LEFT:
		matCamera = Translate(0.1, 0.0, 0.0) * matCamera;
		matReverse *= Translate(-0.1, 0.0, 0.0);
		break;
	case GLUT_KEY_RIGHT:
		matCamera = Translate(-0.1, 0.0, 0.0) * matCamera;
		matReverse *= Translate(0.1, 0.0, 0.0);
		break;
	}
	glutPostRedisplay();
}

void TimerFunction(int value)
{
	yRot += 0.5f;
	if (yRot >= 360)
	{
		yRot -= 360;
	}

	glutPostRedisplay();
	glutTimerFunc(1000 / 60, TimerFunction, value);
}

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

	glutSpecialFunc(SpecialKeys);
	glutKeyboardFunc(MyKeyDown);
	glutKeyboardUpFunc(MyKeyUp);

	glutTimerFunc(100, TimerFunction, 0);
	Init();  // 初始化函数

	glutMainLoop();

	return 0;
}