#include <stdlib.h>
#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#include <vector>
using namespace std;

#include <math.h>

#define _PI_ 3.14159

float alfa = 0.0f, beta = 0.0f, radius = 5.0f;
float camX, camY, camZ;

GLuint vertexCount, vbo_buffers[2];

int timebase = 0, frame = 0;

void sphericalToCartesian() {

	camX = radius * cos(beta) * sin(alfa);
	camY = radius * sin(beta);
	camZ = radius * cos(beta) * cos(alfa);
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



void prepareCilinder(float height, float radius, int sides) {

	vector<float> v;
	vector<float> norms;

	int vertex = 0;
	float delta = 2.0f * _PI_ / sides;

	for (int i = 0; i < sides; ++i) {
		// top
		// central point
		v.push_back(0.0f);
		v.push_back(height /2.0f);
		v.push_back(0.0f);
		vertex++;
		norms.push_back(0.0f);
		norms.push_back(1.0f);
		norms.push_back(0.0f);

		v.push_back(radius * sin( i * delta));
		v.push_back(height /2.0f);
		v.push_back(radius * cos( i * delta));
		vertex++;
		norms.push_back(0.0f);
		norms.push_back(1.0f);
		norms.push_back(0.0f);

		v.push_back(radius * sin( (i+1) * delta));
		v.push_back(height /2.0f);
		v.push_back(radius * cos( (i+1) * delta));
		vertex++;
		norms.push_back(0.0f);
		norms.push_back(1.0f);
		norms.push_back(0.0f);

		// body
		// tri�ngulo 1
		v.push_back(radius * sin( (i+1) * delta));
		v.push_back(height /2.0f);
		v.push_back(radius * cos( (i+1) * delta));
		vertex++;
		norms.push_back(sin( (i+1) * delta));
		norms.push_back(0);
		norms.push_back(cos( (i+1) * delta));

		v.push_back(radius * sin( i * delta));
		v.push_back(height /2.0f);
		v.push_back(radius * cos( i * delta));
		vertex++;
		norms.push_back(sin( (i) * delta));
		norms.push_back(0);
		norms.push_back(cos( (i) * delta));

		v.push_back(radius * sin( i * delta));
		v.push_back(-height /2.0f);
		v.push_back(radius * cos( i * delta));
		vertex++;
		norms.push_back(sin( (i) * delta));
		norms.push_back(0);
		norms.push_back(cos( (i) * delta));

		// triangle 2
		v.push_back(radius * sin( (i+1) * delta));
		v.push_back(-height /2.0f);
		v.push_back(radius * cos( (i+1) * delta));
		vertex++;
		norms.push_back(sin( (i+1) * delta));
		norms.push_back(0);
		norms.push_back(cos( (i+1) * delta));

		v.push_back(radius * sin( (i+1) * delta));
		v.push_back(height /2.0f);
		v.push_back(radius * cos( (i+1) * delta));
		vertex++;
		norms.push_back(sin( (i+1) * delta));
		norms.push_back(0);
		norms.push_back(cos( (i+1) * delta));

		v.push_back(radius * sin( i * delta));
		v.push_back(-height /2.0f);
		v.push_back(radius * cos( i * delta));
		vertex++;
		norms.push_back(sin( (i) * delta));
		norms.push_back(0);
		norms.push_back(cos( (i) * delta));

		// base
		// central point
		v.push_back(0.0f);
		v.push_back(-height /2.0f);
		v.push_back(0.0f);
		vertex++;
		norms.push_back(0.0f);
		norms.push_back(-1.0f);
		norms.push_back(0.0f);

		v.push_back(radius * sin( (i+1) * delta));
		v.push_back(-height /2.0f);
		v.push_back(radius * cos( (i+1) * delta));
		vertex++;
		norms.push_back(0.0f);
		norms.push_back(-1.0f);
		norms.push_back(0.0f);

		v.push_back(radius * sin( i * delta));
		v.push_back(-height /2.0f);
		v.push_back(radius * cos( i * delta));
		vertex++;
		norms.push_back(0.0f);
		norms.push_back(-1.0f);
		norms.push_back(0.0f);
	}

	vertexCount = vertex;

	/*
	Alterar norma de vetores normais.
	for (int k = 0; k < norms.size(); k++) {
		norms[k] *= 2;
	}
	*/

	glGenBuffers(2, vbo_buffers);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, v.data(),     GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexCount * 3, norms.data(), GL_STATIC_DRAW);


}


