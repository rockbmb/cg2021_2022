

#include<stdio.h>
#include<stdlib.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <IL/il.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

using namespace std;

#include <vector>
#include <random>


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

struct Color {
	float r;
	float g;
	float b;
};


GLuint buffers[1];
vector<float> gridVertices;

// Usados no carregamento da imagem.
unsigned int t, imageWidth, imageHeight;
unsigned int halfImgWidth, halfImgHeight;

unsigned char *imageData;

/*
-------------------------------------------
Carregamento da iamgem e criação do seu VBO
-------------------------------------------
*/

float h(int i, int j) {
	return imageData[i * imageWidth + j];
}

float height_float(float x_, float z_) {
	float x = x_ + halfImgWidth;
	float z = z_ + halfImgHeight;

	int x1 = floor(x);
	int x2 = ceil(x);

	int z1 = floor(z);
	int z2 = ceil(z);

	float fx = x - x1;
	float fz = z - z1;

	float h_x1_z = h(x1, z1) * (1 - fz) + h(x2, z2) * fz;
	float h_x2_z = h(x2, z1) * (1 - fz) + h(x2, z2) * fz;

	float height_xz = h_x1_z * (1 - fx) + h_x2_z * fx;

	return height_xz;
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
	imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
	halfImgWidth = imageWidth / 2;
	imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
	halfImgHeight = imageHeight / 2;

	if (imageWidth != 256 || imageHeight != 256) {
		puts("imageWidth ou ts não têm o valor 256.");
		return ;
	}

	// imageData is a LINEAR array with the pixel values
	imageData = ilGetData();

	// 	Load the height map "terreno.jpg"

	// Iterating over lines
	for (int j = 0; j < imageHeight - 1; j++) {
		float vertZ = (float) j - (float) halfImgHeight;

		// Iterating over columns
		for (int i = 0; i < imageWidth; i++) {
			float vertX = (float) i - (float) halfImgWidth;
			gridVertices.push_back(vertX);
			gridVertices.push_back(h(i, j));
			gridVertices.push_back(vertZ);

			gridVertices.push_back(vertX);
			gridVertices.push_back(h(i, j + 1));
			gridVertices.push_back(vertZ + 1);
		}
	}

// 	Build the vertex arrays

	glGenBuffers(1, buffers);
	glBindBuffer(GL_ARRAY_BUFFER,buffers[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * gridVertices.size(), gridVertices.data(), GL_STATIC_DRAW);

// 	OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
}

/*
-------------------------------------------
Carregamento da iamgem e criação do seu VBO
-------------------------------------------
*/

/*
----------------------------------
Desenho de árvores e bules de chá.
----------------------------------
*/

float rc = 10;
float ri = 25;
float outer_radius = 50;

int blueTeapots = 8;
int redTeapots = 16;

int trees = 400;

vector<Point> treeCoords;
vector<float> treeSizes;
vector<float> leafColors;
vector<Color> trunkColors;

/*
Trees are in a square in the XZ subplane [-100, 100] x [-100, 100].
*/
int tree_range = 100;

void genTreeData(int num) {
	/*
	Para impedir que as árvores pareçam estar a flutuar.
	*/
	float tree_root = 0.1;

	float minXZ = -tree_range;
	float maxXZ = tree_range;
	std::random_device rd;
	std::default_random_engine eng(rd());
	std::uniform_real_distribution<> distr1(minXZ, maxXZ);

	float x;
	float z;
	Point p;

	for (int i = 0; i < num; i++) {
		bool done = false;
		while (!done) {
			x = distr1(eng);
			z = distr1(eng);
			if ((pow(x, 2.0) + pow(z, 2.0)) >= pow(outer_radius, 2)) {
				done = true;
			}
		}
		p.x = x;
		p.y = height_float(x, z) - tree_root;
		p.z = z;
		treeCoords.push_back(p);

	// Generation of a multiple of the default size, to make trees bigger.
	std::uniform_real_distribution<> distr2(1, 2);
	treeSizes.push_back(distr2(eng));

	std::uniform_real_distribution<> distr3(100, 255);
	leafColors.push_back(distr3(eng)/256.0f);

	Color c;
	c.r = distr3(eng)/256.0;
	c.g = distr3(eng)/256.0;
	c.b = 0.1;
	trunkColors.push_back(c);

	}
}

void drawTree(float height, float radius_trunk, float radius_leaves, float leaf_color, Color trunk_color) {
	glColor3f(0.1, leaf_color, 0.1);
	glPushMatrix();

	glPushMatrix();
	glTranslatef(0, height/3, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(radius_leaves, height, 25, 25);
	glPopMatrix();

	glColor3f(trunk_color.r, trunk_color.g, trunk_color.b);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(radius_trunk, height/2, 25, 25);

	glPopMatrix();
}

void drawTrees() {
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	for (int i = 0; i < treeCoords.size(); i++) {
		glPushMatrix();

		glTranslatef(
			treeCoords[i].x, 
			treeCoords[i].y,
			treeCoords[i].z);

		float h = 6;
		float leaves = 2;
		float trunk = 1;

		drawTree(treeSizes[i] * h, treeSizes[i] * trunk, treeSizes[i] * leaves, leafColors[i], trunkColors[i]);
		glPopMatrix();
	}
}

void drawTeapots(int num, float dist, float r, float g, float b, bool outer, float timer) {
	float angle;
	float height;
	for (int i = 0; i < num; i++) {
		glPushMatrix();

		/*
		Outer and inner teapots rotate in opposite directions, so their angles must
		have different signs - one decreases, while the other must increase.
		*/
		if (outer) {
			angle = (360 * i / num) - timer/10;
		} else {
			angle = (360 * i / num) + timer/10;
		}

		/*
		Necessary adding a small fraction to the teapots' height here,
		otherwise they'll be partially rendered under the ground.
		*/
		height = 1 + height_float(dist * sin(angle), dist * cos(angle));

		glRotatef(angle, 0, 1, 0);
		if (outer) {
			glTranslatef(0, height, dist);
		} else {
			glTranslatef(dist, height, 0);
		}
		glColor3f(r, g, b);
		glutSolidTeapot(1);
		glPopMatrix();
	}
	glutPostRedisplay();
}

/*
-----------------------
Fim de árvores e bules.
-----------------------
*/

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
	// Necessário deslocar o terreno, porque a imagem por defeito está em [0, 256] x [0, 256].
	glPushMatrix();

	// colocar aqui o código de desnho do terreno usando VBOs com TRIANGLE_STRIPS
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	for (int i = 0; i < imageHeight - 1 ; i++) {
		glDrawArrays(GL_TRIANGLE_STRIP, imageWidth * 2 * i, imageWidth * 2);
	}

	glPopMatrix();
}

void axis_system() {
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0, 0, 0);
	glVertex3f(15, 0, 0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 15, 0);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 15);

	glColor3f(1.0, 0.647, 0.0);
	glEnd();
}

void renderScene(void) {

	float pos[4] = {-1.0, 1.0, 1.0, 0.0};

	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(movementX + camX, camY, camZ + movementZ,
		      movementX, 0, movementZ,
			  0.0f,1.0f,0.0f);

	glPolygonMode(GL_FRONT, GL_LINE);

	drawTerrain();

	axis_system();

	drawTrees();

	float timer = glutGet(GLUT_ELAPSED_TIME);
	drawTeapots(blueTeapots, rc, 0, 0, 1, false, timer);
	drawTeapots(redTeapots, ri, 1, 0, 0, true, timer);

	glColor3f(0.78, 0.68, 0.78);
	glutSolidTorus(0.5, 2, 25, 25);

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

	genTreeData(trees);

	// enter GLUT's main cycle
	glutMainLoop();
	
	return 0;
}

