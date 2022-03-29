#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;

#include <vector>
#include <random>

/*
Useful structs
*/
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

// Para movimento da câmara
float alfaDelta = 0.0f;
float betaDelta = 0.0f;
int xOrigin = -1;
int yOrigin = 0;

float alfa = 0.0f, beta = 0.5f, radius = 100.0f;
float camX, camY, camZ;

float rc = 10;
float ri = 25;
float outer_radius = 50;

int blueTeapots = 8;
int redTeapots = 16;

int trees = 200;

float lookAtX = 0.0f, lookAtY = 0.0f, lookAtZ = 0.0f;

// How many times is the normalized viewing direction vector added to both
// the lookAt point, and the camera location.
float mov_cam_z = 0;
float mov_cam_z_step = 5;

float mov_cam_x = 0;
float mov_cam_x_step = 5;

float viewDirX, viewDirY, viewDirZ;
Point upVec = {0, 1, 0};

Point camZVersor;
Point camXVersor;
Point camYVersor;

Point cross(Point vec1, Point vec2) {
	Point res;

	res.x = vec1.y * vec2.z - vec1.z * vec2.y;
	res.y = vec1.z * vec2.x - vec1.x * vec2.z;
	res.z = vec1.x * vec2.y - vec1.y * vec2.x;

	return res;
}

void spherical2Cartesian() {
	camX = radius * cos(beta) * sin(alfa);
	camY = radius * sin(beta);
	camZ = radius * cos(beta) * cos(alfa);


	float tempX = lookAtX - camX;
	float tempY = lookAtY - camY;
	float tempZ = lookAtZ - camZ;
	float norm  = sqrt(pow(tempX, 2) + pow(tempY, 2) + pow(tempZ, 2));

	viewDirX = tempX / norm;
	viewDirY = tempY / norm;
	viewDirZ = tempZ / norm;

	Point temp;

	camZVersor = {viewDirX, viewDirY, viewDirZ};
	temp = cross(camZVersor, upVec);
	camYVersor = cross(camZVersor, temp);
	camXVersor = cross(camYVersor, camZVersor);
}


void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if(h == 0)
		h = 1;

	// compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void drawTeapots(int num, float dist, float r, float g, float b, bool outer, float timer) {
	float angle;
	for (int i = 0; i < num; i++) {
		glPushMatrix();
		if (outer) {
			angle = (360 * i / num) - timer/10;
		} else {
			angle = (360 * i / num) + timer/10;
		}
		glRotatef(angle, 0, 1, 0);
		if (outer) {
			glTranslatef(0, 0, dist);
		} else {
			glTranslatef(dist, 0, 0);
		}
		glColor3f(r, g, b);
		glutSolidTeapot(1);
		glPopMatrix();
	}
	glutPostRedisplay();
}

vector<Point> treeCoords;
vector<float> treeSizes;
vector<float> leafColors;
vector<Color> trunkColors;

void genTreeData(int num) {
	float minXZ = -100;
	float maxXZ = 100;
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
		p.y = 0;
		p.z = z;
		treeCoords.push_back(p);

	// Generation of a multiple of the default size, to make trees bigger.
	std::uniform_real_distribution<> distr2(1, 4);
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
	glTranslatef(0, height/3, 0);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(radius_leaves, height, 25, 25);
	glPopMatrix();
	glColor3f(trunk_color.r, trunk_color.g, trunk_color.b);
	glRotatef(-90, 1, 0, 0);
	glutSolidCone(radius_trunk, height/2, 25, 25);
}

void drawTrees() {
	for (int i = 0; i < treeCoords.size(); i++) {
		glPushMatrix();
		glTranslatef(treeCoords[i].x, 0, treeCoords[i].z);

		float h = 6;
		float leaves = 2;
		float trunk = 1;

		drawTree(treeSizes[i] * h, treeSizes[i] * trunk, treeSizes[i] * leaves, leafColors[i], trunkColors[i]);
		glPopMatrix();
	}
}

