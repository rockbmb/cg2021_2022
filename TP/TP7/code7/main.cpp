

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

struct Point {
	float x;
	float y;
	float z;
};

struct Vector {
	float x;
	float y;
	float z;
};

struct Color {
	float r;
	float g;
	float b;
};

/*
--------------------
Câmara, e movimento.
--------------------
*/


float movementX = 0;
float movementZ = 0;
float movement_step = 1;
float forward_displacement = 0;
float lateral_displacement = 0;

float eyeHeight = 1.5;
float camAlpha = 0;
float camAlphaStep = 0.1;

Vector camXVersor, camYVersor, camZVersor, camMinusZVersor;

Point cam = {0, eyeHeight, 0};
Point lookAt = {cam.x + sin(camAlpha), cam.y, cam.z + cos(camAlpha)};
Vector up = {0, 1, 0};

int startX, startY, tracking = 0;

int alpha = 0, beta = 45, r = 50;

Vector cross(Vector vec1, Vector vec2) {
	Vector res;

	res.x = vec1.y * vec2.z - vec1.z * vec2.y;
	res.y = vec1.z * vec2.x - vec1.x * vec2.z;
	res.z = vec1.x * vec2.y - vec1.y * vec2.x;

	return res;
}

/*
-------------------------
Fim de câmara e movimento
-------------------------
*/

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

/**
 * @brief Given two XZ coordinate pairs in [-halfimageWidth, halfimageWidth] x [-halfimageHeight, halfimageHeight],
 * produce their terrain height via bilinear interpolation.
 * 
 * @param x_ X coordinate of specified point, in [0, imageWidth].
 * @param z_ Z coordinate of specified point, in [0, imageHeight].
 * @return float Bilinearly interpolated height of given point, loaded from terreno.jpg.
 */
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

	/*
	The image vertices would normally be in [0, imageWidth] x [0, imageHeight],
	but here, before putting the vertex coordinates in the vector, they're moved
	by halfimageWidth/halfimageHeight to perform a translation on the terrain vertices.
	This is so that the terrain square is in
	[-halfimageWidth, halfimageWidth] x [-halfimageHeight, halfimageHeight] instead.
	*/
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
Note that the color the torus in drawn in will, for now, influence the color of
the terrain mesh.
*/
void drawTorus() {
	glColor3f(0.78, 0.68, 0.78);
	glPushMatrix();
	glTranslatef(0, height_float(0, 0), 0);
	glutSolidTorus(0.5, 2, 25, 25);
	glPopMatrix();
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
	gluLookAt(cam.x, cam.y, cam.z,
		      lookAt.x, lookAt.y, lookAt.z,
			  up.x, up.y, up.z);

	glPolygonMode(GL_FRONT, GL_LINE);

	drawTerrain();

	axis_system();

	drawTrees();

	float timer = glutGet(GLUT_ELAPSED_TIME);
	drawTeapots(blueTeapots, rc, 0, 0, 1, false, timer);
	drawTeapots(redTeapots, ri, 1, 0, 0, true, timer);

	drawTorus();

// End of frame
	glutSwapBuffers();
}

// Called at every keyboard stroke (if it causes movement).
void updateCamera() {
	Vector viewDir = {
		lookAt.x - cam.x,
		lookAt.y - cam.y,
		lookAt.z - cam.z
	};

	float norm  = sqrt(pow(viewDir.x, 2) + pow(viewDir.y, 2) + pow(viewDir.z, 2));

	camZVersor = {
		- viewDir.x / norm,
		- viewDir.y / norm,
		- viewDir.z / norm
	};

	Vector temp;

	temp = cross(camZVersor, up);
	camYVersor = cross(camZVersor, temp);
	camXVersor = cross(camYVersor, camZVersor);

	cam.y = eyeHeight + height_float(cam.x, cam.z);
	lookAt.x = cam.x + sin(camAlpha);
	lookAt.y = cam.y;
	lookAt.z = cam.z + cos(camAlpha);

	camMinusZVersor = {
		-camZVersor.x,
		-camZVersor.y,
		-camZVersor.z
	};

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
			cam.x += camMinusZVersor.x;
			lookAt.x += camMinusZVersor.x;

			cam.z += camMinusZVersor.z;
			lookAt.z += camMinusZVersor.z;
			break;
		case 's':
			cam.x -= camMinusZVersor.x;
			lookAt.x -= camMinusZVersor.x;

			cam.z -= camMinusZVersor.z;
			lookAt.z -= camMinusZVersor.z;
			break;
		case 'a':
			cam.x += camXVersor.x;
			lookAt.x += camXVersor.x;

			cam.z += camXVersor.z;
			lookAt.z += camXVersor.z;
			break;
		case 'd':
			cam.x -= camXVersor.x;
			lookAt.x -= camXVersor.x;

			cam.z -= camXVersor.z;
			lookAt.z -= camXVersor.z;
			break;
		case 'q':
			camAlpha += camAlphaStep;
			break;
		case 'e':
			camAlpha -= camAlphaStep;
			break;
	}

	/*
	Lower and upper camera position coordinate bounds.
	*/
	float maxX = halfImgWidth - 2;
	float minX = -maxX;
	float maxZ = halfImgHeight - 2;
	float minZ = -maxZ;
	cam.x = min(maxX, max(minX, cam.x));
	cam.z = min(maxZ, max(minZ, cam.z));

	updateCamera();
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

