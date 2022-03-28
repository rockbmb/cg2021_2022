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
float sides = 4;
float camX, camY, camZ;
int mode = GL_LINE;

enum DrawingMode {
	immediate,
	vbo_no_index,
	vbo_index
};

DrawingMode drawing_mode = vbo_no_index;

string dmToStr(DrawingMode dm) {
	string str = "none";
	switch (dm) {
	case immediate:
		str = "immediate";
		break;
	case vbo_no_index:
	    str = "vbo_no_index";
		break;
	case vbo_index:
		str = "vbo_index";
		break;
	default:
		break;
	}

	return str;
}

int timebase;
int timem;
float frames;
float fps;
char fps_counter[10];

// Variable to count cylinder vertices in VBO mode (no index).
GLuint cylinderVertexNum;
// Vertex vector for VBO mode (no index).
vector<float> vertexBuffer;
// glGenBuffer will place a buffer object name for VBO mode (no index) in this variable.
GLuint vertices;

// Variable to count cylinder vertices in VBO mode (indexed).
GLuint verticesIx;
// glGenBuffer will place a buffer object name for VBO mode (indexed) in this variable.
GLuint indices;
// Index vector for VBO mode (no index).
vector<unsigned int> indexBuffer;
// Vertex vector for VBO mode (indexed).
vector<float> vertexBufferIx;
// Index count in VBO mode (indexed).
unsigned int indexCount;

// Helper struct, used to represent points in 3D space.
struct Point {
	float x;
	float y;
	float z;
};

// Draw a point using glVertex3f.
void vertexHelper(Point p) {
	glVertex3f(p.x, p.y, p.z);
}

// Push a point into a vector reference.
void vertexVectorHelper(vector<float> *v, Point p) {
	v->push_back(p.x);
	v->push_back(p.y);
	v->push_back(p.z);
}

void writeCylinderVertices(float r, float height, int slices) {
	cylinderVertexNum = 3 * slices + 6 * slices + 3 * slices;
	vertexBuffer.clear();

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

		vertexVectorHelper(&vertexBuffer, top_center);
		vertexVectorHelper(&vertexBuffer, p2);
		vertexVectorHelper(&vertexBuffer, p1);

		vertexVectorHelper(&vertexBuffer, p1);
		vertexVectorHelper(&vertexBuffer, p2);
		vertexVectorHelper(&vertexBuffer, p3);

		vertexVectorHelper(&vertexBuffer, p1);
		vertexVectorHelper(&vertexBuffer, p3);
		vertexVectorHelper(&vertexBuffer, p4);

		vertexVectorHelper(&vertexBuffer, bottom_center);
		vertexVectorHelper(&vertexBuffer, p4);
		vertexVectorHelper(&vertexBuffer, p3);
	}
}

// Versão com índices.
void writeCylinderVerticesIndexed(float r, float height, int slices) {
	indexCount = 3 * slices + 6 * slices + 3 * slices;
	indexBuffer.clear();
	vertexBufferIx.clear();

	// put code to draw cylinder in here
	float angle = 2 * M_PI;
	float angle_low;
	float angle_high;
	Point top_center = { 0, height, 0};
	Point bottom_center = { 0, 0, 0 };

	// Which index of the vertex buffer we are on.
	int k = 0;

	vertexVectorHelper(&vertexBufferIx, top_center);
	k++;
	vertexVectorHelper(&vertexBufferIx, bottom_center);
	k++;

	Point p1, p2, p3, p4;
	/* Note-se que este ciclo, ao contrário do modo VBO sem índice que só corre
	slices vezes, este precisa correr slices + 1 vezes.
	Está-se a trabalhar com índices de vértices, e não com conjuntos de 3 vértices
	de cada vez.
	*/
	for (int i = 0; i <= slices; i++) {
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

		vertexVectorHelper(&vertexBufferIx, p1);
		vertexVectorHelper(&vertexBufferIx, p2);
		vertexVectorHelper(&vertexBufferIx, p3);
		vertexVectorHelper(&vertexBufferIx, p4);
		k += 4;

		indexBuffer.push_back(0);
		indexBuffer.push_back(k + 1);
		indexBuffer.push_back(k);

		indexBuffer.push_back(k);
		indexBuffer.push_back(k + 1);
		indexBuffer.push_back(k + 2);

		indexBuffer.push_back(k);
		indexBuffer.push_back(k + 2);
		indexBuffer.push_back(k + 3);

		indexBuffer.push_back(1);
		indexBuffer.push_back(k + 3);
		indexBuffer.push_back(k + 2);
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

void init_vbo_ix() {
	writeCylinderVerticesIndexed(raio, height, sides);

	glGenBuffers(1, &verticesIx);
	glGenBuffers(1, &indices);

	glBindBuffer(GL_ARRAY_BUFFER, verticesIx);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferIx.size() * sizeof(float), vertexBufferIx.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexBuffer.size(), indexBuffer.data(), GL_STATIC_DRAW);
}

void update_vbo() {
	// The vertex buffer is resized in this function.
	writeCylinderVertices(raio, height, sides);

	glBindBuffer(GL_ARRAY_BUFFER, vertices);
	glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(float), vertexBuffer.data(), GL_STATIC_DRAW);
}

void update_vbo_ix() {
	writeCylinderVerticesIndexed(raio, height, sides);

	glBindBuffer(GL_ARRAY_BUFFER, verticesIx);
	glBufferData(GL_ARRAY_BUFFER, vertexBufferIx.size() * sizeof(float), vertexBufferIx.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indexBuffer.size(), indexBuffer.data(), GL_STATIC_DRAW);
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
		fps = frames*1000.0/(timem - timebase);
		timebase = timem;
		frames = 0;
		snprintf(fps_counter, 10, "%5.2f", fps);
		glutSetWindowTitle((dmToStr(drawing_mode) + "; FPS: ").append(fps_counter).c_str());
	}

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	gluLookAt(camX, camY, camZ,
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);

	glColor3f(0.78f, 0.65f, 0.8f);

	switch (drawing_mode) {
	case immediate:
		drawCylinder(1, 2, sides);
		break;

	case vbo_no_index:
		glBindBuffer(GL_ARRAY_BUFFER, vertices);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glDrawArrays(GL_TRIANGLES, 0, cylinderVertexNum);
		break;

	case vbo_index:
		glBindBuffer(GL_ARRAY_BUFFER, verticesIx);
		glVertexPointer(3, GL_FLOAT, 0, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		break;
	default:
		break;
	}

	// End of frame
	glutSwapBuffers();
}


void processKeys(unsigned char c, int xx, int yy) {

// put code to process regular keys in here
	switch (c) {
		case '+':
			sides *= 2;
			switch(drawing_mode) {
				case vbo_no_index:
					update_vbo();
					break;

				case vbo_index:
					update_vbo_ix();
					break;
				default:
					break;
			}
			break;
		case '-':
			sides /= 2;
			switch(drawing_mode) {
				case vbo_no_index:
					update_vbo();
					break;

				case vbo_index:
					update_vbo_ix();
					break;
				default:
					break;
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
			switch (drawing_mode) {
				case immediate:
					drawing_mode = vbo_no_index;
					update_vbo();
					break;
				case vbo_no_index:
					drawing_mode = vbo_index;
					update_vbo_ix();
					break;
				case vbo_index:
					drawing_mode = immediate;
					break;
				default:
					break;
			}
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
	printf("\nPage Up and Page Down control the distance from the camera to the origin\n");
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
	init_vbo_ix();
// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
