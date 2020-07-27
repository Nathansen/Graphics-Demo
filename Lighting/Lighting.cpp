//------------------------------------------------------------------------------
//		Copyright(c) 2020 WarZhan zhanweilong1992@gmail.com
//		All rights reserved.
//		Use, modificationand distribution are subject to the "MIT License"
//------------------------------------------------------------------------------

#include "Angel.h"

typedef vec3 point3;
typedef vec4 color4;

//typedef struct lightingStruct {
//	color4 ambient;
//	color4 diffuse;
//	color4 specular;
//}lightingStruct;

//typedef struct materialStruct {
//	color4 ambient;		// 环境反射系数
//	color4 diffuse;		// 漫反射系数
//	color4 specular;	// 镜面反射系数
//	color4 emission;	// 发射光
//	GLfloat shininess;	// 高光系数
//}materialStruct;

point3* sphere;			// 存放一个球的顶点坐标数据的指针
GLsizei NumVertices;	// 球的顶点数

float RotateAngle = 0.0f;		// 绕y轴旋转的角度
float AngleStepSize = 10.0f;

GLuint vPosition;	// Shader 中 in 变量 vPosition 的索引
GLuint vNormal;		// Shader 中 vNormal 的索引
GLuint PMatrix;	// Shader 中 uniform 变量 PMatrix 的索引
GLuint MVMatrix;	// Shader 中 uniform 变量 MVMatrix 的索引
GLuint LigithPos;	// Shader 中 uniform 变量 LigithPos 的索引
GLuint ViewPos;	// Shader 中 uniform 变量 ViewPos 的索引
GLuint blinn;		// Shader 中 uniform 变量 blinn 的索引
GLuint bAmbieni; // 使用环境光
GLuint bDiffuse; // 使用漫反射
GLuint bSpecular; // 使用 高光

GLuint uColor;		// Shader 中 uniform 变量 uColor 的索引

mat4 proj;	// 投影矩阵

bool useBlinnPhong = false; // Blinn-Phong
bool useLine = false; // 线框模式
bool useAmbieni; // 使用环境光
bool useDiffuse; // 使用漫反射
bool useSpecular; // 使用 高光

//环境光1
//lightingStruct whiteLight = {
//	color4(0.2, 0.2, 0.2, 1.0),
//	color4(1.0, 1.0, 1.0, 1.0),
//	color4(1.0, 1.0, 1.0, 1.0)
//};

// 光源位置
vec3 lightPos = vec3(2.0f, 2.0f, 4.0f);
//vec3 viewPos = vec3(0.0f, 0.0f, -15.0f);


MatrixStack matStack;

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

