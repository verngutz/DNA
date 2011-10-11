#include "util.hpp"
#include "geometries.hpp"

using namespace std;

// TO DO: Add variables here
GLuint	UNIFORM_viewProjMatrix, 
		UNIFORM_modelMatrix, 
		UNIFORM_normMatrix,
		UNIFORM_lightPos0,
		UNIFORM_lightIntensity0,
		UNIFORM_lightAmbient;

GLMatrix4 identityMat;
GLMatrix4 projection, view;

SceneNode root;

void initialize()
{
	// TO DO: Add Initialization code here
	if (!glfwInit()) 
	{
		cerr << "Unable to initialize OpenGL!\n";
		exit(1);
	}

	if (!glfwOpenWindow(640,480,	//width and height of the screen
						8,8,8,0,	//Red, Green, Blue and Alpha bits
						0,0,		//Depth and Stencil bits
						GLFW_WINDOW)) 
	{
		cerr << "Unable to create OpenGL window.\n";
		glfwTerminate();
		exit(1);
	}

	// Enable vertical sync (on cards that support it)
	glfwSwapInterval(1);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	
	glClearColor(0,0,0,0);
	glfwSetWindowTitle("DNA");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	identityMat.setIdentity();
	projection.setPerspective(PI / 3, 1, 10, 1000);
	lookAt(20, 20, 20, 0, 0, 0, 0, 0, 1, view.mat);

	glUniform3f(UNIFORM_lightIntensity0, 1, 1, 1);
	glUniform3f(UNIFORM_lightAmbient, 1, 1, 1);

	PlaneNode plane(20, 20, 0xFFFFFF);
	root.children.push_back(&plane);
}

void loadContent()
{
	// TO DO: Load necessary graphics resources here
	GLuint vtxShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	if ( !loadShaderSource(vtxShader, "vertexShader.vsh") )  
	{
		cerr << "Unable to load vertex shader.";
		exit(1);
	}
	if ( !loadShaderSource(fragShader, "fragmentShader.fsh") ) 
	{
		cerr << "Unable to load fragment shader.";
		exit(1);
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vtxShader);
	glAttachShader(program, fragShader);
	
	glBindAttribLocation(program, ATTRIB_POS, "pos");
	glBindAttribLocation(program, ATTRIB_COLOR, "color");
	glBindAttribLocation(program, ATTRIB_NORMAL, "normal");
	
	glLinkProgram(program);
	
	GLint logLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	if ( logLength > 0 )
	{
		GLchar *log = new GLchar[logLength];
		glGetProgramInfoLog(program, logLength, &logLength, log);
		cout << "Program Compile Log:\n" << log << endl;
		delete [] log;
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
	// TO DO: Add Update (logic) code here
}

void draw(unsigned long long time)
{
	// TO DO: Add Drawing code here
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GLMatrix4 vp = projection * view;
	glUniformMatrix4fv(UNIFORM_viewProjMatrix, 1, false, vp.mat);
	root.draw(identityMat, identityMat);
}

#ifdef ENABLE
int main()
{
	// Initialize.
	initialize();

	// Load graphics resources
	loadContent();

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