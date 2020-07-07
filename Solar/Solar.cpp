//------------------------------------------------------------------------------
//		Copyright(c) 2020 WarZhan zhanweilong1992@gmail.com
//		All rights reserved.
//		Use, modificationand distribution are subject to the "MIT License"
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//	用于演示变换和坐标系
//	画了一个简单的太阳系，包括一个太阳，一个地球，一个地球同步卫星和一个月球
//	使用说明：
//	按 "r" 键来启动和停止动画
//	按 "s" 键单步执行动画
//	按 "t" 键切换透视和俯视图
//	方向键 上下箭头 用于控制动画中每一帧的时间间隔,每次按键时间间隔乘2或除2
//	按ESC键退出
//------------------------------------------------------------------------------

#include "Angel.h"

typedef vec3 point3;

GLenum spinMode = GL_TRUE;		// 控制动画运行和暂停
GLenum singleStep = GL_FALSE;	// 控制单步执行模式开启和关闭

GLuint vPosition;  // shader中in变量vPosition的索引

GLuint vaoSphere;
GLuint vaoRing;

// 这3个变量控制动画的状态和速度
float HourOfDay = 0.0;			// 一天中的小时数，初始为0
float DayOfYear = 0.0;			// 一年中的天数，初始为0
float AnimateIncrement = 24.0;  // 动画步长变量，表示时间间隔, 以小时为单位

float ErothAxialAngle = -23.44; // 地球轴与 Y 轴夹角
float ViewAngle = 15.0; // 观察角度

MatrixStack mvStack;  // 模视矩阵栈
mat4 proj;	// 投影矩阵

GLuint MVPMatrix;	// Shader中uniform变量"MVPMatrix"的索引
GLuint uColor;		// Shader中uniform变量"uColor"的索引

GLsizei NumVertices;	// 一个球的顶点数
point3* sphere;			// 存放一个球的顶点坐标数据的指针

GLsizei NumRing = 72; // 一个环的顶点数
point3* ring; // 存放一个环的顶点数

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
	NumVertices = rows * columns * 6; // 顶点数
	sphere = new point3[NumVertices];

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

	return NumVertices;
}

void BuildRing(GLfloat radius, GLsizei num)
{
	int index = 0;	// 数组索引
	/*生成最终顶点数组数据*/
	if (ring)
	{
		delete[] ring;	// 如果 ring 已经有数据，先回收
	}
	ring = new point3[num]; // 存放不同顶点的数组
	NumRing = num;

	for (int i = 0; i < num; i++)
	{
		float v = (float)i / (float)num;  // [0,1] 单前第几段
		float theta1 = v * 2.0 * (float)M_PI;	   // [0,PI]

		point3 n(0, 0, 0);
		GLfloat cosTheta1 = cos(theta1);
		GLfloat sinTheta1 = sin(theta1);
		n.x = radius * cosTheta1;
		n.y = radius * sinTheta1;

		ring[i] = n;
	}

}

