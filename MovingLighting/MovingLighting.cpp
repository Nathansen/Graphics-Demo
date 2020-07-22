//------------------------------------------------------------------------------
//		Copyright(c) 2020 WarZhan zhanweilong1992@gmail.com
//		All rights reserved.
//		Use, modificationand distribution are subject to the "MIT License"
//------------------------------------------------------------------------------

#include "Angel.h"

const int NUM_SPHERES = 50;
const float MIN_POS = -20.f;
const float MAX_POS = 20.f;

typedef vec3 point3;
typedef vec4 color4;

typedef struct lightingStruct
{
	color4 ambient;	 // 环境光
	color4 diffuse;  // 漫反射光
	color4 specular; // 镜面高光
}lightingStruct;

typedef struct materialStruct {
	color4 ambient;		// 环境反射系数
	color4 diffuse;		// 漫反射系数
	color4 specular;	// 镜面反射系数
	color4 emission;	// 发射光
	GLfloat shininess;	// 高光系数
}materialStruct;

// 白光
lightingStruct whileLight =
{
	color4(0.2, 0.2, 0.2, 1.0),
	color4(1.0, 1.0, 1.0, 1.0),
	color4(1.0, 1.0, 1.0, 1.0)
};

lightingStruct redLight =
{
	color4(0.2, 0.0, 0.0, 1.0),
	color4(1.0, 1.0, 1.0, 1.0),
	color4(1.0, 1.0, 1.0, 1.0)
};

lightingStruct yellowLight =
{
	color4(0.2, 0.0, 0.0, 1.0),
	color4(1.0, 1.0, 1.0, 1.0),
	color4(1.0, 1.0, 1.0, 1.0)
};

lightingStruct Lights[] =
{
	whileLight,
	redLight,
	yellowLight
};

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
GLuint vNormal;
GLuint ModelView; // shader中uniform变量 ModelView 的索引
GLuint Projection;  // shader中uniform变量 Projection 的索引
GLuint AmbienProduct;
GLuint DiffuseProduct;
GLuint SpecularProduct;
GLuint Shininess;
GLuint Emission;
GLuint LightPosition;

bool bUseLine = false; // 使用线框模式

// 地面
point3* ptGround;
vec3* nGround;
GLuint numVerticesGround;
GLuint vaoGround;

materialStruct materialGround =
{
	color4(0.1, 0.1, 0.1, 1.0),
	color4(0.8, 0.8, 0.8, 1.0),
	color4(0.3, 0.3, 0.3, 1.0),
	color4(0.0, 0.0, 0.0, 0.0),
	10
};

materialStruct redLightMaterial =
{
	color4(0.1, 0.1, 0.1, 1.0),
	color4(0.2, 0.2, 0.2, 1.0),
	color4(0.2, 0.2, 0.2, 1.0),
	color4(1.0, 0.0, 0.0, 0.0),
	150
};

// 球
point3* ptSphere;
GLuint numVerticesSphere;
GLuint vaoSphere;
point3 spheres[NUM_SPHERES];

GLfloat yRot = 0.0f;

// 圆环
point3* ptTorus;
vec3* nTorus;
GLuint numVerticesTorus;
GLuint vaoTorus;

enum { UP, DOWN, LEFT, RIGHT, NUM_KEY };
bool KeyDown[NUM_KEY];

