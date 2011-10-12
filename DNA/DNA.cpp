#include "util.hpp"
#include <time.h>
#include <list>

using namespace std;

// TO DO: Add variables here
GLuint	UNIFORM_viewProjMatrix, 
		UNIFORM_modelMatrix, 
		UNIFORM_normMatrix,
		UNIFORM_lightPos0,
		UNIFORM_lightIntensity0,
		UNIFORM_lightAmbient;

#include "geometries.hpp"

GLMatrix4 identityMat;
GLMatrix4 projection, view;

GLint oldLeftMouseState;

SceneNode root;

const GLfloat GRAVITY = -5;

void initialize()
{
	// TO DO: Add Initialization code here	
	glfwSetWindowTitle("DNA");
	glClearColor(0,0,0,0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	identityMat.setIdentity();
	projection.setPerspective(PI / 3, 1, 0.1, 300);
	lookAt(20, 20, 20, 0, 0, 0, 0, 0, 1, view.mat);

	glUniform3f(UNIFORM_lightIntensity0, 1, 1, 1);
	glUniform3f(UNIFORM_lightAmbient, 0.15, 0.15, 0.15);

	oldLeftMouseState = glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT);

	PlaneNode planeNode(20, 20, 0xFFFFFFFF);
	root.children.push_back(&planeNode);
}

void loadContent()
{
	// TO DO: Load necessary graphics resources here
	GLuint vtxShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char *vertexShader[] = 
	{
		"#version 120\n",

		"uniform mat4 viewProjMatrix;\n",
		"uniform mat4 modelMatrix;\n",
		"uniform mat4 normMatrix;\n",

		"attribute vec3 pos;\n",
		"attribute vec3 normal;\n",
		"attribute vec4 color;\n",

		"varying vec4 out_color;\n",
		"varying vec3 out_worldPos;\n",
		"varying vec3 out_norm;\n",

		"void main()\n",
		"{\n",
		"	vec4 p = modelMatrix * vec4(pos, 1);\n",
		"	out_worldPos = p.xyz;\n",
		"	out_norm = (normMatrix * vec4(normal, 0)).xyz;\n",
		"	out_color = color;\n",

		"	gl_Position = viewProjMatrix * p;\n",
		"}\n"
	};

	const char *fragmentShader[] = 
	{
		"#version 120\n",

		"varying vec4 out_color;\n",
		"varying vec3 out_worldPos;\n",
		"varying vec3 out_norm;\n",

		"uniform vec3 lightPos0;\n",
		"uniform vec3 lightIntensity0;\n",
		"uniform vec3 lightAmbient;\n",

		"void main()\n",
		"{\n",
		"	gl_FragColor.rgb = out_color.rgb * (lightIntensity0 * max(0,dot(normalize(lightPos0 - out_worldPos), normalize(out_norm))) + lightAmbient);\n",
		"	gl_FragColor.a = out_color.a;\n",
		"}\n"
	};

	glShaderSource(vtxShader, 18, vertexShader, NULL);
	glShaderSource(fragShader, 12, fragmentShader, NULL);

	glCompileShader(vtxShader);
	glCompileShader(fragShader);

	GLint logLength;
	glGetShaderiv(vtxShader, GL_INFO_LOG_LENGTH, &logLength);
	GLchar *log = new GLchar[logLength];
	glGetShaderInfoLog(vtxShader, logLength, &logLength, log);
	std::cout << "Vertex Shader Compile Log:\n" << log << std::endl;
	delete [] log;

	GLint logLength2;
	glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &logLength2);
	GLchar *log2 = new GLchar[logLength2];
	glGetShaderInfoLog(fragShader, logLength, &logLength, log2);
	std::cout << "Fragment Shader Compile Log:\n" << log2 << std::endl;
	delete [] log2;

	GLuint program = glCreateProgram();

	glAttachShader(program, vtxShader);
	glAttachShader(program, fragShader);
	
	glBindAttribLocation(program, ATTRIB_POS, "pos");
	glBindAttribLocation(program, ATTRIB_COLOR, "color");
	glBindAttribLocation(program, ATTRIB_NORMAL, "normal");
	
	glLinkProgram(program);
	
	GLint logLength3;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength3);
	if ( logLength3 > 0 )
	{
		GLchar *log3 = new GLchar[logLength3];
		glGetProgramInfoLog(program, logLength, &logLength3, log3);
		cout << "Program Compile Log:\n" << log << endl;
		delete [] log3;
	}

	glDeleteShader(fragShader);
	glDeleteShader(vtxShader);
	fragShader = 0;
	vtxShader = 0;
	
	UNIFORM_viewProjMatrix = glGetUniformLocation(program, "viewProjMatrix");
	UNIFORM_modelMatrix = glGetUniformLocation(program, "modelMatrix");
	UNIFORM_normMatrix = glGetUniformLocation(program, "normMatrix");
	
	UNIFORM_lightPos0 = glGetUniformLocation(program, "lightPos0");
	UNIFORM_lightIntensity0 = glGetUniformLocation(program, "lightIntensity0");
	UNIFORM_lightAmbient = glGetUniformLocation(program, "lightAmbient");
	
	glUseProgram(program);
	
	glEnableVertexAttribArray(ATTRIB_POS);
	glEnableVertexAttribArray(ATTRIB_COLOR);
	glEnableVertexAttribArray(ATTRIB_NORMAL);
}

