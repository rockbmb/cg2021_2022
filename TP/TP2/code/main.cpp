#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <math.h>

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

float rot_step = 5;
float v_rot_angle = 0;

float scale_step = 0.1;
float scale_factor = 1;

float x_translate = 0;
float z_translate = 0;
float trans_step = 0.1;

int mode = GL_FRONT;

void renderScene(void) {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	gluLookAt(5.0,5.0,5.0, 
		      0.0,0.0,0.0,
			  0.0f,1.0f,0.0f);

// put the geometric transformations here
	axis_system();

	glTranslatef(x_translate, 0, z_translate);
	glRotatef(v_rot_angle, 0.0f, 1.0f, 0.0f);
	glScalef(1, scale_factor, 1);
	glPolygonMode(GL_FRONT,  mode);


// put drawing instructions here
	glBegin(GL_TRIANGLES);
		glColor3f(1.0f, 1.0f, 1.0f);

		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(-1.0f, 0.0f, 1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);

		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(1.0f, 0.0f, -1.0f);

		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(-1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 2.0f, 0.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(1.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, -1.0f);
		glVertex3f(0.0f, 2.0f, 0.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(1.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(0.0f, 2.0f, 0.0f);

		glColor3f(1.0f, 0.0f, 1.0f);
		glVertex3f(-1.0f, 0.0f, -1.0f);
		glVertex3f(-1.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 2.0f, 0.0f);
	glEnd();

	// End of frame
	glutSwapBuffers();
}

// write function to process keyboard events
void handle_keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 'q':
		v_rot_angle += rot_step;
		break;
	case 'e':
		v_rot_angle -= rot_step;
		break;
	case 'w':
		scale_factor += scale_step;
		break;
	case 's':
		scale_factor -= scale_step;
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

// write function to process special keyboard events
void handle_arrows(int key_code, int x, int y) {
	switch (key_code) {
	case GLUT_KEY_UP:
		x_translate -= trans_step;
		break;
	case GLUT_KEY_DOWN:
		x_translate += trans_step;
		break;
	case GLUT_KEY_LEFT:
		z_translate += trans_step;
		break;
	case GLUT_KEY_RIGHT:
		z_translate -= trans_step;
		break;
	case GLUT_KEY_HOME:
		x_translate = 0;
		z_translate = 0;
		v_rot_angle = 0;
		scale_factor = 1;
		mode = GL_FILL;
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
	glutKeyboardFunc(handle_keyboard);
	glutSpecialFunc(handle_arrows);

	
// put here the registration of the keyboard callbacks



//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
