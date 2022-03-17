#include <stdio.h>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

#include <vector>
#include <string>
using namespace std;

float alfa = 0.0f, beta = 0.0f, radius = 5.0f;
float raio = 1; 
float height = 2;
float sides = 128;
float camX, camY, camZ;
int mode = GL_LINE;
string drawing_mode = "vbo_no_index";

int timebase;
int timem;
float frames;
float fps;
char fps_counter[10];

GLuint cylinderVertexNum;
vector<float> vertexBuffer;
GLuint vertices;

struct Point {
	float x;
	float y;
	float z;
};

void vertexHelper(Point p) {
	glVertex3f(p.x, p.y, p.z);
}

void vertexVectorHelper(Point p) {
	vertexBuffer.push_back(p.x);
	vertexBuffer.push_back(p.y);
	vertexBuffer.push_back(p.z);
}

void writeCylinderVertices(float r, float height, int slices) {
	cylinderVertexNum = 3 * slices + 6 * slices + 3 * slices;
	vertexBuffer.resize(cylinderVertexNum*3, 0);
	vertexBuffer.resize(0);

	// put code to draw cylinder in here
	float angle = 2 * M_PI;
	float angle_low;
	float angle_high;
	Point top_center = { 0, height, 0};
	Point bottom_center = { 0, 0, 0 };

	Point p1, p2, p3, p4;
	for (int i = 0; i < slices; i++) {
		angle_low = (i * angle) / slices;
		angle_high = ((i + 1) * angle) / slices;
		p1.x = r * sin(angle_high);
		p1.z = r * cos(angle_high);
		p1.y = height;

		p2.x = r * sin(angle_low);
		p2.z = r * cos(angle_low);
		p2.y = height;

		p3.x = r * sin(angle_low);
		p3.z = r * cos(angle_low);
		p3.y = 0;

		p4.x = r * sin(angle_high);
		p4.z = r * cos(angle_high);
		p4.y = 0;

		float col1 = pow(sin(angle_low), 2);
		float col2 = pow(cos(angle_low), 2);
		vertexVectorHelper(top_center);
		vertexVectorHelper(p2);
		vertexVectorHelper(p1);

		vertexVectorHelper(p1);
		vertexVectorHelper(p2);
		vertexVectorHelper(p3);

		vertexVectorHelper(p1);
		vertexVectorHelper(p3);
		vertexVectorHelper(p4);

		vertexVectorHelper(bottom_center);
		vertexVectorHelper(p4);
		vertexVectorHelper(p3);
	}
}

/*-----------------------------------------------------------------------------------
	Draw Cylinder

		parameters: radius, height, sides

-----------------------------------------------------------------------------------*/

void drawCylinder(float radius, float height, int slices) {
// put code to draw cylinder in here
	float angle = 2 * M_PI;
	float angle_low;
	float angle_high;
	Point top_center = { 0, height, 0};
	Point bottom_center = { 0, 0, 0 };

	Point p1, p2, p3, p4;
	glBegin(GL_TRIANGLES);
	for (int i = 0; i < slices; i++) {
		angle_low = (i * angle) / slices;
		angle_high = ((i + 1) * angle) / slices;
		p1.x = radius * sin(angle_high);
		p1.z = radius * cos(angle_high);
		p1.y = height;

		p2.x = radius * sin(angle_low);
		p2.z = radius * cos(angle_low);
		p2.y = height;

		p3.x = radius * sin(angle_low);
		p3.z = radius * cos(angle_low);
		p3.y = 0;

		p4.x = radius * sin(angle_high);
		p4.z = radius * cos(angle_high);
		p4.y = 0;

		vertexHelper(top_center);
		vertexHelper(p2);
		vertexHelper(p1);

		vertexHelper(p1);
		vertexHelper(p2);
		vertexHelper(p3);

		vertexHelper(p1);
		vertexHelper(p3);
		vertexHelper(p4);

		vertexHelper(bottom_center);
		vertexHelper(p4);
		vertexHelper(p3);
	}
	glEnd();

}

void init_vbo() {
	// The vertex buffer is resized in this function.
	writeCylinderVertices(raio, height, sides);

	glGenBuffers(1, &vertices);

	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float), vertexBuffer.data(), GL_STATIC_DRAW);
}

void update_vbo() {
	// The vertex buffer is resized in this function.
	writeCylinderVertices(raio, height, sides);

	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float), vertexBuffer.data(), GL_STATIC_DRAW);
}

void spherical2Cartesian() {
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

void renderScene(void) {
	frames++;
	timem = glutGet(GLUT_ELAPSED_TIME);
	if (timem - timebase > 1000) {
		fps = frames*1000.0/(timem-timebase);
		timebase = timem;
		frames = 0;
		snprintf(fps_counter, 10, "%f", fps);
		glutSetWindowTitle(drawing_mode.c_str());
	}

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	gluLookAt(camX, camY, camZ,
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);

	glColor3f(0.78f, 0.65f, 0.8f);

	if (drawing_mode == "immediate") {
		drawCylinder(1, 2, sides);
	} else {
		glBindBuffer(GL_ARRAY_BUFFER, vertices);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexNum);
	}

	// End of frame
	glutSwapBuffers();
}


void processKeys(unsigned char c, int xx, int yy) {

// put code to process regular keys in here
	switch (c) {
		case '+':
			sides *= 2;
			if (drawing_mode == "vbo_no_index") {
				update_vbo();
			}
			break;
		case '-':
			sides /= 2;
			if (drawing_mode == "vbo_no_index") {
				update_vbo();
			}
			break;
		case ',':
			mode = GL_FILL;
			break;
		case '.':
			mode = GL_POINT;
			break;
		case 'l':
			mode = GL_LINE;
			break;
		case ' ':
			if (drawing_mode == "immediate") {
				update_vbo();
				drawing_mode = "vbo_no_index";
			} else {
				drawing_mode = "immediate";
			}
			break;
		default:
			break;
	}
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

	case GLUT_KEY_PAGE_DOWN: radius -= 0.1f;
		if (radius < 0.1f)
			radius = 0.1f;
		break;

	case GLUT_KEY_PAGE_UP: radius += 0.1f; break;
	}
	spherical2Cartesian();
	glutPostRedisplay();

}


void printInfo() {

	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version: %s\n", glGetString(GL_VERSION));

	printf("\nUse Arrows to move the camera up/down and left/right\n");
	printf("Page Up and Page Down control the distance from the camera to the origin");
}


int main(int argc, char **argv) {

// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
	timebase = glutGet(GLUT_ELAPSED_TIME);

// Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	
// Callback registration for keyboard processing
	glutKeyboardFunc(processKeys);
	glutSpecialFunc(processSpecialKeys);

	// init GLEW
#ifndef __APPLE__
	glewInit();
#endif

	glEnableClientState(GL_VERTEX_ARRAY);


//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT, mode);

	spherical2Cartesian();

	printInfo();

	init_vbo();
// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