void InitSphere(GLuint numVertices)
{
	GLuint vaoSphere;

	glGenVertexArrays(1, &vaoSphere);
	glBindVertexArray(vaoSphere);
	GLuint buffSphere;
	glGenBuffers(1, &buffSphere);
	glBindBuffer(GL_ARRAY_BUFFER, buffSphere);
	glBufferData(GL_ARRAY_BUFFER,
		sizeof(point3) * numVertices,
		sphere,
		GL_STATIC_DRAW);

	delete[] sphere;

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		BUFFER_OFFSET(0));

	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal,
		3,
		GL_FLOAT,
		GL_FALSE,
		0,
		BUFFER_OFFSET(0));

	///*创建一个顶点数组对象(VAO)*/
	//glGenVertexArrays(1, &vaoSphere);  // 生成一个未用的VAO ID，存于变量vao中
	//glBindVertexArray(vaoSphere);      // 创建id为vao的VAO，并绑定为当前VAO

	///*创建并初始化一个缓冲区对象(Buffer Object)*/
	//GLuint buffer;
	//glGenBuffers(1, &buffer); // 生成一个未用的缓冲区对象ID，存于变量buffer中
	//// 创建id为buffer的Array Buffer对象，并绑定为当前Array Buffer对象
	//glBindBuffer(GL_ARRAY_BUFFER, buffer);
	//// 为Buffer对象在GPU端申请空间，并提供数据
	//glBufferData(GL_ARRAY_BUFFER,	// Buffer类型
	//	(sizeof(point3) + sizeof(vec3)) * numVertices,  // 申请空间大小(顶点 + 法线)
	//	NULL,			 // 暂时不提供数据
	//	GL_STATIC_DRAW	// 表明将如何使用Buffer的标志(GL_STATIC_DRAW含义是一次提供数据，多遍绘制)
	//);

	//// 填充顶点
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * numVertices, sphere);
	//// 填充法线 球的顶点和法线数据一样 直接复用顶点
	//glBufferSubData(GL_ARRAY_BUFFER,
	//	sizeof(point3) * numVertices,
	//	sizeof(vec3) * numVertices,
	//	sphere);

	//delete[] sphere; // 数据已传到 GPU，可删除顶点数据


	//glEnableVertexAttribArray(vPosition);	// 启用顶点属性数组
	//// 为顶点属性数组提供数据(数据存放在之前buffer对象中)
	//glVertexAttribPointer(
	//	vPosition,			// 属性变量索引
	//	3,					// 每个顶点属性的分量个数
	//	GL_FLOAT,			// 数组数据类型
	//	GL_FALSE,			// 是否进行归一化处理
	//	0,  // 在数组中相邻属性成员起始位置间的间隔(以字节为单位)
	//	BUFFER_OFFSET(0)    // 第一个属性值在buffer中的偏移量
	//);

	//glEnableVertexAttribArray(vNormal);
	//glVertexAttribPointer(
	//	vNormal,			// 属性变量索引
	//	3,					// 每个顶点属性的分量个数
	//	GL_FLOAT,			// 数组数据类型
	//	GL_FALSE,			// 是否进行归一化处理
	//	0,  // 在数组中相邻属性成员起始位置间的间隔(以字节为单位)
	//	BUFFER_OFFSET(sizeof(point3) * numVertices)    // 第一个属性值在buffer中的偏移量
	//);
}

// 初始化OpenGL的状态
void Init(void)
{
	// 生成中心在原点半径为1,15条经线和纬线的球的顶点
	NumVertices = BuildSphere(1.0, 15, 15);

	/*加载shader并使用所得到的shader程序*/
	// InitShader为InitShader.cpp中定义的函数，参数分别为顶点和片元shader的文件名
	// 返回值为shader程序对象的ID
	GLuint program = InitShader("vPhongTest.glsl", "fPhongTest.glsl");
	glUseProgram(program); // 使用该shader程序

	/*初始化顶点着色器中的顶点位置属性*/
	// 获取shader程序中属性变量的位置(索引)
	//vPosition = glGetAttribLocation(program, "vPosition");
	//vNormal = glGetAttribLocation(program, "vNormal");
	vPosition = glGetAttribLocation(program, "vPosition");
	vNormal = glGetAttribLocation(program, "vNormal");
	InitSphere(NumVertices);

	// 获取shader中uniform变量的索引
	PMatrix = glGetUniformLocation(program, "PMatrix");
	MVMatrix = glGetUniformLocation(program, "MVMatrix");
	LigithPos = glGetUniformLocation(program, "LigithPos");
	ViewPos = glGetUniformLocation(program, "ViewPos");
	blinn = glGetUniformLocation(program, "blinn");
	bAmbieni = glGetUniformLocation(program, "bAmbieni");
	bDiffuse = glGetUniformLocation(program, "bDiffuse");
	bSpecular = glGetUniformLocation(program, "bSpecular");

	// 获取shader中uniform变量"uColor"的索引
	//uColor = glGetUniformLocation(program, "uColor");

	glClearColor(0.5, 0.5, 0.5, 0.0);		// 背景为白色			
	glEnable(GL_DEPTH_TEST);				// 开启深度检测

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	// 线框模式

	glCullFace(GL_BACK);		// 剔除背面	
	glEnable(GL_CULL_FACE);	// 开启面剔除，默认剔除背面
}

