# ShaderBuilder使用说明

本人在学习opengl的shader相关内容的时候，感觉shader使用很不方便，因此写了这个ShaderBuilder来方便简单shader的使用。（当然也预留出了接口方便使用自定义shader。）

## 默认的shader模板

如下：（来自[https://blog.csdn.net/Jaihk662/article/details/105581060](https://blog.csdn.net/Jaihk662/article/details/105581060)）

输入：

```shader
#version 330 core
layout (location = 0) in vec3 position;
void main()
{
    gl_Position = vec4(position.x, position.y, position.z, 1.0);
}
```

输出：

```shader
#version 330 core
out vec4 color;
void main()
{
    color = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
```

## 开始使用

下载ShaderBuider.h和ShaderBuilder.cpp，然后直接加入到你的项目里（确保opengl环境已经配好），就可以开始使用了。


## 基本使用样例

```c++
	glfwMakeContextCurrent(window);  // 必须在ShaderBuilder初始化前设置gl上下文，否则会报错
	...
	GLHelper::ShaderBuilder sbuilder; // 初始化ShaderBuilder对象
    GLuint shaderProgram;
	...
	while(true)
	{
		...
		shaderProgram = sbuilder.GetShaderProgram(1.0f, 0.0f, 0.0f); // 获取ShaderProgram
        glUseProgram(shaderProgram);
		glDrawXXX(); // 绘制图形
		...
	}
```

## 高级用法

```c++
const GLchar* myInputShader = "..."; // 你自己的shader
const GLchar* myOutputShader = "...";

glfwMakeContextCurrent(window);
GLHelper::ShaderBuilder sbuilder; // 初始化ShaderBuilder对象
GLuint shaderProgram;

sbuilder.SetInputShader(myInputShader);
int markId = 1; // 这个markId由你自己指定，用作对某个shader的标记，方便查找
shaderProgram = sbuilder.GetUniqueShaderProgram(myOutputShader);

while(true)
{
	...
	shaderProgram = sbuilder.GetUniqueShaderProgram(markId); // 获取ShaderProgram
    glUseProgram(shaderProgram);
	glDrawXXX(); // 绘制图形
	...
}
```

# SimpleShaderBuilder
