#define ACC 0
#define VEL 1
#define POS 2

class SceneNode 
{
protected:
	bool alive;
	GLfloat newX;
	GLfloat newY;
	GLfloat newZ;

public:
	GLfloat x[3];
	GLfloat y[3];
	GLfloat z[3];

	GLMatrix4 transform, normTransform;
	list<SceneNode*> children;
	
	static bool is_dead (SceneNode * node) { return !node->alive; }

	SceneNode(GLfloat x = 0, GLfloat y = 0, GLfloat z = 0, 
		GLfloat velx = 0, GLfloat vely = 0, GLfloat velz = 0,
		GLfloat accx = 0, GLfloat accy = 0, GLfloat accz = 0) 
	{
		normTransform.setIdentity();
		alive = true;
	}

	virtual void draw(const GLMatrix4 &parentTransform, const GLMatrix4 &parentNormTransform) 
	{
		drawChildren(parentTransform * transform, parentNormTransform * normTransform);
	}
	
	virtual void update(unsigned long long t) 
	{
		GLfloat newX = x[ACC] * t * t + x[VEL] * t + x[POS];
		GLfloat newY = y[ACC] * t * t + y[VEL] * t + y[POS];
		GLfloat newZ = z[ACC] * t * t + z[VEL] * t + z[POS];

		transform.setTranslation(newX, newY, newZ);

		for(list<SceneNode*>::iterator i = children.begin(); i != children.end(); i++)
			(*i)->update(t);

		children.remove_if(is_dead);
	}
	
	void drawChildren(const GLMatrix4 &t, const GLMatrix4 &nt) 
	{
		for(list<SceneNode*>::iterator i = children.begin(); i != children.end(); i++)
			(*i)->draw(t, nt);
	}
	
	virtual ~SceneNode() { }

	static void setTransform(const GLMatrix4 &t, const GLMatrix4 &nt) 
	{
		glUniformMatrix4fv(UNIFORM_modelMatrix, 1, false, t.mat);
		glUniformMatrix4fv(UNIFORM_normMatrix, 1, false, nt.mat);
	}
};

class PlaneNode : public SceneNode 
{
	Vtx vertices[4];
public:
	PlaneNode(GLsizei xSize, GLsizei ySize, GLuint color) 
	{
		vertices[0].x = -xSize/2;
		vertices[0].y = ySize/2;
		vertices[0].z = 0;
		vertices[0].color = color;
		vertices[0].nx = 0;
		vertices[0].ny = 0;
		vertices[0].nz = 1;
		
		vertices[1].x = -xSize/2;
		vertices[1].y = -ySize/2;
		vertices[1].z = 0;
		vertices[1].color = color;
		vertices[1].nx = 0;
		vertices[1].ny = 0;
		vertices[1].nz = 1;
		
		vertices[2].x = xSize/2;
		vertices[2].y = ySize/2;
		vertices[2].z = 0;
		vertices[2].color = color;
		vertices[2].nx = 0;
		vertices[2].ny = 0;
		vertices[2].nz = 1;
		
		vertices[3].x = xSize/2;
		vertices[3].y = -ySize/2;
		vertices[3].z = 0;
		vertices[3].color = color;
		vertices[3].nx = 0;
		vertices[3].ny = 0;
		vertices[3].nz = 1;
	}
	
	virtual void draw(const GLMatrix4 &parentTransform, const GLMatrix4 &parentNormalTransform) 
	{
		const GLMatrix4 &t = parentTransform * transform,
		&nt = parentNormalTransform * normTransform;
		bindVertexArray(vertices);
		setTransform(t, nt);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		drawChildren(t, nt);
	}
};