// 窗口调整回调函数，当窗口大小发生变化时被调用
void ResizeWindow(int w, int h)
{
	h = (h == 0) ? 1 : h;
	w = (w == 0) ? 1 : w;
	glViewport(0, 0, w, h);	// 视口占满整个窗口
	float aspectRatio = (float)w / (float)h;

	// 设置投影矩阵
	proj = Perspective(30.0, aspectRatio, 1.0, 30.0);
}

// 显示回调函数
void Display(void)
{
	// 用背景色清空颜色缓存，将深度缓存恢复为初始值
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mat4 mv = Translate(0.0f, 0.0f, -15.0f);
	mv *= Rotate(RotateAngle, 1.0, 0.0, 0.0);

	matStack.push(mv);
	//mv *= Translate(viewPos) * RotateY(RotateAngle); // 构建 View Matrix
	mv *= Rotate(90, 1.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(MVMatrix, 1, GL_TRUE, mv);
	glUniformMatrix4fv(PMatrix, 1, GL_TRUE, proj); // 传模视投影矩阵
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	mv = matStack.pop();

	vec4 mvLightPos = mv * lightPos;
	glUniform3f(LigithPos, mvLightPos.x, mvLightPos.y, mvLightPos.z);

	//mv *= Translate(lightPos.x, lightPos.y, lightPos.z);
	//// 在光源位置绘制一个球
	////glUniform3f(LigithPos, 0, 0, 0);
	//glUniformMatrix4fv(MVMatrix, 1, GL_TRUE, mv * Scale(0.1, 0.1, 0.1));
	//glUniformMatrix4fv(PMatrix, 1, GL_TRUE, proj); // 传模视投影矩阵
	//glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	// 观察者位于原点
	glUniform3f(ViewPos, 0, 0, 0);

	glutSwapBuffers();					// 交换缓存
}

void KeyPressFunc(unsigned char Key, int x, int y)
{
	switch (Key)
	{
	case '1':
		useAmbieni = !useAmbieni;
		glUniform1i(bAmbieni, useAmbieni ? 1 : 0);
		break;
	case '2':
		useDiffuse = !useDiffuse;
		glUniform1i(bDiffuse, useDiffuse ? 1 : 0);
		break;
	case '3':
		useSpecular = !useSpecular;
		glUniform1i(bSpecular, useSpecular ? 1 : 0);
		break;
	case 't':
	case 'T':
		useBlinnPhong = !useBlinnPhong;
		glUniform1i(blinn, useBlinnPhong ? 1 : 0);
		break;
	case 'l':
	case 'L':
		useLine = !useLine;
		if (useLine)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	// 线框模式
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		break;
	case 27:	// Esc键
		exit(EXIT_SUCCESS);
		break;
	default:
		break;
	}

	glutPostRedisplay();
}

/*glutSpecialFunc设置以下函数用于处理“特殊”按键事件*/
void SpecialKeyFunc(int key, int x, int y)
{
	switch (key)
	{
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



int main(int argc, char** argv)
{
	glutInit(&argc, argv);	// 初始化GLUT库，必须在其他glut函数前执行

	// 以下初始化窗口属性和上下文(Context)的函数必须在glutCreateWindow之前调用
	// 初始化显示模式，使用RGB颜色模式、双缓存和多重采样
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

	// 以下2个函数来自freeglut库，用于确认 
	// 代码是基于OpenGL 3.1版本的
	glutInitContextVersion(3, 1); // 表明使用OpenGL 3.1
	// 保持向前兼容，即不使用任何弃用的函数
	// 如程序中使用了弃用函数则注释掉本行
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitWindowSize(900, 540);

	glutCreateWindow("Lighting");	// 创建窗口，标题替换成自己的

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
	// 设置键盘按键的回调函数
	glutKeyboardFunc(KeyPressFunc);
	glutSpecialFunc(SpecialKeyFunc);

	// 设置窗口调整的回调函数
	glutReshapeFunc(ResizeWindow);

	// 设置显示回调函数
	glutDisplayFunc(Display);

	// 自定义的初始化函数
	// 通常将shader初始化和在程序运行过程中保持不变的属性的设置放在此函数中
	Init();

	glutMainLoop(); // 进入主循环(无限循环：不断检测事件处理事件)，此时窗口才会显示
	return 0;
}