// 显示回调函数
void Animate(void)
{
	// 用背景色清空颜色缓存，将深度缓存恢复为初始值
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (spinMode) {				// 如果动画开启
		// 更新动画状态
		HourOfDay += AnimateIncrement;
		DayOfYear += AnimateIncrement / 24.0;

		// 防止溢出
		HourOfDay = HourOfDay - ((int)(HourOfDay / 24)) * 24;
		DayOfYear = DayOfYear - ((int)(DayOfYear / 365)) * 365;
	}

	mat4 mv;	// 定义模视矩阵，默认初始化为恒等矩阵

	// 在观察坐标系(照相机坐标系)下思考，定位整个场景(第一种观点)或世界坐标系(第二种观点)
	// 向负z轴方向平移15个单位
	mv *= Translate(0.0, 0.0, -15.0);

	// 将太阳系绕x轴旋转15度以便在xy-平面上方观察
	mv *= Rotate(ViewAngle, 1.0, 0.0, 0.0);
	//mv *= Rotate(90.0, 1.0, 0.0, 0.0); // 俯视角度

	/*下面开始构建整个3D世界，在世界坐标系下考虑问题*/
	// 太阳直接画在原点，无须变换，用一个黄色的球体表示
	mvStack.push(mv); // 保存矩阵状态
	mv *= Rotate(90.0, 1.0, 0.0, 0.0);
	glBindVertexArray(vaoSphere);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, proj * mv * Scale(0.8, 0.8, 0.8)); // 传模视投影矩阵
	glUniform3f(uColor, 1.0, 1.0, 0.0);  // 黄色
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	mv = mvStack.pop();

	// 绘制地球轨道
	mvStack.push(mv);
	mv *= Rotate(90.0, 1.0, 0.0, 0.0);
	glBindVertexArray(vaoRing);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, proj * mv * Scale(4, 4, 4)); // 传模视投影矩阵
	glUniform3f(uColor, 0.0, 0.0, 1.0);  // 蓝色
	glDrawArrays(GL_LINE_LOOP, 0, NumRing);
	mv = mvStack.pop();

	/*对地球系统定位，绕太阳放置它*/
	// 用DayOfYear来控制其绕太阳的旋转
	mv *= Rotate(360.0 * DayOfYear / 365.0, 0.0, 1.0, 0.0);
	mv *= Translate(4.0, 0.0, 0.0);

	/*下面开始在地球系统的小世界坐标系下考虑问题*/
	// 绘制地球，地球的自转不应该影响月球
	mvStack.push(mv); // 保存矩阵状态

	// 抵消公转对自身倾斜方向的影响，保证公转后 仍然向右倾斜
	mv *= Rotate(-360.0 * DayOfYear / 365.0, 0.0, 1.0, 0.0);

	// 地球向 右倾斜 23.44度
	mv *= Rotate(ErothAxialAngle, 0.0, 0.0, 1.0);
	// 地球自转，用HourOfDay进行控制
	mv *= Rotate(360.0 * HourOfDay / 24.0, 0.0, 1.0, 0.0);
	mv *= Rotate(90.0, 1.0, 0.0, 0.0);
	// 最后，画一个蓝色的球来表示地球
	glBindVertexArray(vaoSphere);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, proj * mv * Scale(0.4, 0.4, 0.4)); // 传模视投影矩阵
	glUniform3f(uColor, 0.2, 0.2, 1.0);  // 蓝色
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	mv = mvStack.pop(); // 恢复矩阵状态

	// 绘制地球同步卫星轨道
	mvStack.push(mv);
	// 抵消地球公转对自身倾斜方向的影响，保证公转后 仍然向右倾斜
	mv *= Rotate(-360.0 * DayOfYear / 365.0, 0.0, 1.0, 0.0);

	mv *= Rotate(ErothAxialAngle, 0.0, 0.0, 1.0);
	mv *= Rotate(90.0, 1.0, 0.0, 0.0);
	glBindVertexArray(vaoRing);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, proj * mv * Scale(0.5, 0.5, 0.5));
	glUniform3f(uColor, 0.5, 0.0, 0.5);  // 紫色
	glDrawArrays(GL_LINE_LOOP, 0, NumRing);
	mv = mvStack.pop();

	// 地球同步卫星
	mvStack.push(mv);
	// 抵消地球公转对自身倾斜方向的影响，保证公转后 仍然向右倾斜
	mv *= Rotate(-360.0 * DayOfYear / 365.0, 0.0, 1.0, 0.0);

	mv *= Rotate(ErothAxialAngle, 0.0, 0.0, 1.0);
	mv *= Rotate(360.0 * HourOfDay / 24.0, 0.0, 1.0, 0.0); // 旋转速度与地球相同
	mv *= Translate(0.5, 0.0, 0.0);
	mv *= Rotate(90.0, 1.0, 0.0, 0.0);
	glBindVertexArray(vaoSphere);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, proj * mv * Scale(0.05, 0.05, 0.05)); // 传模视投影矩阵
	glUniform3f(uColor, 0.5, 0.0, 0.5);  // 紫色
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
	mv = mvStack.pop();

	// 绘制月球轨道
	mvStack.push(mv);
	mv *= Rotate(90.0, 1.0, 0.0, 0.0);
	glBindVertexArray(vaoRing);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, proj * mv * Scale(0.7, 0.7, 0.7));
	glUniform3f(uColor, 0.3, 0.7, 0.3);
	glDrawArrays(GL_LINE_LOOP, 0, NumRing);
	mv = mvStack.pop();

	/*画月球*/
	// 用DayOfYear来控制其绕地球的旋转
	mv *= Rotate(360.0 * 12.0 * DayOfYear / 365.0, 0.0, 1.0, 0.0);
	mv *= Translate(0.7, 0.0, 0.0);
	mv *= Scale(0.1, 0.1, 0.1);
	mv *= Rotate(90.0, 1.0, 0.0, 0.0);
	glBindVertexArray(vaoSphere);
	glUniformMatrix4fv(MVPMatrix, 1, GL_TRUE, proj * mv); // 传模视投影矩阵
	glUniform3f(uColor, 0.3, 0.7, 0.3);
	glDrawArrays(GL_TRIANGLES, 0, NumVertices);

	glutSwapBuffers();					// 交换缓存

	if (singleStep) {					// 如果是单步执行，则关闭动画
		spinMode = GL_FALSE;
	}
}

