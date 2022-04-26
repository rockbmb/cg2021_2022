#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdio>
#include <glm/glm.hpp>
// Necessário para poder usar glm::value_ptr
#include <glm/gtc/type_ptr.hpp>

#include <vector>
using namespace std;


float camX = 0, camY, camZ = 5;
int startX, startY, tracking = 0;

int alpha = 0, beta = 0, r = 5;

// Used to draw the teapot and Catmull-Rom curve in renderScene().
vector<glm::vec3> positions;
vector<glm::vec3> derivatives;
vector<glm::vec3> x_versors;
vector<glm::vec3> y_versors;
vector<glm::vec3> z_versors;

// Used to control the precision of the Catmull-Rom curve.
int tesselation = 100;

#define POINT_COUNT 5
// Points that make up the loop for catmull-rom interpolation
float p[POINT_COUNT][3] = {{-1,-1,0},{-1,1,0},{1,1,0},{0,0,0},{1,-1,0}};

void buildRotMatrix(float *x, float *y, float *z, float *m) {

	m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
	m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
	m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
	m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}


void cross(float *a, float *b, float *res) {

	res[0] = a[1]*b[2] - a[2]*b[1];
	res[1] = a[2]*b[0] - a[0]*b[2];
	res[2] = a[0]*b[1] - a[1]*b[0];
}


void normalize(float *a) {

	float l = sqrt(a[0]*a[0] + a[1] * a[1] + a[2] * a[2]);
	a[0] = a[0]/l;
	a[1] = a[1]/l;
	a[2] = a[2]/l;
}


float length(float *v) {

	float res = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
	return res;

}

void multMatrixVector(float *m, float *v, float *res) {

	for (int j = 0; j < 4; ++j) {
		res[j] = 0;
		for (int k = 0; k < 4; ++k) {
			res[j] += v[k] * m[j * 4 + k];
		}
	}

}


void getCatmullRomPoint(
	float t,
	float *p0,
	float *p1,
	float *p2,
	float *p3,
	glm::vec3 *pos,
	glm::vec3 *deriv,
	glm::vec3 prev_y_versor,
	glm::vec3 *x_versor,
	glm::vec3 *y_versor,
	glm::vec3 *z_versor) {
	glm::mat4x3 p (
		p0[0], p0[1], p0[2],
		p1[0], p1[1], p1[2],
		p2[0], p2[1], p2[2],
		p3[0], p3[1], p3[2]
	);

	// catmull-rom matrix
	glm::mat4x4 m (
		-0.5f,  1.5f, -1.5f,  0.5f,
		 1.0f, -2.5f,  2.0f, -0.5f,
		-0.5f,  0.0f,  0.5f,  0.0f,
		 0.0f,  1.0f,  0.0f,  0.0f
	);

	// Compute pos = T * A
	glm::vec4 t_ (pow(t, 3), pow(t, 2), t, 1.0);
	*pos = p * m * t_;

	// compute deriv = T' * A
	glm::vec4 t_prime (3 * pow(t, 2), 2 * t, 1, 0);
	*deriv = p * m * t_prime;
	deriv->x = deriv->x / 3;
	deriv->y = deriv->y / 3;
	deriv->z = deriv->z / 3;

	// ...
	*x_versor = glm::normalize(*deriv);
	*z_versor = glm::normalize(glm::cross(*x_versor, prev_y_versor));
	*y_versor = glm::normalize(glm::cross(*z_versor, *x_versor));
}


// given  global t, returns the point in the curve
void getGlobalCatmullRomPoint(
	float gt,
	glm::vec3 *pos,
	glm::vec3 *deriv,
	glm::vec3 prev_y_versor,
	glm::vec3 *x_versor,
	glm::vec3 *y_versor,
	glm::vec3 *z_versor) {

	float t = gt * POINT_COUNT; // this is the real global t
	int index = floor(t);  // which segment
	t = t - index; // where within  the segment

	// indices store the points
	int indices[4];
	indices[0] = (index + POINT_COUNT-1)%POINT_COUNT;	
	indices[1] = (indices[0]+1)%POINT_COUNT;
	indices[2] = (indices[1]+1)%POINT_COUNT; 
	indices[3] = (indices[2]+1)%POINT_COUNT;

	getCatmullRomPoint(t, p[indices[0]], p[indices[1]], p[indices[2]], p[indices[3]], pos, deriv, prev_y_versor, x_versor, y_versor, z_versor);
}


void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if(h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Reset the coordinate system before modifying
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set the correct perspective
	gluPerspective(45,ratio,1,1000);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}