void unloadContent()
{
	// TO DO: Release used graphics resources here
}

void update(unsigned long long time)
{
	glUniform3f(UNIFORM_lightPos0, 0, 10 * sin((long double)time / 1000), 10 * cos((long double)time / 1000));
	
	GLint newLeftMouseState = glfwGetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
	if(oldLeftMouseState == GLFW_PRESS && newLeftMouseState == GLFW_RELEASE)
	{
		GLint x;
		GLint y;

		glfwGetMousePos(&x, &y);

		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		GLfloat mouseRayNear[3];
		GLfloat mouseRayFar[3];

		unproject(x, y, view.mat, projection.mat, viewport, mouseRayNear, mouseRayFar);

		CubeNode cube(5);
		cube.x[POS] = mouseRayNear[0];
		cube.y[POS] = mouseRayNear[1];
		cube.z[POS] = mouseRayNear[2];

		cube.x[VEL] = mouseRayFar[0];
		cube.y[VEL] = mouseRayFar[1];
		cube.z[VEL] = mouseRayFar[2];

		cube.x[ACC] = 0;
		cube.y[ACC] = 0;
		cube.z[ACC] = GRAVITY;
	}
	root.update(time);
}

void draw(unsigned long long time)
{
	// TO DO: Add Drawing code here
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLMatrix4 vp = projection * view;
	glUniformMatrix4fv(UNIFORM_viewProjMatrix, 1, false, vp.mat);
	root.draw(identityMat, identityMat);
}

void setup()
{
	srand(time(NULL));

	if (!glfwInit()) 
	{
		cerr << "Unable to initialize OpenGL!\n";
		exit(1);
	}

	if (!glfwOpenWindow(640,480,	//width and height of the screen
						8,8,8,0,	//Red, Green, Blue and Alpha bits
						24,0,		//Depth and Stencil bits
						GLFW_WINDOW)) 
	{
		cerr << "Unable to create OpenGL window.\n";
		glfwTerminate();
		exit(1);
	}

	if ( glewInit() != GLEW_OK ) 
	{
		cerr << "Unable to hook OpenGL extensions!\n";
		exit(1);
	}

	// Enable vertical sync (on cards that support it)
	glfwSwapInterval(1);
	glfwEnable(GLFW_STICKY_KEYS);
}

#ifdef ENABLE
int main()
{
	// Set up OpenGL
	setup();

	// Load graphics resources
	loadContent();
	
	// Initialize.
	initialize();

	unsigned long long time = 0;

	do 
	{
		// Respond to changes in window size.
		int width, height;
		glfwGetWindowSize( &width, &height );
		glViewport( 0, 0, width, height );

		// Update.
		update(time);

		// Draw.
		draw(time);

		// Show buffers to screen.
		glfwSwapBuffers();
		time++;
	}
	while (glfwGetWindowParam(GLFW_OPENED));

	// Exit.
	glfwTerminate();

	// Unload graphics resources
	unloadContent();
	return 0;
}
#endif