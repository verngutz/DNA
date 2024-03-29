#define ENABLE
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <algorithm>
#include <gl\glew.h>
#include <gl\glfw.h>

using namespace std;

static const double PI = 3.14159265358979323846264338327;

struct Vtx 
{
	GLfloat x, y, z;
	GLfloat nx, ny, nz;
	GLuint color;
};

enum 
{
	ATTRIB_POS, 
	ATTRIB_COLOR,
	ATTRIB_NORMAL 
};

void bindVertexArray(Vtx *vtx) 
{
	glVertexAttribPointer(ATTRIB_POS, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), &(vtx->x));
	glVertexAttribPointer(ATTRIB_COLOR, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vtx), &(vtx->color));
	glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(Vtx), &(vtx->nx));
}

bool loadShaderSource(GLuint shader, const char *filePath) 
{
	FILE *f = fopen(filePath, "r");
	if ( !f ) 
	{
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
	if ( logLength > 0 )
	{
		GLchar *log = new GLchar[logLength];
		glGetShaderInfoLog(shader, logLength, &logLength, log);
		std::cout << "Shader Compile Log:\n" << log << std::endl;
		delete [] log;
	}
	
	return true;
}

void multiply_matrix_4x4(const GLfloat *A, const GLfloat *B, GLfloat *C) 
{
	for ( size_t i = 0; i < 16; ++i ) 
	{
		C[i] = 0;
		const size_t aBase = i % 4, bBase = (i/4) * 4;
		for ( size_t j = 0; j < 4; ++j ) 
		{
			C[i] += A[aBase + j * 4] * B[bBase + j];
		}
	}
}

void create_rotation_matrix_4x4(GLfloat x, GLfloat y, GLfloat z, GLfloat theta, GLfloat *output) 
{
	for ( size_t i = 0; i < 16; ++i ) 
	{
		if ( i % 5 == 0 )
			output[i] = 1;
		else
			output[i] = 0;
	}
	
	GLfloat m = sqrt(x * x + y * y + z * z);
	if ( m < 0.00001 )
		return;

	x /= m; 
	y /= m; 
	z /= m;
	
	const GLfloat	c = cos(theta), 
					s = sin(theta);
	
	GLfloat normalizationFactor = z*z + y*y;

	GLfloat alignMat[16];
	if ( normalizationFactor < 0.0001 ) 
	{
	  normalizationFactor = y*y + x*x;
	  alignMat[0] = x,	alignMat[1] = -x*z,		alignMat[2] = y,	alignMat[3] = 0;
	  alignMat[4] = y,	alignMat[5] = -y*z,		alignMat[6] = -x,	alignMat[7] = 0;
	  alignMat[8] = z,	alignMat[9] = y*y+x*x,	alignMat[10] = 0,	alignMat[11] = 0;
	  alignMat[12] = 0,	alignMat[13] = 0,		alignMat[14] = 0,	alignMat[15] = 1;
	} 
	else 
	{
	  alignMat[0] = x,	alignMat[1] = z*z+y*y,	alignMat[2] = 0,	alignMat[3] = 0;
	  alignMat[4] = y,	alignMat[5] = -x*y,		alignMat[6] = z,	alignMat[7] = 0;
	  alignMat[8] = z,	alignMat[9] = -x*z,		alignMat[10] = -y,	alignMat[11] = 0;
	  alignMat[12] = 0,	alignMat[13] = 0,		alignMat[14] = 0,	alignMat[15] = 1;
	}

	GLfloat tmp[16];
	
	normalizationFactor = 1.0/sqrt(normalizationFactor);
	alignMat[1] *= normalizationFactor;
	alignMat[5] *= normalizationFactor;
	alignMat[9] *= normalizationFactor;
	
	alignMat[2] *= normalizationFactor;
	alignMat[6] *= normalizationFactor;
	alignMat[10] *= normalizationFactor;

	memcpy(output, alignMat, sizeof(tmp));
	
	GLfloat xRot[16] = 
	{
		1, 0, 0, 0,
		0, c, s, 0,
		0, -s, c, 0,
		0, 0, 0, 1
	};
	
	multiply_matrix_4x4(xRot,output,tmp);

	memcpy(output, tmp, sizeof(tmp));

	for ( size_t k = 0; k < 4; ++k ) 
	{
		for ( size_t j = k; j < 4; ++j ) 
		{
			const size_t other = j * 4 + k;
			const size_t i = k * 4 + j;
			
			const GLfloat tmp = alignMat[i];
			alignMat[i] = alignMat[other];
			alignMat[other] = tmp;
		}		
	}
	
	multiply_matrix_4x4(alignMat, output, tmp);
	memcpy(output, tmp, sizeof(tmp));

}

struct GLMatrix4 
{
	GLfloat mat[16];
	
	void setIdentity() 
	{
		mat[0] = 1, mat[4] = 0, mat[8] = 0, mat[12] = 0;
		mat[1] = 0, mat[5] = 1, mat[9] = 0, mat[13] = 0;
		mat[2] = 0, mat[6] = 0, mat[10] = 1, mat[14] = 0;
		mat[3] = 0, mat[7] = 0, mat[11] = 0, mat[15] = 1;
	}
	
	void setRotation(GLfloat x, GLfloat y, GLfloat z, GLfloat theta) 
	{
		create_rotation_matrix_4x4(x, y, z, theta, mat);
	}
	
	void setTranslation(GLfloat x, GLfloat y, GLfloat z) 
	{
		mat[0] = 1, mat[4] = 0, mat[8] = 0, mat[12] = x;
		mat[1] = 0, mat[5] = 1, mat[9] = 0, mat[13] = y;
		mat[2] = 0, mat[6] = 0, mat[10] = 1, mat[14] = z;
		mat[3] = 0, mat[7] = 0, mat[11] = 0, mat[15] = 1;
	}
	
	void translate(GLfloat x, GLfloat y, GLfloat z) 
	{
		mat[12] += x;
		mat[13] += y;
		mat[14] += z;
	}
	
	void scale(GLfloat sx, GLfloat sy, GLfloat sz) 
	{
		mat[0] *= sx;
		mat[4] *= sx;
		mat[8] *= sx;
		mat[12] *= sx;
		
		mat[1] *= sy;
		mat[5] *= sy;
		mat[9] *= sy;
		mat[13] *= sy;
		
		mat[2] *= sz;
		mat[6] *= sz;
		mat[10] *= sz;
		mat[14] *= sz;
	}
	
	void mult(GLfloat &x, GLfloat &y, GLfloat &z, GLfloat &w) 
	{
		GLfloat ox = x, oy = y, oz = z, ow = w;
		x = mat[0] * ox + mat[4] * oy + mat[8] * oz + mat[12] * ow;
		y = mat[1] * ox + mat[5] * oy + mat[9] * oz + mat[13] * ow;
		z = mat[2] * ox + mat[6] * oy + mat[10] * oz + mat[14] * ow;
		w = mat[3] * ox + mat[7] * oy + mat[11] * oz + mat[15] * ow;
	}
	
	void transpose() 
	{
		std::swap(mat[4],mat[1]);
		std::swap(mat[8],mat[2]);
		std::swap(mat[12],mat[3]);
		std::swap(mat[9],mat[6]);
		std::swap(mat[13],mat[7]);
		std::swap(mat[14], mat[11]);
	}
	
	GLMatrix4& operator=(const GLMatrix4 &rhs) 
	{
		memcpy(mat, rhs.mat, sizeof(mat));
		return *this;
	}
	
	GLMatrix4 operator*(const GLMatrix4 &rhs) const 
	{
		GLMatrix4 ret;
		
		multiply_matrix_4x4(mat, rhs.mat, ret.mat);
		
		return ret;
	}
	
	GLMatrix4& operator*=(const GLMatrix4 &rhs) 
	{
		GLfloat tmp[16];
		multiply_matrix_4x4(mat, rhs.mat, tmp);
		
		memcpy(mat, tmp, sizeof(mat));
		return *this;
	}
	
	void setOrtho(const GLfloat l, const GLfloat r,
				  const GLfloat b, const GLfloat t,
				  const GLfloat n, const GLfloat f) 
	{
		
		mat[0] = 2/(r-l),	mat[4] = 0,			mat[8] = 0,				mat[12] = -(r+l)/(r-l);
		mat[1] = 0,			mat[5] = 2/(t-b),	mat[9] = 0,				mat[13] = -(t+b)/(t-b);
		mat[2] = 0,			mat[6] = 0,			mat[10] = 2/(f-n),		mat[14] = -(n+f)/(f-n);
		mat[3] = 0,			mat[7] = 0,			mat[11] = 0,			mat[15] = 1;
	}

	void setOblique(const GLfloat vx, const GLfloat vy, const GLfloat vz, const GLfloat n) 
	{
		mat[0] = 1, mat[4] = 0, mat[8] = -vx/vz,	mat[12] = n*vx/vz;
		mat[1] = 0, mat[5] = 1, mat[9] = -vy/vz,	mat[13] = n*vy/vz;
		mat[2] = 0, mat[6] = 0, mat[10] = 1,		mat[14] = 0;
		mat[3] = 0, mat[7] = 0, mat[11] = 0,		mat[15] = 1;
	}
	
	void setPerspective(const GLfloat fovy, const GLfloat aspect, const GLfloat near, const GLfloat far) 
	{
		const GLfloat cott = 1.0/tan(fovy/2);
		mat[0] = cott/aspect,	mat[4] = 0,		mat[8] = 0,								mat[12] = 0;
		mat[1] = 0,				mat[5] = cott,	mat[9] = 0,								mat[13] = 0;
		mat[2] = 0,				mat[6] = 0,		mat[10] = (near+far)/(near-far),		mat[14] = 2*far*near/(near-far);
		mat[3] = 0,				mat[7] = 0,		mat[11] = -1,							mat[15] = 1;
	}
};

void cross(const GLfloat *a, const GLfloat *b, GLfloat *output) 
{
	output[0] = a[1]*b[2] - a[2]*b[1];
	output[1] = a[2]*b[0] - a[0]*b[2];
	output[2] = a[0]*b[1] - a[1]*b[0];
}

void lookAt(GLfloat camX, GLfloat camY, GLfloat camZ, 
			GLfloat tX, GLfloat tY, GLfloat tZ,
			GLfloat uX, GLfloat uY, GLfloat uZ, GLfloat *output) 
{
	GLfloat z[] = { camX - tX, camY - tY, camZ - tZ };
	
	{
		const GLfloat dMag = sqrt(z[0] * z[0] + z[1] * z[1] + z[2] * z[2]);
		if ( dMag < 0.00001 ) return;
		z[0] /= dMag, z[1] /= dMag, z[2] /= dMag;
	}
	
	const GLfloat u[] = {uX, uY, uZ};
	GLfloat x[3], y[3];
	cross(u, z, x);
	cross(z, x, y);
	
	output[0] = x[0],	output[4] = x[1],	output[8] = x[2];
	output[1] = y[0],	output[5] = y[1],	output[9] = y[2];
	output[2] = z[0],	output[6] = z[1],	output[10] = z[2];
	output[3] = 0,		output[7] = 0,		output[11] = 0;
	
	output[12] = -camX * x[0] - camY * x[1] - camZ * x[2];
	output[13] = -camX * y[0] - camY * y[1] - camZ * y[2];
	output[14] = -camX * z[0] - camY * z[1] - camZ * z[2];
	output[15] = 1;
}

#define SWAP_ROWS_DOUBLE(a, b) { GLdouble *_tmp = a; (a)=(b); (b)=_tmp; }
#define SWAP_ROWS_FLOAT(a, b) { GLfloat *_tmp = a; (a)=(b); (b)=_tmp; }
#define MAT(m, r, c) (m)[(c) * 4 + (r)]

GLuint invert(const GLfloat *m, GLfloat *out)
{
	GLfloat wtmp[4][8];
	GLfloat m0, m1, m2, m3, s;
	GLfloat *r0, *r1, *r2, *r3;
	r0 = wtmp[0], r1 = wtmp[1], r2 = wtmp[2], r3 = wtmp[3];
	r0[0] = MAT(m, 0, 0), r0[1] = MAT(m, 0, 1),
    r0[2] = MAT(m, 0, 2), r0[3] = MAT(m, 0, 3),
    r0[4] = 1.0, r0[5] = r0[6] = r0[7] = 0.0,
    r1[0] = MAT(m, 1, 0), r1[1] = MAT(m, 1, 1),
    r1[2] = MAT(m, 1, 2), r1[3] = MAT(m, 1, 3),
    r1[5] = 1.0, r1[4] = r1[6] = r1[7] = 0.0,
    r2[0] = MAT(m, 2, 0), r2[1] = MAT(m, 2, 1),
    r2[2] = MAT(m, 2, 2), r2[3] = MAT(m, 2, 3),
    r2[6] = 1.0, r2[4] = r2[5] = r2[7] = 0.0,
    r3[0] = MAT(m, 3, 0), r3[1] = MAT(m, 3, 1),
    r3[2] = MAT(m, 3, 2), r3[3] = MAT(m, 3, 3),
    r3[7] = 1.0, r3[4] = r3[5] = r3[6] = 0.0;
   
	if (fabsf(r3[0]) > fabsf(r2[0]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (fabsf(r2[0]) > fabsf(r1[0]))
		SWAP_ROWS_FLOAT(r2, r1);
	if (fabsf(r1[0]) > fabsf(r0[0]))
		SWAP_ROWS_FLOAT(r1, r0);
	if (0.0 == r0[0])
		return GL_FALSE;

	m1 = r1[0] / r0[0];
	m2 = r2[0] / r0[0];
	m3 = r3[0] / r0[0];
	s = r0[1];
	r1[1] -= m1 * s;
	r2[1] -= m2 * s;
	r3[1] -= m3 * s;
	s = r0[2];
	r1[2] -= m1 * s;
	r2[2] -= m2 * s;
	r3[2] -= m3 * s;
	s = r0[3];
	r1[3] -= m1 * s;
	r2[3] -= m2 * s;
	r3[3] -= m3 * s;
	s = r0[4];
	if (s != 0.0) 
	{
		r1[4] -= m1 * s;
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r0[5];
	if (s != 0.0) 
	{
		r1[5] -= m1 * s;
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r0[6];
	if (s != 0.0) 
	{
		r1[6] -= m1 * s;
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r0[7];
	if (s != 0.0) 
	{
		r1[7] -= m1 * s;
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}
   
	if (fabsf(r3[1]) > fabsf(r2[1]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (fabsf(r2[1]) > fabsf(r1[1]))
		SWAP_ROWS_FLOAT(r2, r1);
	if (0.0 == r1[1])
		return GL_FALSE;

	m2 = r2[1] / r1[1];
	m3 = r3[1] / r1[1];
	r2[2] -= m2 * r1[2];
	r3[2] -= m3 * r1[2];
	r2[3] -= m2 * r1[3];
	r3[3] -= m3 * r1[3];
	s = r1[4];
	if (0.0 != s) 
	{
		r2[4] -= m2 * s;
		r3[4] -= m3 * s;
	}
	s = r1[5];
	if (0.0 != s) 
	{
		r2[5] -= m2 * s;
		r3[5] -= m3 * s;
	}
	s = r1[6];
	if (0.0 != s) 
	{
		r2[6] -= m2 * s;
		r3[6] -= m3 * s;
	}
	s = r1[7];
	if (0.0 != s) 
	{
		r2[7] -= m2 * s;
		r3[7] -= m3 * s;
	}
   
	if (fabsf(r3[2]) > fabsf(r2[2]))
		SWAP_ROWS_FLOAT(r3, r2);
	if (0.0 == r2[2])
		return GL_FALSE;

	m3 = r3[2] / r2[2];
	r3[3] -= m3 * r2[3], r3[4] -= m3 * r2[4],
    r3[5] -= m3 * r2[5], r3[6] -= m3 * r2[6], r3[7] -= m3 * r2[7];

	if (0.0 == r3[3])
		return GL_FALSE;
	s = 1.0 / r3[3];
	r3[4] *= s;
	r3[5] *= s;
	r3[6] *= s;
	r3[7] *= s;
	m2 = r2[3];
	s = 1.0 / r2[2];
	r2[4] = s * (r2[4] - r3[4] * m2), r2[5] = s * (r2[5] - r3[5] * m2),
	r2[6] = s * (r2[6] - r3[6] * m2), r2[7] = s * (r2[7] - r3[7] * m2);
	m1 = r1[3];
	r1[4] -= r3[4] * m1, r1[5] -= r3[5] * m1,
    r1[6] -= r3[6] * m1, r1[7] -= r3[7] * m1;
	m0 = r0[3];
	r0[4] -= r3[4] * m0, r0[5] -= r3[5] * m0,
    r0[6] -= r3[6] * m0, r0[7] -= r3[7] * m0;
	m1 = r1[2];
	s = 1.0 / r1[1];
	r1[4] = s * (r1[4] - r2[4] * m1), r1[5] = s * (r1[5] - r2[5] * m1),
    r1[6] = s * (r1[6] - r2[6] * m1), r1[7] = s * (r1[7] - r2[7] * m1);
	m0 = r0[2];
	r0[4] -= r2[4] * m0, r0[5] -= r2[5] * m0,
    r0[6] -= r2[6] * m0, r0[7] -= r2[7] * m0;
	m0 = r0[1];
	s = 1.0 / r0[0];
	r0[4] = s * (r0[4] - r1[4] * m0), r0[5] = s * (r0[5] - r1[5] * m0),
	r0[6] = s * (r0[6] - r1[6] * m0), r0[7] = s * (r0[7] - r1[7] * m0);
	MAT(out, 0, 0) = r0[4];
	MAT(out, 0, 1) = r0[5], MAT(out, 0, 2) = r0[6];
	MAT(out, 0, 3) = r0[7], MAT(out, 1, 0) = r1[4];
	MAT(out, 1, 1) = r1[5], MAT(out, 1, 2) = r1[6];
	MAT(out, 1, 3) = r1[7], MAT(out, 2, 0) = r2[4];
	MAT(out, 2, 1) = r2[5], MAT(out, 2, 2) = r2[6];
	MAT(out, 2, 3) = r2[7], MAT(out, 3, 0) = r3[4];
	MAT(out, 3, 1) = r3[5], MAT(out, 3, 2) = r3[6];
	MAT(out, 3, 3) = r3[7];
	return GL_TRUE;
}

GLuint unproject(const GLint winx, const GLint winy, const GLfloat *view, const GLfloat *projection, GLint *viewport, GLfloat *rayEnd1, GLfloat *rayEnd2)
{
    GLfloat in1[16], in2[16], out1[16], out2[16];

	GLfloat A[16];
	multiply_matrix_4x4(projection, view, A);
	
    GLfloat m[16];
    if(invert(A, m) == GL_FALSE)
        return GL_FALSE;

    in1[0] = in2[0] = (winx - (GLfloat)viewport[0]) / (GLfloat)viewport[2] * 2.0 - 1.0;
    in1[1] = in2[1] = (winy - (GLfloat)viewport[1]) / (GLfloat)viewport[3] * 2.0 - 1.0;
    in1[2] = -1;
	in2[2] = 1;
    in1[3] = in2[3] = 1;

    multiply_matrix_4x4(m, in1, out1);
	multiply_matrix_4x4(m, in2, out2);

    if(out1[3] == 0 || out2[3] == 0)
        return GL_FALSE;

    out1[3] = 1.0 / out1[3];
    rayEnd1[0] = out1[0] * out1[3];
    rayEnd1[1] = out1[1] * out1[3];
    rayEnd1[2] = out1[2] * out1[3];

	
	out2[3] = 1.0 / out2[3];
    rayEnd2[0] = out2[0] * out2[3];
    rayEnd2[1] = out2[1] * out2[3];
    rayEnd2[2] = out2[2] * out2[3];
	
    return GL_TRUE;
}