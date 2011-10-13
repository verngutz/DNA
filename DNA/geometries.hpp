#define ACC 0
#define VEL 1
#define POS 2

#include <list>
#include <vector>

class SceneNode 
{
protected:
	GLfloat newX;
	GLfloat newY;
	GLfloat newZ;
	bool alive;

public:
	GLfloat x[3];
	GLfloat y[3];
	GLfloat z[3];
	
	unsigned long long birthTime;

	GLMatrix4 transform, normTransform;
	list<SceneNode*> children;
	list<SceneNode*>::iterator i;
	
	static bool is_dead (SceneNode * node) { return !node->alive; }

	SceneNode(GLfloat x = 0, GLfloat y = 0, GLfloat z = 0, 
		GLfloat velx = 0, GLfloat vely = 0, GLfloat velz = 0,
		GLfloat accx = 0, GLfloat accy = 0, GLfloat accz = 0) 
	{
		newX = 0;
		newY = 0;
		newZ = 0;

		this->x[POS] = x;
		this->x[VEL] = velx;
		this->x[ACC] = accx;

		this->y[POS] = y;
		this->y[VEL] = vely;
		this->y[ACC] = accy;

		this->z[POS] = z;
		this->z[VEL] = velz;
		this->z[ACC] = accz;

		transform.setIdentity();
		normTransform.setIdentity();
		alive = true;
	}

	virtual void draw(const GLMatrix4 &parentTransform, const GLMatrix4 &parentNormTransform) 
	{
		drawChildren(parentTransform * transform, parentNormTransform * normTransform);
	}
	
	virtual void update(unsigned long long time)
	{
		transform.translate(-newX, -newY, -newZ);

		x[VEL] += x[ACC];
		y[VEL] += y[ACC];
		z[VEL] += z[ACC];

		x[POS] += x[VEL];
		y[POS] += y[VEL];
		z[POS] += z[VEL];
		if(z[POS] < 0) z[POS] = 0;

		newX = x[POS];
		newY = y[POS];
		newZ = z[POS];

		transform.translate(newX, newY, newZ);

		for(i = children.begin(); i != children.end(); i++)
			(*i)->update(time);

		children.remove_if(is_dead);
	}
	
	void drawChildren(const GLMatrix4 &t, const GLMatrix4 &nt) 
	{
		for(i = children.begin(); i != children.end(); i++)
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
		transform.setIdentity();
		alive = true;
	}
	
	virtual void draw(const GLMatrix4 &parentTransform, const GLMatrix4 &parentNormalTransform) 
	{
		if(tornado)
		{
			for(int i = 0; i < 4; i++)
				vertices[i].color = 0xFF0033CC;
		}
		else
		{
			for(int i = 0; i < 4; i++)
				vertices[i].color = 0xFFCC3300;
		}
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
	GLMatrix4 rot;
	Vtx vtx[6*6];
public:
	CubeNode(GLfloat size, int color) 
	{
		GLuint colors[6];
		if(color == 0)
		{
			for(int i = 0; i < 6; i++)
			{
				colors[i] = (rand() % 0x100000000) | 0xFF000000;
			}
		}
		else
		{
			for(int i = 0; i < 6; i++)
			{
				colors[i] = color;
			}
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

		rot.setRotation(rand(), rand(), rand(), 0.005);
	}

	virtual void update(unsigned long long time)
	{
		SceneNode::update(time);
		alive = z[POS] > 0;
		if(tornado)
		{
			x[ACC] = (log(z[POS])/100 * (sin((float)(time - birthTime)/10.0) - y[POS]/10)) / 50;
			y[ACC] = -(log(z[POS])/100 * (cos((float)(time - birthTime)/10.0) - x[POS]/10)) / 50;
			z[ACC] = 0.00001;
		}
		else
		{
			x[ACC] = 0;
			y[ACC] = 0;
			z[ACC] = -GRAVITY;
		}

		transform = transform * rot;
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

class PyramidNode : public SceneNode {
	vector<Vtx> vertices;
	GLfloat height;
public:
	PyramidNode(GLfloat radius, GLfloat height, GLuint sides, GLuint tipColor, GLuint color) : vertices(2*(2 + max(sides,3u))), height(height) {
		sides = max(sides,3u);
		vertices[0].x = vertices[0].y = 0;
		vertices[0].z = height;
		vertices[0].color = tipColor;
		vertices[0].nx = 0;
		vertices[0].ny = 0;
		vertices[0].nz = 1;
		
		const GLuint skip = vertices.size()/2;
		
		vertices[skip] = vertices.front();
		vertices[skip].color = color;
		vertices[skip].z = 0;
		vertices[skip].nz = -1;
		//||perp(<radius, 0, 0> - <0, 0, height>)|| = ||perp(radius, 0, -height)|| = <height, 0, radius>/sqrt(height^2 + radius^2)
		const double normFactor = 1.0/sqrt(height*height + radius*radius);
		const double baseNormX = height * normFactor, baseNormZ = radius * normFactor;
		for ( size_t i = 0; i < sides; ++i ) {
			const double angle = 2.0 * i * PI/(double)sides;
			const double c = cos(angle), s = sin(angle);
			vertices[i + 1].x = radius * c;
			vertices[i + 1].y = radius * s;
			vertices[i + 1].z = 0;
			vertices[i + 1].color = color;
			//Rz(||<height, 0, radius>||) 
			vertices[i + 1].nx = c * baseNormX;
			vertices[i + 1].ny = s * baseNormX;
			vertices[i + 1].nz = baseNormZ;
			
			vertices[skip+i+1] = vertices[i+1];
			vertices[skip+i+1].y *= -1;
			vertices[skip+i+1].nx = 0;
			vertices[skip+i+1].ny = 0;
			vertices[skip+i+1].nz = -1;
		}
		vertices[skip-1].x = radius;
		vertices[skip-1].y = 0;
		vertices[skip-1].z = 0;
		vertices[skip-1].nx = baseNormX;
		vertices[skip-1].ny = 0;
		vertices[skip-1].nz = baseNormZ;
		vertices[skip-1].color = color;
		
		vertices.back() = vertices[skip-1];
		vertices.back().nx = 0;
		vertices.back().ny = 0;
		vertices.back().nz = -1;
	}
	
	virtual void draw(const GLMatrix4 &parentTransform, const GLMatrix4 &parentNormTransform) {
		const GLMatrix4 &t = parentTransform * transform,
		&nt = parentNormTransform * normTransform;
		bindVertexArray(&vertices[0]);
		setTransform(t, nt);
		
		
		glDrawArrays(GL_TRIANGLE_FAN, 0, vertices.size()/2);
		glDrawArrays(GL_TRIANGLE_FAN, vertices.size()/2, vertices.size()/2);		
		
		drawChildren(t, nt);
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