class CubeNode : public SceneNode 
{
	Vtx vtx[6*6];
public:
	CubeNode(GLfloat size) 
	{
		GLuint colors[6];
		for(int i = 0; i < 6; i++)
		{
			colors[i] = (std::rand() % 0x100000000) | 0xFF000000;
		}
		static const GLfloat permutation[][4][3] = 
		{
			{
				{-1,1,1},{-1,-1,1},{1,-1,1},{1,1,1}
			},
			{
				{1,1,-1},{1,-1,-1},{-1,-1,-1},{-1,1,-1}
			},
			{
				{1,1,1},{1,-1,1},{1,-1,-1},{1,1,-1}
			},
			{
				{-1,1,-1},{-1,-1,-1},{-1,-1,1},{-1,1,1}
			},
			{
				{-1,1,-1},{-1,1,1},{1,1,1},{1,1,-1}
			},
			{
				{1,-1,-1},{1,-1,1},{-1,-1,1},{-1,-1,-1}
			}
		};
		static const GLfloat norms[][3] = 
		{
			{0, 0, 1},
			{0, 0, -1},
			{1, 0, 0},
			{-1, 0, 0},
			{0, 1, 0},
			{0, -1, 0}
		};

		int index = 0;
		size /= 2;
		for ( size_t i = 0; i < 6; ++i ) 
		{
			vtx[index].x = size * permutation[i][0][0];
			vtx[index].y = size * permutation[i][0][1];
			vtx[index].z = size * permutation[i][0][2];
			vtx[index].color = colors[i];
			vtx[index].nx = norms[i][0];
			vtx[index].ny = norms[i][1];
			vtx[index].nz = norms[i][2];
			++index;
			vtx[index].x = size * permutation[i][1][0];
			vtx[index].y = size * permutation[i][1][1];
			vtx[index].z = size * permutation[i][1][2];
			vtx[index].color = colors[i];
			vtx[index].nx = norms[i][0];
			vtx[index].ny = norms[i][1];
			vtx[index].nz = norms[i][2];
			++index;
			vtx[index].x = size * permutation[i][3][0];
			vtx[index].y = size * permutation[i][3][1];
			vtx[index].z = size * permutation[i][3][2];
			vtx[index].color = colors[i];
			vtx[index].nx = norms[i][0];
			vtx[index].ny = norms[i][1];
			vtx[index].nz = norms[i][2];
			++index;
			vtx[index].x = size * permutation[i][3][0];
			vtx[index].y = size * permutation[i][3][1];
			vtx[index].z = size * permutation[i][3][2];
			vtx[index].color = colors[i];
			vtx[index].nx = norms[i][0];
			vtx[index].ny = norms[i][1];
			vtx[index].nz = norms[i][2];
			++index;
			vtx[index].x = size * permutation[i][1][0];
			vtx[index].y = size * permutation[i][1][1];
			vtx[index].z = size * permutation[i][1][2];
			vtx[index].color = colors[i];
			vtx[index].nx = norms[i][0];
			vtx[index].ny = norms[i][1];
			vtx[index].nz = norms[i][2];
			++index;
			vtx[index].x = size * permutation[i][2][0];
			vtx[index].y = size * permutation[i][2][1];
			vtx[index].z = size * permutation[i][2][2];
			vtx[index].color = colors[i];
			vtx[index].nx = norms[i][0];
			vtx[index].ny = norms[i][1];
			vtx[index].nz = norms[i][2];
			++index;
		}
	}
	
	virtual void update(unsigned long long t)
	{
		SceneNode::update(t);
		alive = newZ < 0;
	}

	virtual void draw(const GLMatrix4 &parentTransform, const GLMatrix4 &parentNormTransform) 
	{
		const GLMatrix4 &t = parentTransform * transform,
		&nt = parentNormTransform * normTransform;
		bindVertexArray(vtx);
		setTransform(t, nt);
		
		glDrawArrays(GL_TRIANGLES, 0, sizeof(vtx)/sizeof(Vtx));
		
		drawChildren(t,nt);
	}
};

class Cylinder : public SceneNode
{
	Vtx vTopCircle[16];
	Vtx vCylinder[32];
public:
	Cylinder(GLfloat rad, GLfloat height)
	{
		/*for(i=0; i<8; ++i)
		{
			vBotCircle[i].x = rad*cos(MY_PI*i/4);
			vBotCircle[i].y = 0;
			vBotCircle[i].z = rad*sin(MY_PI*i/4);
		}*/
	
		for(int i=0; i<32; ++i)
		{
			if(2%i==0)
			{
				vCylinder[i].x = rad*cos(PI*i/16);
				vCylinder[i].y = 0;	
				vCylinder[i].z = rad*sin(PI*i/16);
			}
		
			else
			{
				vCylinder[i].x = rad*cos(PI*i/16);
				vCylinder[i].y = height;	
				vCylinder[i].z = rad*sin(PI*i/16);
			}
		}
	
		for(int i=0; i<16; ++i)
		{
			if(2%i==0)
			{
				vTopCircle[i].x = rad*cos(PI*i/8);
				vTopCircle[i].y = height;	
				vTopCircle[i].z = rad*sin(PI*i/8);
			}
		
			else
			{
				vTopCircle[i].x = 0;
				vTopCircle[i].y = height;	
				vTopCircle[i].z = 0;
			}
		}
	}

	virtual void draw(const GLMatrix4 &parentTransform, const GLMatrix4 &parentNormTransform)
	{
		const GLMatrix4 &t = parentTransform * transform,
		&nt = parentNormTransform * normTransform;
		bindVertexArray(vCylinder);
		setTransform(t, nt);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		drawChildren(t,nt);
		
		//bindVertexArray(vTopCircle);
		
	}
};