void renderCatmullRomCurve() {
	glm::vec3 position;
	glm::vec3 derivative;
	glm::vec3 x_versor;
	glm::vec3 y_versor;
	glm::vec3 z_versor;

	positions.clear();
	derivatives.clear();
	x_versors.clear();
	y_versors.clear();
	z_versors.clear();

	y_versors.push_back(glm::vec3 (0.0, 1.0, 0.0));

// draw curve using line segments with GL_LINE_LOOP
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < tesselation; i++) {
		float t = (float) i / (float) tesselation;
		getGlobalCatmullRomPoint(t, &position, &derivative, y_versors[i], &x_versor, &y_versor, &z_versor);
		positions.push_back(position);
		derivatives.push_back(derivative);
		x_versors.push_back(x_versor);
		y_versors.push_back(y_versor);
		z_versors.push_back(z_versor);
	}

	for (int i = 0; i < positions.size(); i++) {
		glVertex3f(positions[i][0], positions[i][1], positions[i][2]);
	}
	glEnd();
	
	glBegin(GL_LINES);
	for (int i = 0; i < positions.size(); i++) {
		glPushMatrix();
		glTranslatef(positions[i].x, positions[i].y, positions[i].z);
		glVertex3f(positions[i].x + derivatives[i].x, positions[i].y + derivatives[i].y, positions[i].z + derivatives[i].z);
		glVertex3f(positions[i].x, positions[i].y, positions[i].z);
		glPopMatrix();
	}
	glEnd();
}

void drawTeapotAlongCatmullRomCurve() {
	//int j = 1 + (int) ((0.5 * (1 + sin(2 * M_PI * glutGet(GLUT_ELAPSED_TIME)/4000))) * (float) tesselation);
	/*
	Remember, x_versors and z_versors have tesselation = 100 elements, and
	y_versors has tesselation + 1 = 101 elements - the first is the y_versor y_0
	used to initialize the process, it is not used.
	*/

	int i = glutGet(GLUT_ELAPSED_TIME)/40 % (tesselation + 1);
	glm::mat4 rot_matrix (
		x_versors[i - 1].x, x_versors[i - 1].y, x_versors[i - 1].z, 0,
		    y_versors[i].x,     y_versors[i].y,     y_versors[i].z, 0,
		z_versors[i - 1].x, z_versors[i - 1].y, z_versors[i - 1].z, 0,
		0,                  0,                  0,                  1
	);
	glPushMatrix();
	glTranslatef(positions[i - 1].x, positions[i - 1].y, positions[i - 1].z);
	glMultMatrixf(glm::value_ptr(rot_matrix));
	glutWireCone(0.1, 1, 10, 10);
	glPopMatrix();
	glutPostRedisplay();
}

void renderScene(void) {

	static float t = 0;

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(camX, camY, camZ, 
		      0.0,0.0,0.0,
			  0.0f,1.0f,0.0f);

	renderCatmullRomCurve();

	// apply transformations here
	// ...
	drawTeapotAlongCatmullRomCurve();

	glutSwapBuffers();
	t+=0.00001;
}


void processMouseButtons(int button, int state, int xx, int yy) 
{
	if (state == GLUT_DOWN)  {
		startX = xx;
		startY = yy;
		if (button == GLUT_LEFT_BUTTON)
			tracking = 1;
		else if (button == GLUT_RIGHT_BUTTON)
			tracking = 2;
		else
			tracking = 0;
	}
	else if (state == GLUT_UP) {
		if (tracking == 1) {
			alpha += (xx - startX);
			beta += (yy - startY);
		}
		else if (tracking == 2) {
			
			r -= yy - startY;
			if (r < 3)
				r = 3.0;
		}
		tracking = 0;
	}
}


void processMouseMotion(int xx, int yy)
{
	int deltaX, deltaY;
	int alphaAux, betaAux;
	int rAux;

	if (!tracking)
		return;

	deltaX = xx - startX;
	deltaY = yy - startY;

	if (tracking == 1) {

		alphaAux = alpha + deltaX;
		betaAux = beta + deltaY;

		if (betaAux > 85.0)
			betaAux = 85.0;
		else if (betaAux < -85.0)
			betaAux = -85.0;

		rAux = r;
	}
	else if (tracking == 2) {

		alphaAux = alpha;
		betaAux = beta;
		rAux = r - deltaY;
		if (rAux < 1.5)
			rAux = 1.5;
	}
	camX = rAux * sin(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
	camZ = rAux * cos(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
	camY = rAux *							     sin(betaAux * 3.14 / 180.0);
}


int main(int argc, char **argv) {

// inicialization
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(500,500);
	glutCreateWindow("CG@DI-UM");
		
// callback registration 
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);

// mouse callbacks
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);

// OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

// enter GLUT's main cycle 
	glutMainLoop();
	
	return 1;
}

