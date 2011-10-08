#include <gl\glfw.h>
#include <iostream>
#include <cmath>

// TO DO: Add variables here

void initialize()
{
	// TO DO: Add Initialization code here
	glfwSetWindowTitle("DNA");
}

void update()
{
	// TO DO: Add Update (logic) code here
}

void draw()
{
	// TO DO: Add Drawing code here
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
}

void prepareGL()
{
	if (!glfwInit()) 
	{
		std::cerr << "Unable to initialize OpenGL!\n";
		exit(1);
	}

	if (!glfwOpenWindow(640,480,	//width and height of the screen
						8,8,8,0,	//Red, Green, Blue and Alpha bits
						0,0,		//Depth and Stencil bits
						GLFW_WINDOW)) 
	{
		std::cerr << "Unable to create OpenGL window.\n";
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
}

int main()
{
	// Prepare openGL.
	prepareGL();

	// Initialize.
	initialize();

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
	return 0;
}