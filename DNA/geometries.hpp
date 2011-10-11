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