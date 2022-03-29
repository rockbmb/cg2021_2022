

#include<stdio.h>
#include<stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>

#include <IL/il.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

using namespace std;

// Modo de desenho.
int mode = GL_LINE;

float movementX = 0;
float movementZ = 0;
float movement_step = 1;

float camX = 00, camY = 30, camZ = 40;
int startX, startY, tracking = 0;

int alpha = 0, beta = 45, r = 50;

struct Point {
	float x;
	float y;
	float z;
};

GLuint buffers[1];
vector<float> gridVertices;

// Usados no carregamento da imagem.
unsigned int t, imageWidth, imageHeight;
unsigned char *imageData;
float maxHeight = 255.0f;
float maxAllowedHeight = 30.0f;

// glGenBuffer will place a buffer object name for VBO mode (no index) in this variable.
GLuint terrainVertices;

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

void initTerrain(){

}

void drawTerrain() {
    // colocar aqui o código de desnho do terreno usando VBOs com TRIANGLE_STRIPS
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	//glDrawArrays(GL_TRIANGLE_STRIP, imageWidth * 2 * 0, imageWidth * 2);
	//glDrawArrays(GL_TRIANGLE_STRIP, imageWidth * 2 * 1, imageWidth * 2);
	for (int i = 0; i < imageHeight - 1 ; i++) {
		glDrawArrays(GL_TRIANGLE_STRIP, imageWidth * 2 * i, imageWidth * 2);
	}
}

void renderScene(void) {

	float pos[4] = {-1.0, 1.0, 1.0, 0.0};

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(camX + movementX, camY, camZ + movementZ,
		      0.0 + movementX, 0.0, 0.0 + movementZ,
			  0.0f,1.0f,0.0f);

	glPolygonMode(GL_FRONT, GL_LINE);
	glPushMatrix();
	drawTerrain();
	glPopMatrix();

	// just so that it renders something before the terrain is built
	// to erase when the terrain is ready

// End of frame
	glutSwapBuffers();
}



void processKeys(unsigned char key, int xx, int yy) {

// put code to process regular keys in here
	switch(key) {
		case ',':
			mode = GL_FILL;
			break;
		case '.':
			mode = GL_POINT;
			break;
		case 'l':
			mode = GL_LINE;
			break;
		case 'w':
			movementZ += movement_step;
			break;
		case 's':
			movementZ -= movement_step;
			break;
		case 'a':
			movementX += movement_step;
			break;
		case 'd':
			movementX -= movement_step;
			break;
	}
}



void processMouseButtons(int button, int state, int xx, int yy) {
	
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


void processMouseMotion(int xx, int yy) {

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
		if (rAux < 3)
			rAux = 3;
	}
	camX = rAux * sin(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
	camZ = rAux * cos(alphaAux * 3.14 / 180.0) * cos(betaAux * 3.14 / 180.0);
	camY = rAux *								 sin(betaAux * 3.14 / 180.0);
}

float h(int i, int j) {
	return imageData[i * imageWidth + j]; //maxAllowedHeight * temp_;
}

void init() {
	ilGenImages(1,&t);
	ilBindImage(t);

	// terreno.jpg is the image containing our height map
	ilLoadImage((ILstring)"terreno.jpg");

	// convert the image to single channel per pixel
	// with values ranging between 0 and 255
	ilConvertImage(IL_LUMINANCE, IL_UNSIGNED_BYTE);

	// important: check tw and imageHeight values
	// both should be equal to 256
	// if not there was an error loading the image
	// most likely the image could not be found
	imageWidth = 8;//ilGetInteger(IL_IMAGE_WIDTH);
	imageHeight = 8;//ilGetInteger(IL_IMAGE_HEIGHT);

	/*if (imageWidth != 256 || imageHeight != 256) {
		puts("imageWidth ou ts não têm o valor 256.");
		return ;
	}*/

	// imageData is a LINEAR array with the pixel values
	imageData = ilGetData();

	// 	Load the height map "terreno.jpg"

	// Iterating over lines
	for (int i = 0; i < imageWidth - 1; i++) {

		// Iterating over columns
		for (int j = 0; j < imageHeight - 1; j++) {
			gridVertices.push_back(i);
			gridVertices.push_back(0);//gridVertices.push_back(h(j + 1, translateI));
			gridVertices.push_back(j);

			gridVertices.push_back(i + 1);
			gridVertices.push_back(0);//gridVertices.push_back(h(j, i));
			gridVertices.push_back(j);
		}
	}

// 	Build the vertex arrays

	glGenBuffers(1, buffers);
	glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, gridVertices.size(), gridVertices.data(), GL_STATIC_DRAW);

// 	OpenGL settings
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
}


int main(int argc, char **argv) {

// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(500,500);
	glutCreateWindow("CG@DI-UM");
		

// Required callback registry 
	glutDisplayFunc(renderScene);
	glutIdleFunc(renderScene);
	glutReshapeFunc(changeSize);

// Callback registration for keyboard processing
	glutKeyboardFunc(processKeys);
	glutMouseFunc(processMouseButtons);
	glutMotionFunc(processMouseMotion);

	glPolygonMode(GL_FRONT, mode);

	glEnableClientState(GL_VERTEX_ARRAY);

	glewInit();
	ilInit();
	init();

// enter GLUT's main cycle
	glutMainLoop();
	
	return 0;
}