void InitSphere()
{
	/*创建一个顶点数组对象(VAO)*/
	glGenVertexArrays(1, &vaoSphere);  // 生成一个未用的VAO ID，存于变量vao中
	glBindVertexArray(vaoSphere);      // 创建id为vao的VAO，并绑定为当前VAO

	/*创建并初始化一个缓冲区对象(Buffer Object)*/
	GLuint buffer;
	glGenBuffers(1, &buffer); // 生成一个未用的缓冲区对象ID，存于变量buffer中
	// 创建id为buffer的Array Buffer对象，并绑定为当前Array Buffer对象
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	// 为Buffer对象在GPU端申请空间，并提供数据
	glBufferData(GL_ARRAY_BUFFER,	// Buffer类型
		sizeof(point3) * NumVertices,  // 申请空间大小(注意不能是sizeof(sphere),否则是指针变量的大小——4字节)
		sphere,			 // 提供数据
		GL_STATIC_DRAW	// 表明将如何使用Buffer的标志(GL_STATIC_DRAW含义是一次提供数据，多遍绘制)
	);

	delete[] sphere; // 数据已传到 GPU，可删除顶点数据

	glEnableVertexAttribArray(vPosition);	// 启用顶点属性数组
	// 为顶点属性数组提供数据(数据存放在之前buffer对象中)
	glVertexAttribPointer(
		vPosition,			// 属性变量索引
		3,					// 每个顶点属性的分量个数
		GL_FLOAT,			// 数组数据类型
		GL_FALSE,			// 是否进行归一化处理
		0,  // 在数组中相邻属性成员起始位置间的间隔(以字节为单位)
		BUFFER_OFFSET(0)    // 第一个属性值在buffer中的偏移量
	);
}

void InitRing()
{
	/*创建一个顶点数组对象(VAO)*/
	glGenVertexArrays(1, &vaoRing);  // 生成一个未用的VAO ID，存于变量vao中
	glBindVertexArray(vaoRing);      // 创建id为vao的VAO，并绑定为当前VAO

	/*创建并初始化一个缓冲区对象(Buffer Object)*/
	GLuint bufferRing;
	glGenBuffers(1, &bufferRing); // 生成一个未用的缓冲区对象ID，存于变量buffer中
	// 创建id为buffer的Array Buffer对象，并绑定为当前Array Buffer对象
	glBindBuffer(GL_ARRAY_BUFFER, bufferRing);
	// 为Buffer对象在GPU端申请空间，并提供数据
	glBufferData(GL_ARRAY_BUFFER,	// Buffer类型
		sizeof(point3) * NumRing,  // 申请空间大小(注意不能是sizeof(sphere),否则是指针变量的大小——4字节)
		ring,			 // 提供数据
		GL_STATIC_DRAW	// 表明将如何使用Buffer的标志(GL_STATIC_DRAW含义是一次提供数据，多遍绘制)
	);

	delete[] ring; // 数据已传到 GPU，可删除顶点数据

	glEnableVertexAttribArray(vPosition);	// 启用顶点属性数组
	// 为顶点属性数组提供数据(数据存放在之前buffer对象中)
	glVertexAttribPointer(
		vPosition,			// 属性变量索引
		3,					// 每个顶点属性的分量个数
		GL_FLOAT,			// 数组数据类型
		GL_FALSE,			// 是否进行归一化处理
		0,  // 在数组中相邻属性成员起始位置间的间隔(以字节为单位)
		BUFFER_OFFSET(0)    // 第一个属性值在buffer中的偏移量
	);
}