void renderScene(void) {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	float movementX = mov_cam_z * camZVersor.x + mov_cam_x * camXVersor.x;
	float movementY = mov_cam_z * camZVersor.y + mov_cam_x * camXVersor.y;
	float movementZ = mov_cam_z * camZVersor.z + mov_cam_x * camXVersor.z;
	gluLookAt(camX + movementX, camY + movementY, camZ + movementZ,
		lookAtX + movementX, lookAtY + movementY, lookAtZ + movementZ,
		upVec.x, upVec.y, upVec.z);

	glColor3f(0.2f, 0.8f, 0.2f);
	glBegin(GL_TRIANGLES);
		glVertex3f(100.0f, 0, -100.0f);
		glVertex3f(-100.0f, 0, -100.0f);
		glVertex3f(-100.0f, 0, 100.0f);

		glVertex3f(100.0f, 0, -100.0f);
		glVertex3f(-100.0f, 0, 100.0f);
		glVertex3f(100.0f, 0, 100.0f);
	glEnd();

	// End of frame
	
	// put code to draw scene in here
	glPushMatrix();
	glTranslatef(0, 0.5, 0);
	float timer = glutGet(GLUT_ELAPSED_TIME);
	drawTeapots(blueTeapots, rc, 0, 0, 1, false, timer);
	drawTeapots(redTeapots, ri, 1, 0, 0, true, timer);
	glPopMatrix();

	glColor3f(0.78, 0.68, 0.78);
	glutSolidTorus(0.5, 2, 25, 25);

	drawTrees();

	glutSwapBuffers();
}

void processKeys(unsigned char c, int xx, int yy) {

// put code to process regular keys in here
	switch(c) {
		case 'w':
			mov_cam_z += mov_cam_z_step;
			break;

		case 's':
			mov_cam_z -= mov_cam_z_step;
			break;

		case 'a':
			mov_cam_x -= mov_cam_x_step;
			break;
		case 'd':
			mov_cam_x += mov_cam_x_step;
			break;
	}
	spherical2Cartesian();
	glutPostRedisplay();

}

void processSpecialKeys(int key, int xx, int yy) {
	switch (key) {

	case GLUT_KEY_RIGHT:
		alfa -= 0.1; break;

	case GLUT_KEY_LEFT:
		alfa += 0.1; break;

	case GLUT_KEY_UP:
		beta += 0.1f;
		if (beta > 1.5f)
			beta = 1.5f;
		break;
	case GLUT_KEY_DOWN:
		beta -= 0.1f;
		if (beta < -1.5f)
			beta = -1.5f;
		break;

	case GLUT_KEY_PAGE_DOWN: radius -= 1.0f;
		if (radius < 1.0f)
			radius = 1.0f;
		break;

	case GLUT_KEY_PAGE_UP: radius += 1.0f; break;
	}
	spherical2Cartesian();
	glutPostRedisplay();

}

void mouseMove(int x, int y) {

	// this will only be true when the left button is down
	if (xOrigin >= 0) {

		// update deltaAngle
		alfaDelta = (x - xOrigin) * 10.f;
		betaDelta = (y - yOrigin) * 10.f;

		// update camera's direction
		beta += betaDelta;
		alfa += alfaDelta;
	}

	spherical2Cartesian();
	glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y) {

	// only start motion if the left button is pressed
	if (button == GLUT_LEFT_BUTTON) {

		// when the button is released
		if (state == GLUT_UP) {
			//angle += deltaAngle;
			xOrigin = -1;
			yOrigin = -1;
		}
		// state = GLUT_DOWN
		else  {
			xOrigin = x;
			yOrigin = y;
		}
	}
}

void printInfo() {

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));

	printf("\nUse Arrows to move the camera up/down and left/right\n");
	printf("Home and End control the distance from the camera to the origin");
}


int main(int argc, char **argv) {

// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
		
// Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	
// Callback registration for keyboard processing
	glutKeyboardFunc(processKeys);
	glutSpecialFunc(processSpecialKeys);

//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);

	spherical2Cartesian();

	printInfo();

	genTreeData(trees);

// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