// 构建 y = 0 且中心为原点平面
// fExtent 地面范围
// fStep 间隔
void BuildGround(GLfloat fExtent, GLfloat fStep)
{
	int iterations = 2 * fExtent / fStep;
	numVerticesGround = iterations * iterations * 6;
	if (NULL != ptGround)
	{
		delete[] ptGround;
	}
	ptGround = new point3[numVerticesGround];

	if (NULL != nGround)
	{
		delete[] nGround;
	}
	nGround = new vec3[numVerticesGround];
	vec3 defaultNormal = vec3(0.0, 1.0, 0.0); // 默认向上
	int index = 0;
	for (GLint x = -fExtent; x < fExtent; x += fStep)
	{
		for (GLint z = fExtent; z > -fExtent; z -= fStep)
		{
			point3 ptLowerLeft = point3(x, 0, z);
			point3 ptLowerRight = point3(x + fStep, 0, z);
			point3 ptUpperLeft = point3(x, 0, z - fStep);
			point3 ptUpperRight = point3(x + fStep, 0, z - fStep);

			nGround[index] = defaultNormal;
			ptGround[index++] = ptUpperLeft;

			nGround[index] = defaultNormal;
			ptGround[index++] = ptLowerLeft;

			nGround[index] = defaultNormal;
			ptGround[index++] = ptLowerRight;

			nGround[index] = defaultNormal;
			ptGround[index++] = ptUpperLeft;

			nGround[index] = defaultNormal;
			ptGround[index++] = ptLowerRight;

			nGround[index] = defaultNormal;
			ptGround[index++] = ptUpperRight;
		}
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
		(sizeof(point3) + sizeof(vec3)) * numVerticesGround,
		NULL,
		GL_STATIC_DRAW);

	glBufferSubData(GL_ARRAY_BUFFER,
		0,
		sizeof(point3) * numVerticesGround,
		ptGround);

	delete[] ptGround;

	glBufferSubData(GL_ARRAY_BUFFER,
		sizeof(point3) * numVerticesGround,
		sizeof(vec3) * numVerticesGround,
		nGround);

	delete[] nGround;

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * numVerticesGround));
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

	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal,
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
	numVerticesTorus = numMajor * numMinor * 6; // 顶点数

	if (NULL != ptTorus)
	{
		delete[] ptTorus;	// 如果ptTorus已经有数据，先回收
	}
	ptTorus = new point3[numVerticesTorus];

	if (NULL != nTorus)
	{
		delete[] nTorus;
	}
	nTorus = new vec3[numVerticesTorus];

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

		point3 center0 = majorRadius * point3(x0, y0, 0);
		point3 center1 = majorRadius * point3(x1, y1, 0);

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
			nTorus[index] = left0 - center0;
			ptTorus[index++] = left0;

			nTorus[index] = right0 - center1;
			ptTorus[index++] = right0;

			nTorus[index] = left1 - center0;
			ptTorus[index++] = left1;

			nTorus[index] = left1 - center0;
			ptTorus[index++] = left1;

			nTorus[index] = right0 - center1;
			ptTorus[index++] = right0;

			nTorus[index] = right1 - center1;
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
	glBufferData(GL_ARRAY_BUFFER, (sizeof(point3) + sizeof(vec3)) * numVerticesTorus, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point3) * numVerticesTorus, ptTorus);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(point3) * numVerticesTorus,
		sizeof(vec3) * numVerticesTorus, nTorus);

	delete[] ptTorus;
	delete[] nTorus;

	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point3) * numVerticesTorus));
}