// 初始化OpenGL的状态
void Init(void)
{
	// 生成中心在原点半径为1,15条经线和纬线的球的顶点
	BuildSphere(1.0, 15, 15);
	// 生成一个位于 x y 平面的轨道环
	BuildRing(1.0, 72);

	/*加载shader并使用所得到的shader程序*/
	// InitShader为InitShader.cpp中定义的函数，参数分别为顶点和片元shader的文件名
	// 返回值为shader程序对象的ID
	GLuint program = InitShader("vSolar.glsl", "fSolar.glsl");
	glUseProgram(program); // 使用该shader程序

	/*初始化顶点着色器中的顶点位置属性*/
	// 获取shader程序中属性变量的位置(索引)
	vPosition = glGetAttribLocation(program, "vPosition");

	InitSphere();
	InitRing();

	// 获取shader中uniform变量"MVPMatrix"的索引
	MVPMatrix = glGetUniformLocation(program, "MVPMatrix");

	// 获取shader中uniform变量"uColor"的索引
	uColor = glGetUniformLocation(program, "uColor");

	glClearColor(0.0, 0.0, 0.0, 0.0);		// 背景为黑色				
	glEnable(GL_DEPTH_TEST);				// 开启深度检测

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);	// 线框模式
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

// r键处理
void Key_r(void)
{
	if (singleStep) {			// 如果之前为单步执行模式
		singleStep = GL_FALSE;	// 结束单步执行
		spinMode = GL_TRUE;		// 重启动画
	}
	else {
		spinMode = !spinMode;	// 切换动画开关状态
	}
}

// s键处理(单步显示)
void Key_s(void)
{
	singleStep = GL_TRUE;
	spinMode = GL_TRUE;
}

//	普通按键回调函数
void KeyPressFunc(unsigned char Key, int x, int y)
{
	switch (Key) {
	case 'R':
	case 'r':
		Key_r();
		break;
	case 's':
	case 'S':
		Key_s();
		break;
	case 't':
	case 'T':
		if (ViewAngle > 15.0)
		{
			ViewAngle = 15.0;
		}
		else
		{
			ViewAngle = 90.0;
		}
		break;
	case 27:	// Esc键
		exit(EXIT_SUCCESS);
	}
}

// 方向键上处理
void Key_up(void)
{
	AnimateIncrement *= 2.0;			// 动画时间间隔翻倍，即加快动画速度
}

void Key_down(void)
{
	AnimateIncrement /= 2.0;			// 动画时间间隔减半，即减慢动画速度
}

//	特殊按键回调函数
void SpecialKeyFunc(int Key, int x, int y)
{
	switch (Key) {
	case GLUT_KEY_UP:
		Key_up();
		break;
	case GLUT_KEY_DOWN:
		Key_down();
		break;
	}
}

// 定时器，实现每秒绘制60帧
void myTimer(int value)
{
	glutPostRedisplay();						// 刷新显示
	glutTimerFunc(1000 / 60, myTimer, value);		// 循环计时
}

// Main函数：初始化OpenGL,设置回调函数,启动消息循环
int main(int argc, char** argv)
{
	glutInit(&argc, argv);	// 初始化GLUT库，必须在其他glut函数前执行

	/*以下初始化窗口属性和上下文(Context)的函数必须在glutCreateWindow之前调用*/
	// 初始化显示模式，使用RGB颜色模式、双缓存和多重采样
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

	// 以下2个函数来自freeglut库，用于确认 
	// 代码是基于OpenGL 3.1版本的
	glutInitContextVersion(3, 1); // 表明使用OpenGL 3.1
	// 保持向前兼容，即不使用任何弃用的函数
	// 如程序中使用了弃用函数则注释掉本行
	glutInitContextFlags(GLUT_FORWARD_COMPATIBLE);

	glutInitWindowSize(900, 540);

	glutCreateWindow("Solar System");	// 创建窗口，标题替换成自己的

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
	glutDisplayFunc(Animate);

	// 设置定时器
	glutTimerFunc(100, myTimer, 0);

	// 自定义的初始化函数
	// 通常将shader初始化和在程序运行过程中保持不变的属性的设置放在此函数中
	Init();

	glutMainLoop(); // 进入主循环(无限循环：不断检测事件处理事件)，此时窗口才会显示
}

