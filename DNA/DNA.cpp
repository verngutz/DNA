#include "util.hpp"

using namespace std;

// TO DO: Add variables here

void initialize()
{
	// TO DO: Add Initialization code here
	glfwSetWindowTitle("DNA");
}

void loadContent()
{
	// TO DO: Load necessary graphics resources here
	GLuint	vtxShader = glCreateShader(GL_VERTEX_SHADER),
			fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	if ( !loadShaderSource(vtxShader, "diffuse_pf.vsh") )  
	{
		cerr << "Unable to load vertex shader.";
		exit(1);
	}
	if ( !loadShaderSource(fragShader, "diffuse_pf.fsh") ) 
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
	
	GLuint UNIFORM_lightPos0 = glGetUniformLocation(program, "lightPos0"),
	UNIFORM_lightIntensity0 = glGetUniformLocation(program, "lightIntensity0"),
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

void update()
{
	// TO DO: Add Update (logic) code here
}

void draw()
{
	// TO DO: Add Drawing code here
	glClear(GL_COLOR_BUFFER_BIT);
}

void prepareGL()
{
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

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glClearColor(0,0,0,0);
}

#ifdef ENABLE
int main()
{
	// Prepare openGL.
	prepareGL();

	// Initialize.
	initialize();

	// Load graphics resources
	loadContent();

	do 
	{
		// Respond to changes in window size.
		int width, height;
		glfwGetWindowSize( &width, &height );
		glViewport( 0, 0, width, height );

		// Update.
		update();

		// Draw.
		draw();

		// Show buffers to screen.
		glfwSwapBuffers();
	}
	while (glfwGetWindowParam(GLFW_OPENED));

	// Exit.
	glfwTerminate();

	// Unload graphics resources
	unloadContent();
	return 0;
}
#endif