#include "Angel.h"

namespace Angel
{
	static char* readShaderSource(const char* shaderFile)
	{
		FILE* fp;
		char* buff = NULL;
		if (!fopen_s(&fp, shaderFile, "r"))
		{
			fseek(fp, 0L, SEEK_END); // 将文件指针移动到文件尾
			long size = ftell(fp); // 获取文件长度

			fseek(fp, 0L, SEEK_SET); // 将指针移到文件头
			buff = new char[size + 1]; // 增加一位给结尾符 '\0'
			memset(buff, 0, size + 1); // 初始化
			fread(buff, 1, size, fp); // 一次性读出整个文件

			buff[size] = '\0';
			fclose(fp);
		}

		return buff;
	}

	// 根据顶点和片元Shader文件 创建 GLSL 程序对象
	GLuint InitShader(const char* vShaderFile, const char* fShaderFile)
	{
		GLuint program = glCreateProgram();

		Shader shaders[2] =
		{
			{vShaderFile, GL_VERTEX_SHADER, NULL},
			{fShaderFile, GL_FRAGMENT_SHADER, NULL},
		};

		for (int i = 0; i < 2; i++)
		{
			Shader& s = shaders[i];
			s.source = readShaderSource(s.filename);
			if (NULL == s.source)
			{
				std::cerr << "Failed to read " << s.filename << std::endl;
				exit(EXIT_FAILURE);
			}

			GLuint shader = glCreateShader(s.type);

			std::cout << (const GLint*)strlen(s.source) << std::endl;
			GLint stringLen = strlen(s.source);
			//std::cout << s.source << std::endl;

			// 参数1：shader对象ID,
			// 参数2：参数3中含有的字符串个数, 
			// 参数3：含有源码的字符串数组, 
			// 参数4：字符串长度数组(成员为参数3中各字符串长度)，为NULL表示各字符串均以NULL结束)
			glShaderSource(shader, 1, (const GLchar**)&s.source, NULL);
			glCompileShader(shader); // 编译shader程序

			GLint compiled;
			// 获取编译状态信息
			glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

			if (!compiled)
			{
				// 如果编译失败
				std::cerr << s.filename << " failed to compile:" << std::endl; // 输出错误信息
				GLint logSize;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize); // 获取错误信息的长度
				char* logMsg = new char[logSize];  // 根据信息长度创建buffer

				// 获取错误信息
				// 参数1：shader对象ID
				// 参数2：用于存储信息的buffer(这里即logMsg)的大小
				// 参数3：输出参数，存储获取到的信息长度的变量指针
				// 参数4：用于保存信息的字符buffer
				glGetShaderInfoLog(shader, logSize, NULL, logMsg);

				std::cerr << logMsg << std::endl;	// 输出错误信息
				delete[] logMsg;
				system("pause");
				exit(EXIT_FAILURE);	// 退出程序
			}

			delete[] s.source;
			glAttachShader(program, shader);
		}

		/* 链接并检查错误信息 */
		glLinkProgram(program);	// 链接shader程序

		GLint linked;
		glGetProgramiv(program, GL_LINK_STATUS, &linked); // 获取链接信息，参数含义和glGetShaderiv类似
		if (!linked)
		{
			// 链接失败？
			std::cerr << "Shader program failed to link" << std::endl;
			GLint logSize;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);	// 获取链接信息的长度
			char* logMsg = new char[logSize];
			glGetProgramInfoLog(program, logSize, NULL, logMsg);	// 获取链接信息
			std::cerr << logMsg << std::endl;
			delete[] logMsg;
			system("pause");
			exit(EXIT_FAILURE);
		}

		return program;
	}

}