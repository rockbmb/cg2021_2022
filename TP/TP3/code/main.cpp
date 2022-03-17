#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if (h == 0)
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
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

struct Point {
	float x;
	float y;
	float z;
};

void vertexHelper(Point p) {
	glVertex3f(p.x, p.y, p.z);
}

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

		float col1 = pow(sin(angle_low), 2);
		float col2 = pow(cos(angle_low), 2);
		glColor3f(col1, col2, 0.0f);
		vertexHelper(top_center);
		vertexHelper(p2);
		vertexHelper(p1);


		glColor3f(col1, 0.0f, col2);
		vertexHelper(p1);
		vertexHelper(p2);
		vertexHelper(p3);

		glColor3f(col1, 0.0f, col2);
		vertexHelper(p1);
		vertexHelper(p3);
		vertexHelper(p4);


		glColor3f(col2, col1, 0.0f);
		vertexHelper(bottom_center);
		vertexHelper(p4);
		vertexHelper(p3);
	}
	glEnd();

}

void axis_system() {
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0, 0, 0);
	glVertex3f(15, 0, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(-15, 0, 0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 15, 0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, -15, 0);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 15);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, -15);

	glColor3f(1.0, 0.647, 0.0);
	glEnd();
}

float alfa = 0;
float alfa_step = 0.1;

// 1.5 radians.
float beta = 1.5;
float beta_step = 0.1;

int mode = GL_FRONT;

float radius = 15;
float radius_step = 0.5;

Point camera = {
	radius * cos(beta) * sin(alfa),
	radius * sin(beta),
	radius * cos(beta) * cos(alfa)
};

void renderScene(void) {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();

	camera.x = radius* cos(beta)* sin(alfa);
	camera.y = radius * sin(beta);
	camera.z = radius * cos(beta) * cos(alfa);
	gluLookAt(camera.x,camera.y,camera.z,
		      0.0,2.5,0.0,
			  0.0f,1.0f,0.0f);

	//axis_system();

	glPolygonMode(GL_FRONT, mode);
	drawCylinder(1,5,400);

	// End of frame
	glutSwapBuffers();
}


void processKeys(unsigned char c, int xx, int yy) {

// put code to process regular keys in here
	switch (c) {
	case 'q':
		alfa -= alfa_step;
		break;
	case 'e':
		alfa += alfa_step;
		break;
	case 'w':
		beta += beta_step;
		if (beta >= 1.5) {
			beta = 1.5;
		}
		break;
	case 's':
		beta -= beta_step;
		if (beta <= -1.5) {
			beta = -1.5;
		}
		break;
	case ',':
		mode = GL_FILL;
		break;
	case '.':
		mode = GL_POINT;
		break;
	case '-':
		mode = GL_LINE;
		break;
	default:
		break;
	}
	glutPostRedisplay();
}


void processSpecialKeys(int key, int xx, int yy) {

// put code to process special keys in here
	switch (key) {
	case GLUT_KEY_UP:
		radius -= radius_step;
		if (radius < 4) {
			radius = 4;
		}
		break;
	case GLUT_KEY_DOWN:
		radius += radius_step;
		break;
	default:
		break;
	}
	glutPostRedisplay();
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
	glEnable(GL_CULL_FACE);
	
// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