// 初始化函数
void Init()
{
	/*加载shader并使用所得到的shader程序*/
	// InitShader为InitShader.cpp中定义的函数，参数分别为顶点和片元shader的文件名
	// 返回值为shader程序对象的ID
	GLuint program = InitShader("vPhong.glsl", "fPhong.glsl");
	glUseProgram(program); // 使用该shader程序
	// 获取shader程序中属性变量的位置(索引)
	vPosition = glGetAttribLocation(program, "vPosition");
	vNormal = glGetAttribLocation(program, "vNormal");
	ModelView = glGetUniformLocation(program, "ModelView");
	// 获取shader中uniform变量"Projection"的索引
	Projection = glGetUniformLocation(program, "Projection");

	AmbienProduct = glGetUniformLocation(program, "AmbienProduct");
	DiffuseProduct = glGetUniformLocation(program, "DiffuseProduct");
	SpecularProduct = glGetUniformLocation(program, "SpecularProduct");
	Shininess = glGetUniformLocation(program, "Shininess");
	Emission = glGetUniformLocation(program, "Emission");
	LightPosition = glGetUniformLocation(program, "LightPosition");

	// 设置手电筒位置
	glUniform4fv(LightPosition + 2, 1, vec4(0.0f, 0.0f, 0.0f, 1.0f));
	// 聚光灯照射方向(观察坐标系)
	glUniform3fv(glGetUniformLocation(program, "SpotDirection"),
		1, vec3(0.0f, 0.0f, -1.0f));
	// 聚光灯截止角(角度)
	glUniform1f(glGetUniformLocation(program, "SpotCutOff"), 8);
	// 衰减指数
	glUniform1f(glGetUniformLocation(program, "SpotExponent"), 3);

	InitGround();
	InitSphere();
	InitTorus();

	// 蓝色背景
	glClearColor(0.0f, 0.0f, .50f, 1.0f);

	// 以线框模式绘制
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// 开启深度检测
	glEnable(GL_DEPTH_TEST);
	// 开启背面剔除
	glEnable(GL_CULL_FACE);
}

void SetMaterial(const int lightNum, const materialStruct& material, const lightingStruct light[])
{
	for (int i = 0; i < lightNum; i++)
	{
		glUniform4fv(AmbienProduct + i, 1, material.ambient * light[i].ambient);
		glUniform4fv(DiffuseProduct + i, 1, material.diffuse * light[i].diffuse);
		glUniform4fv(SpecularProduct + i, 1, material.specular * light[i].specular);
	}

	glUniform4fv(Emission, 1, material.emission);
	glUniform1f(Shininess, material.shininess);
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

	//mat4 matMVP = matProj * matCamera;
	mat4 matModelView = matCamera;
	// 光源 1 位置
	vec4 lightPos(1.0, 1.0, 1.0, 0.0);
	glUniform4fv(LightPosition, 1, matModelView * lightPos);

	// 绘制地面
	MVPStack.push(matModelView);
	matModelView *= Translate(0.0, -0.4, 0.0);
	SetMaterial(3, materialGround, Lights);
	glBindVertexArray(vaoGround);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, matModelView);
	glDrawArrays(GL_TRIANGLES, 0, numVerticesGround);
	matModelView = MVPStack.pop();

	// 绘制球
	glBindVertexArray(vaoSphere);
	//SetMaterial(3, sh, Lights);
	for (int iSphere = 0; iSphere < NUM_SPHERES; iSphere++)
	{
		MVPStack.push(matModelView);
		matModelView *= Translate(spheres[iSphere].x, 0.0, spheres[iSphere].z);
		matModelView *= Rotate(90, 1.0, 0.0, 0.0);
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, matModelView);
		glDrawArrays(GL_TRIANGLES, 0, numVerticesSphere);
		matModelView = MVPStack.pop();
	}

	matModelView *= Translate(0.0, 0.0, -2.5f);
	// 旋转的球
	MVPStack.push(matModelView);
	matModelView *= Rotate(yRot, 0.0f, 1.0f, 0.0f);
	matModelView *= Translate(1.0, 0.0f, 0.0f);
	// 设置第二个光源位置
	glUniform4fv(LightPosition + 1, 1, matModelView * vec4(0.0f, 0.0f, 0.0f, 1.0f));

	matModelView *= Rotate(90, 1.0f, 0.0f, 0.0f);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, matModelView);
	glDrawArrays(GL_TRIANGLES, 0, numVerticesSphere);
	matModelView = MVPStack.pop();

	// 圆环
	matModelView *= Rotate(-yRot, 0.0f, 1.0f, 0.0f);
	glBindVertexArray(vaoTorus);
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, matModelView);
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
	glUniformMatrix4fv(Projection, 1, GL_TRUE, matProj);
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
	case 'l':
	case 'L':
		bUseLine != bUseLine;
		// 切换线框 与 填充模式
		if (bUseLine)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
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