#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>

void changeSize(int w, int h)
{
	// Prevent a divide by zero, when window is too short
	// (you can’t make a window with zero width).
	if (h == 0)
		h = 1;
	// compute window's aspect ratio
	float ratio = w * 1.0f / h;
	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load the identity matrix
	glLoadIdentity();
	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
	// Set the perspective
	gluPerspective(45.0f, ratio, 1.0f, 1000.0f);
	// return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
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

void renderScene1(void)
{
	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// set camera
	glLoadIdentity();
	gluLookAt(5.0f, 5.0f, 5.0f,
		-1.0f, -1.0f, -1.0f,
		0.0f, 1.0f, 0.0f);

	axis_system();

	// (1 - k)P' + kP
	// put drawing instructions here
	glutWireTeapot(1.5 + sin(2 * M_PI * glutGet(GLUT_ELAPSED_TIME)/4000));
	
	// End of frame
	glutSwapBuffers();
}

void renderScene2(void)
{
	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// (1 - k)P' + kP
	double p1x = 3.0f, p1y = 3.0f, p1z = 3.0f;
	double p2x = 10.0f, p2y = 10.0f, p2z = 10.0f;
	double px = 0.0f, py = 0.0f, pz = 0.0f;

	double sineFactor = 0.5 + 0.5 * sin(2 * M_PI * glutGet(GLUT_ELAPSED_TIME) / 4000);

	px = (1 - sineFactor) * p1x + sineFactor * p2x;
	py = (1 - sineFactor) * p1y + sineFactor * p2y;
	pz = (1 - sineFactor) * p1z + sineFactor * p2z;

	// set camera
	glLoadIdentity();
	gluLookAt(px, py, pz,
		-1.0f, -1.0f, -1.0f,
		0.0f, 1.0f, 0.0f);

	axis_system();

	// put drawing instructions here
	glutWireTeapot(1);

	// End of frame
	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	// put GLUT’s init here
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1300, 800);
	glutCreateWindow("CG@DI");

	// put callback registry here
	glutReshapeFunc(changeSize);
	glutIdleFunc(renderScene2);
	glutDisplayFunc(renderScene2);

	// some OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// enter GLUT’s main cycle
	glutMainLoop();

	return 1;
}