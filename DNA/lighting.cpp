#include "GL\glew.h"
#include "GL\glfw.h"
#include <cstdio>
#include <cmath>
#include <iostream>
#include <vector>
#include <algorithm>
#include "util.hpp"

using namespace std;

static const double MY_PI = 3.14159265358979323846264338327;

struct Vtx 
{
	GLfloat x, y, z;
	GLfloat nx, ny, nz;
	GLuint color;
};

enum { ATTRIB_POS, ATTRIB_COLOR, ATTRIB_NORMAL };

GLuint UNIFORM_viewProjMatrix, UNIFORM_modelMatrix, UNIFORM_normMatrix;

void bindVertexArray(Vtx *vtx) {
	glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), &(vtx->x));
	glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vtx), &(vtx->color));
	glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), &(vtx->nx));
}

bool loadShaderSource(GLuint shader, const char *filePath) {
	FILE *f = fopen(filePath, "r");
	if ( !f ) {
		cerr << "Cannot find file: " << filePath << '\n';
		return false;
	}
	fseek(f, 0, SEEK_END);
	const size_t sz = ftell(f) + 1;
	fseek(f, 0, SEEK_SET);
	
	GLchar *buffer = new GLchar[sz];
	fread(buffer, 1, sz, f);
	fclose(f);
	buffer[sz-1] = 0;
	glShaderSource(shader, 1, (const GLchar**) &buffer, NULL);
	
	glCompileShader(shader);
	delete [] buffer;
	
	GLint logLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	if ( logLength > 0 ) {
		GLchar *log = new GLchar[logLength];
		glGetShaderInfoLog(shader, logLength, &logLength, log);
		cout << "Shader Compile Log:\n" << log << endl;
		delete [] log;
	}
	
	return true;
}

#ifndef ENABLE

int main() {
	if ( !glfwInit() ) {
		cerr << "Unable to initialize OpenGL!\n";
		return -1;
	}
	if ( !glfwOpenWindow(640,640,
						 8,8,8,8,
						 24,0,
						 GLFW_WINDOW) ) {
		cerr << "Unable to create OpenGL window.\n";
		glfwTerminate();
		return -1;
	}
	
	if ( glewInit() != GLEW_OK ) {
		cerr << "Unable to hook OpenGL extensions!\n";
		return -1;
	}
	glfwSetWindowTitle("3D Lighting");
	
	glfwEnable(GLFW_STICKY_KEYS);
	glfwSwapInterval(1);
	
	GLuint vtxShader = glCreateShader(GL_VERTEX_SHADER),
	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	if ( !loadShaderSource(vtxShader, "diffuse_pf.vsh") )  {
		cerr << "Unable to load vertex shader.";
		return -1;
	}
	if ( !loadShaderSource(fragShader, "diffuse_pf.fsh") ) {
		cerr << "Unable to load fragment shader.";
		return -1;
	}
	
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	
	GLuint program = glCreateProgram();
	glAttachShader(program, vtxShader);
	glAttachShader(program, fragShader);
	
	glBindAttribLocation(program, ATTRIB_POS, "pos");
	glBindAttribLocation(program, ATTRIB_COLOR, "color");
	glBindAttribLocation(program, ATTRIB_NORMAL, "normal");

	
	glLinkProgram(program);
	
	
	{
		GLint logLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
		if ( logLength > 0 ) {
			GLchar *log = new GLchar[logLength];
			glGetProgramInfoLog(program, logLength, &logLength, log);
			cout << "Program Compile Log:\n" << log << endl;
			delete [] log;
		}
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
	
	double t = 0;
	
	GLMatrix4 projection, view;
	GLfloat dist = 5;
	projection.setPerspective(MY_PI/3, 1, .1, 300);
	lookAt(dist,dist,dist,0,0,0,0,0,1,view.mat);
	
	
	
	GLMatrix4 identity;
	identity.setIdentity();
	
	glUniform3f(UNIFORM_lightIntensity0, 1, 1, 1);
	glUniform3f(UNIFORM_lightAmbient, 0.15, 0.15, 0.15);
	
	GLfloat lx = 3, ly = 3, lz = 3, lw = 1;
	GLMatrix4 lightAnimation;
	lightAnimation.setRotation(0, 0, 1, 0.01);

	

	SceneNode root;
	PyramidNode pyramid(1, 1.5, 4, 0xFFAAFFAA, 0xFFAAAAAA), pyramid2(2, 1.5, 20, 0xFFAAAAAA, 0xFFAAAAAA);
	pyramid.transform.translate(2, -1, 1);
	pyramid2.transform.translate(-2, 1, 2);
	CubeNode cube(1, 0xFFFF0000, 0xFF00FFFF, 0xFF00FF00, 0xFFFF00FF, 0xFF0000FF, 0xFFFFFF00),
	cube2(1, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000, 0xFFFF0000);
	cube2.transform.translate(0,3,1);
	cube.transform.translate(0,0,1);
	PlaneNode plane(20, 20, 0xFFFFFFFF);
	CubeNode lightPos(.2, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
	PrismNodeStrip prism(2, 1, 10, 0xFF00FF00);
	prism.transform.translate(2, 3, 2);
	
	{
		root.children.push_back(&cube);
		root.children.push_back(&cube2);
		root.children.push_back(&pyramid);
		root.children.push_back(&pyramid2);
		root.children.push_back(&plane);
		root.children.push_back(&prism);
	}
	do {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if ( glfwGetKey(GLFW_KEY_UP) == GLFW_PRESS ) {
			GLMatrix4 r;
			r.setRotation(1,0,0, 0.1);
			view = view * r;
		} else if ( glfwGetKey(GLFW_KEY_DOWN) == GLFW_PRESS ) {
			GLMatrix4 r;
			r.setRotation(1,0,0, -0.1);
			view = view * r;
		} else if ( glfwGetKey(GLFW_KEY_LEFT) == GLFW_PRESS ) {
			GLMatrix4 r;
			r.setRotation(0,0,1, 0.1);
			view = view * r;
		} else if ( glfwGetKey(GLFW_KEY_RIGHT) == GLFW_PRESS ) {
			GLMatrix4 r;
			r.setRotation(0,0,1, -0.1);
			view = view * r;
		}
		
		GLMatrix4 vp = projection * view;
		
		glUniformMatrix4fv(UNIFORM_viewProjMatrix, 1, false, vp.mat);
		
		lightAnimation.mult(lx, ly, lz, lw);
		glUniform3f(UNIFORM_lightPos0, lx, ly, lz);
		
		glUniform3f(UNIFORM_lightAmbient, 0.2, 0.2, 0.2);
		root.draw(identity, identity);
		glUniform3f(UNIFORM_lightAmbient, 1, 1, 1);
		GLMatrix4 lightPosTranslation;
		lightPosTranslation.setIdentity();
		lightPosTranslation.translate(lx, ly, lz);
		lightPos.draw(lightPosTranslation, identity);
		
		glfwSwapBuffers();
		t += 0.02f;
	} while ( glfwGetKey(GLFW_KEY_ESC) != GLFW_PRESS && glfwGetWindowParam(GLFW_OPENED) );
	glfwTerminate();
	
	return 0;
}
#endif