void drawCilinder() {
		
	glBindBuffer(GL_ARRAY_BUFFER, vbo_buffers[0]);
	glVertexPointer(3,GL_FLOAT, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo_buffers[1]);
	glNormalPointer(GL_FLOAT, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}


void renderScene(void) {
	float fps;
	int time;
	char s[64];

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();

	gluLookAt(camX,camY,camZ, 
		      0.0,0.0,0.0,
			  0.0f,1.0f,0.0f);

	//glRotatef(45, 0, 1, 0);

	float pos[4] = {0.0, 0.0, 5.0, 1.0};
	/* última coordenada = 1 => ponto
	float pos[4] = {1.0, 1.0, 1.0, 1.0};
	*/
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	// Color for cylinder's material.
	float dark[] = { 0.2, 0.2, 0.6, 1.0 };
	float white[] = { 0.8, 0.8, 0.8, 1.0 };
	float red[] = { 0.8, 0.2, 0.2, 1.0 };
	float green[] = { 0.2, 0.8, 0, 1.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, dark);
	//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
	glMaterialfv(GL_FRONT, GL_SPECULAR, green);
	//glMaterialf(GL_FRONT, GL_SHININESS, 128);

	//drawCilinder();
	glutSolidSphere(1, 100, 100);

	frame++;
	time=glutGet(GLUT_ELAPSED_TIME); 
	if (time - timebase > 1000) { 
		fps = frame*1000.0/(time-timebase); 
		timebase = time; 
		frame = 0; 
		sprintf(s, "FPS: %f6.2", fps);
		glutSetWindowTitle(s);
	} 

// End of frame
	glutSwapBuffers();
}



void processKeys(int key, int xx, int yy) 
{
	switch(key) {
	
		case GLUT_KEY_RIGHT: 
						alfa -=0.1; break;

		case GLUT_KEY_LEFT: 
						alfa += 0.1; break;

		case GLUT_KEY_UP : 
						beta += 0.1f;
						if (beta > 1.5f)
							beta = 1.5f;
						break;

		case GLUT_KEY_DOWN: 
						beta -= 0.1f; 
						if (beta < -1.5f)
							beta = -1.5f;
						break;

		case GLUT_KEY_PAGE_DOWN : radius -= 0.1f; 
			if (radius < 0.1f)
				radius = 0.1f;
			break;

		case GLUT_KEY_PAGE_UP: radius += 0.1f; break;

	}
	sphericalToCartesian();

}



void initGL() {

// OpenGL settings 
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	// Color of the light emmited by source.
	GLfloat dark[4] = {0.2, 0.2, 0.2, 1.0};
	GLfloat white[4] = {1, 1, 1, 1.0};
	GLfloat red[4] = {0.8, 0.2, 0, 1.0};
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);

	float black[4] = {0, 0, 0, 0};
	glLightModelfv( GL_LIGHT_MODEL_AMBIENT, black);


// init
	sphericalToCartesian();
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	prepareCilinder(2,1,1024);
}


int main(int argc, char **argv) {

// init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(320,320);
	glutCreateWindow("CG@DI-UM");
		
// callback registration
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);

// keyboard callback registration
	glutSpecialFunc(processKeys);

#ifndef __APPLE__	
// init GLEW
	glewInit();
#endif	


	initGL();

	glutMainLoop();
	
	return 1;
}

