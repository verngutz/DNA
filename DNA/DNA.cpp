#include <gl\glfw.h>
#include <iostream>
#include <cmath>

int main() {
	if ( !glfwInit() ) {
		std::cerr << "Unable to initialize OpenGL!\n";
		return -1;
	}

	if ( !glfwOpenWindow(640,480, //width and height of the screen
				8,8,8,0, //Red, Green, Blue and Alpha bits
				0,0, //Depth and Stencil bits
				GLFW_WINDOW)) {
		std::cerr << "Unable to create OpenGL window.\n";
		glfwTerminate();
		return -1;
	}

	glfwSetWindowTitle("DNA");

	// Ensure we can capture the escape key being pressed below
	glfwEnable( GLFW_STICKY_KEYS );

	// Enable vertical sync (on cards that support it)
	glfwSwapInterval( 1 );

	glClearColor(0,0,0,0);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	do {
		int width, height;
		// Get window size (may be different than the requested size)
		//we do this every frame to accommodate window resizing.
		glfwGetWindowSize( &width, &height );
		glViewport( 0, 0, width, height );

		glClear(GL_COLOR_BUFFER_BIT);

		//TO DO: drawing code here

		//VERY IMPORTANT: displays the buffer to the screen
		glfwSwapBuffers();
	} while ( glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS &&
			glfwGetWindowParam(GLFW_OPENED) );

	glfwTerminate();
	return 0;
}
