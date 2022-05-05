#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glut.h>

#endif

#define _USE_MATH_DEFINES

#include <math.h>

#define DEFAULT_GLOBAL_ANGLE_STEP 16
#define DEFAULT_GLOBAL_TRANSLATE_STEP 1
#define DEFAULT_GLOBAL_SCALE_STEP 1

/*rotation*/
float globalAngleStep = DEFAULT_GLOBAL_ANGLE_STEP;
float globalAngle = 0;
float globalRotateX = 0;
float globalRotateY = 0;
float globalRotateZ = 0;

/*translation*/
float globalTranslateStep = DEFAULT_GLOBAL_TRANSLATE_STEP;
float globalTranslateX = 0;
float globalTranslateY = 0;
float globalTranslateZ = 0;

/*scaling*/
float globalScaleStep = DEFAULT_GLOBAL_SCALE_STEP;
float globalScaleX = 1;
float globalScaleY = 1;
float globalScaleZ = 1;

/*coloring*/
float globalBackgroundColorStep = 1;
float globalBackgroundColorR = 0;
float globalBackgroundColorG = 0;
float globalBackgroundColorB = 0;

/*camera*/
#define DEFAULT_GLOBAL_EYE_STEP 1.0f
#define DEFAULT_GLOBAL_EYE_X 5.0f
#define DEFAULT_GLOBAL_EYE_Y (-2.0f)
#define DEFAULT_GLOBAL_EYE_Z (3.0f)

#define DEFAULT_GLOBAL_CENTER_STEP 1.0f
#define DEFAULT_GLOBAL_CENTER_X 0.0f
#define DEFAULT_GLOBAL_CENTER_Y 0.0f
#define DEFAULT_GLOBAL_CENTER_Z 0.0f

#define DEFAULT_GLOBAL_UP_STEP 1.0f
#define DEFAULT_GLOBAL_UP_X 0.0f
#define DEFAULT_GLOBAL_UP_Y 1.0f
#define DEFAULT_GLOBAL_UP_Z 0.0f

float globalEyeStep = DEFAULT_GLOBAL_EYE_STEP;
float globalEyeX = DEFAULT_GLOBAL_EYE_X;
float globalEyeY = DEFAULT_GLOBAL_EYE_Y;
float globalEyeZ = DEFAULT_GLOBAL_EYE_Z;

float globalCenterStep = DEFAULT_GLOBAL_CENTER_STEP;
float globalCenterX = DEFAULT_GLOBAL_CENTER_X;
float globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
float globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;

float globalUpStep = DEFAULT_GLOBAL_UP_STEP;
float globalUpX = DEFAULT_GLOBAL_UP_X;
float globalUpY = DEFAULT_GLOBAL_UP_Y;
float globalUpZ = DEFAULT_GLOBAL_UP_Z;

#define DEFAULT_GLOBAL_STACK 10
#define DEFAULT_GLOBAL_SLICE 10

int globalStackSliceStep = 1;
int globalStack = DEFAULT_GLOBAL_STACK;
int globalSlice = DEFAULT_GLOBAL_SLICE;

static float globalFOV = 60.0f;
static float globalNear = 1.0f;
static float globalFar = 1000.0f;

#include <stdio.h>
#include <tinyxml2.h>

void render3d(const char *path) {
    FILE *fp;
    float p[3];
    fp = fopen(path, "r");

    glBegin(GL_TRIANGLES);

    while (fread(p, sizeof(float), 3, fp))
        glVertex3f(p[0], p[1], p[2]);

    glEnd();
    fclose(fp);
}

size_t readModelToBuffer(const char *path, float *p, unsigned int n) {
    FILE *fp = fopen(path, "r");
    size_t r = fread(p, sizeof(float), n, fp);
    fclose(fp);
    return r;
}

void renderTriangleVertexSeq(float *p, int n) {
    if (!n % 3) {
        fprintf(stderr, "Number of coordinates is not divisible by 3");
        exit(-1);
    }
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < n; i += 3)
        glVertex3f(p[i], p[i + 1], p[i + 2]);
    glEnd();
}

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
    gluPerspective(globalFOV, ratio, globalNear, globalFar);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

void cylinderVertex(float radius, float height, float theta) {
    glVertex3d(radius * sin(theta), height, radius * cos(theta));
}

void drawCylinder(float r, float height, int slices) {
    /*coloring*/
    float c = 1;
    float color_step = (float) 1.0 / (float) (slices + 4);

    float s = 2.0f * (float) M_PI / (float) slices;
    float theta = -M_PI;

    glBegin(GL_TRIANGLES);
    for (int m = 1; m <= slices; m++) {
        float i = (float) m;

        //base
        glColor3f(0, 0, c);
        glVertex3d(0, 0, 0); //O
        cylinderVertex(r, 0, theta + s * i); //P2
        cylinderVertex(r, 0, theta + s * (i - 1)); //P1

        //top
        glColor3f(0, c, 0);
        glVertex3d(0, height, 0); //O'
        cylinderVertex(r, height, theta + s * (i - 1)); //P1'
        cylinderVertex(r, height, theta + s * i); //P2'

        // midsection
        glColor3f(c, 0, 0);
        cylinderVertex(r, 0, theta + s * (i - 1)); //P1
        cylinderVertex(r, 0, theta + s * i); // P2
        cylinderVertex(r, height, theta + s * (i - 1)); // P1'

        glColor3f(c, 0, c);
        cylinderVertex(r, height, theta + s * i); // P2'
        cylinderVertex(r, height, theta + s * (i - 1)); // P1'
        cylinderVertex(r, 0, theta + s * i); //P2

        c -= color_step;
    }
    glEnd();
}

void coneVertex(float r, float height, float theta, float h) {
    glVertex3d(r * h * cos(theta), 2 * (height + h), r * h * sin(theta));
}

void drawCone(float r, float height, unsigned int slices, unsigned int stacks) {
    /*coloring*/
    float c1 = 1;
    float c2 = 1;
    float color_step = (float) 1.0 / (float) (slices + 4);

    // https://www.math3d.org/5gLCN9yBz

    float s = 2.0f * (float) M_PI / (float) slices;
    float t = height / (float) stacks;
    float theta = -M_PI;
    float h = -height;

    glBegin(GL_TRIANGLES);
    for (int m = 1; m <= slices; m++) {
        for (int n = 1; n <= stacks; n++) {
            float i = (float) m;
            float j = (float) n;

            //base
            glColor3f(c1, c2, c2);
            glVertex3d(0, 0, 0); //O
            coneVertex(r, height, theta + s * (i - 1), h); //P1
            coneVertex(r, height, theta + s * i, h); //P2

            glColor3f(c1, 0, c2);
            coneVertex(r, height, theta + s * i, h + t * (j - 1)); // P2
            coneVertex(r, height, theta + s * (i - 1), h + t * j); // P1'
            coneVertex(r, height, theta + s * i, h + t * j); //P2'

            glColor3f(c1, c2, 0);
            coneVertex(r, height, theta + s * i, h + t * (j - 1)); // P2
            coneVertex(r, height, theta + s * (i - 1), h + t * (j - 1)); // P1
            coneVertex(r, height, theta + s * (i - 1), h + t * j); // P1'

            c2 -= color_step;
        }
        c1 -= color_step;
        c2 = 1;
    }
    glEnd();
}

void sphereVertex(float r, float theta, float phi) {
    glVertex3d(r * sin(theta) * cos(phi), r * sin(phi), r * cos(theta) * cos(phi));
}

void drawSphere(float r, unsigned int slices, unsigned int stacks) {
    /*coloring*/
    float c1 = 1;
    float c2 = 1;
    float color_step = (float) 1.0 / (float) (slices + 4);

    // https://www.math3d.org/EumEEZBKe
    // https://www.math3d.org/zE4n6xayX

    float s = 2.0f * (float) M_PI / (float) slices;
    float t = M_PI / (float) stacks;
    float theta = -M_PI;
    float phi = -M_PI / 2.0f;
    glBegin(GL_TRIANGLES);
    for (int m = 1; m <= slices; m++) {
        for (int n = 1; n <= stacks; n++) {
            float i = (float) m;
            float j = (float) n;

            glColor3f(c1, 0, c2);
            sphereVertex(r, theta + s * (i - 1), phi + t * j); // P1'
            sphereVertex(r, theta + s * i, phi + t * (j - 1)); // P2
            sphereVertex(r, theta + s * i, phi + t * j); //P2'

            glColor3f(c1, c2, 0);
            sphereVertex(r, theta + s * (i - 1), phi + t * (j - 1)); // P1
            sphereVertex(r, theta + s * i, phi + t * (j - 1)); // P2
            sphereVertex(r, theta + s * (i - 1), phi + t * j); // P1'

            c2 -= color_step;
        }
        c1 -= color_step;
        c2 = 1;
    }
    glEnd();
}

void drawPlane(float length, unsigned int divisions) {
    /*coloring*/
    float c1 = 1;
    float c2 = 1;
    float color_step = (float) 1.0 / (float) (divisions + 4);

    float o = -length / 2.0f;
    float d = length / (float) divisions;
    glBegin(GL_TRIANGLES);
    for (int m = 1; m <= divisions; m++) {
        for (int n = 1; n <= divisions; n++) {
            float i = (float) m;
            float j = (float) n;
            glColor3f(c1, c2, 0);
            glVertex3d(o + d * (i - 1), 0, o + d * (j - 1)); //P1
            glVertex3d(o + d * (i - 1), 0, o + d * j); //P1'z
            glVertex3d(o + d * i, 0, o + d * (j - 1)); //P1'x

            glColor3f(c1, 0, c2);
            glVertex3d(o + d * i, 0, o + d * (j - 1)); //P1'x
            glVertex3d(o + d * (i - 1), 0, o + d * j); //P1'z
            glVertex3d(o + d * i, 0, o + d * j); //P2

            /*Cull face*/
            glColor3f(c1, c2, 0);
            glVertex3d(o + d * (i - 1), 0, o + d * j); //P1'z
            glVertex3d(o + d * (i - 1), 0, o + d * (j - 1)); //P1
            glVertex3d(o + d * i, 0, o + d * (j - 1)); //P1'x

            glColor3f(c1, 0, c2);
            glVertex3d(o + d * (i - 1), 0, o + d * j); //P1'z
            glVertex3d(o + d * i, 0, o + d * (j - 1)); //P1'x
            glVertex3d(o + d * i, 0, o + d * j); //P2
        }
        c1 -= color_step;
        c2 = 1;
    }
    glEnd();
}

void drawAxes() {
    glBegin(GL_LINES);
    /*X-axis in red*/
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f(100.0f, 0.0f, 0.0f);

    /*Y-Axis in Green*/
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);

    /*Z-Axis in Blue*/
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();
}

void drawCube(float length, unsigned int divisions) {
    /*coloring*/
    float c1 = 1;
    float c2 = 1;
    float color_step = (float) 1.0 / (float) (divisions + 4);

    float o = -length / 2.0f;
    float d = length / (float) divisions;
    glBegin(GL_TRIANGLES);
    for (int m = 1; m <= divisions; m++) {
        for (int n = 1; n <= divisions; n++) {
            float i = (float) m;
            float j = (float) n;

            // top
            glColor3f(c1, c2, 0);
            glVertex3d(o + d * (i - 1), -o, o + d * (j - 1)); //P1
            glVertex3d(o + d * (i - 1), -o, o + d * j); //P1'z
            glVertex3d(o + d * i, -o, o + d * (j - 1)); //P1'x

            glColor3f(c1, 0, c2);
            glVertex3d(o + d * i, -o, o + d * (j - 1)); //P1'x
            glVertex3d(o + d * (i - 1), -o, o + d * j); //P1'z
            glVertex3d(o + d * i, -o, o + d * j); //P2

            // bottom
            glColor3f(c1, c2, 0);
            glVertex3d(o + d * (i - 1), o, o + d * j); //P1'z
            glVertex3d(o + d * (i - 1), o, o + d * (j - 1)); //P1
            glVertex3d(o + d * i, o, o + d * (j - 1)); //P1'x

            glColor3f(c1, 0, c2);
            glVertex3d(o + d * (i - 1), o, o + d * j); //P1'z
            glVertex3d(o + d * i, o, o + d * (j - 1)); //P1'x
            glVertex3d(o + d * i, o, o + d * j); //P2

            // left
            glColor3f(c1, c2, 0);
            glVertex3d(o, o + d * (i - 1), o + d * (j - 1)); //P1
            glVertex3d(o, o + d * (i - 1), o + d * j); //P1'z
            glVertex3d(o, o + d * i, o + d * (j - 1)); //P1'x

            glColor3f(c1, 0, c2);
            glVertex3d(o, o + d * i, o + d * (j - 1)); //P1'x
            glVertex3d(o, o + d * (i - 1), o + d * j); //P1'z
            glVertex3d(o, o + d * i, o + d * j); //P2

            // right
            glColor3f(c1, c2, 0);
            glVertex3d(-o, o + d * (i - 1), o + d * j); //P1'z
            glVertex3d(-o, o + d * (i - 1), o + d * (j - 1)); //P1
            glVertex3d(-o, o + d * i, o + d * (j - 1)); //P1'x

            glColor3f(c1, 0, c2);
            glVertex3d(-o, o + d * (i - 1), o + d * j); //P1'z
            glVertex3d(-o, o + d * i, o + d * (j - 1)); //P1'x
            glVertex3d(-o, o + d * i, o + d * j); //P2

            // front
            glColor3f(c1, c2, 0);
            glVertex3d(o + d * (i - 1), o + d * (j - 1), o); //P1
            glVertex3d(o + d * (i - 1), o + d * j, o); //P1'z
            glVertex3d(o + d * i, o + d * (j - 1), o); //P1'x

            glColor3f(c1, 0, c2);
            glVertex3d(o + d * i, o + d * (j - 1), o); //P1'x
            glVertex3d(o + d * (i - 1), o + d * j, o); //P1'z
            glVertex3d(o + d * i, o + d * j, o); //P2

            // back
            glColor3f(c1, c2, 0);
            glVertex3d(o + d * (i - 1), o + d * j, -o); //P1'z
            glVertex3d(o + d * (i - 1), o + d * (j - 1), -o); //P1
            glVertex3d(o + d * i, o + d * (j - 1), -o); //P1'x

            glColor3f(c1, 0, c2);
            glVertex3d(o + d * (i - 1), o + d * j, -o); //P1'z
            glVertex3d(o + d * i, o + d * (j - 1), -o); //P1'x
            glVertex3d(o + d * i, o + d * j, -o); //P2
        }
        c1 -= color_step;
        c2 = 1;
    }
    glEnd();
}


void renderScene(void) {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity(); /*helps in avoiding rounding mistakes, discards old matrix, read more*/
    gluLookAt(globalEyeX, globalEyeY, globalEyeZ,
              globalCenterX, globalCenterY, globalCenterZ,
              globalUpX, globalUpY, globalUpZ);
//    drawAxes();
    // put the geometric transformations here
    glRotatef(globalAngle, globalRotateX, globalRotateY, globalRotateZ);
    glTranslatef(globalTranslateX, globalTranslateY, globalTranslateZ);
    glScalef(globalScaleX, globalScaleY, globalScaleZ);

    // put drawing instructions here
//    drawSphere(2,10,10);
//    drawPlane(2222, 2222);
//    drawCylinder(2,10,10);
//    drawCube(10, 10);

//    glBegin(GL_TRIANGLES);
//    for (size_t i = 0;i+2 <= r; i+=3)
//        glVertex3f(buf[i],buf[i+1],buf[i+2]);
//    glEnd();
//    render3d("cone.3d");
    drawCone(1, 2, 4, 3);
//    puts("here");
//    drawCone(1, 3, 10, 10);
// End of frame

    glutSwapBuffers();
}

void keyboardFunc(unsigned char key, int xmouse, int ymouse) {
/*
    Rotation:
        x y z       x↓ y↓ z↓
        X Y Z       x↑ y↑ z↑
    angle:
        , .     θ-=s    θ+=s
        < >     s↓     s↑
    translation:
        q w e       y↓ z↑ y↑
        a s d       x↓ z↓ x↑
    scaling:
        u i o       y↓ z↑ y↑
        j k l       x↓ z↓ x↑
    background color: r g b R G B
    camera:
        1 2 3 ⎫       ! @ # ⎫        EyeX     EyeY     EyeZ
        4 5 6 ⎬↓      $ % ^ ⎬↑       CenterX  CenterY  CenterZ
        7 8 9 ⎭       & * ( ⎭        UpX      UpY      UpZ
 */
    switch (key) {
        case 'x':
            globalRotateX -= 1;
            break;

        case 'X':
            globalRotateX += 1;
            break;

        case 'y':
            globalRotateY -= 1;
            break;

        case 'Y':
            globalRotateY += 1;
            break;

        case 'z':
            globalRotateZ -= 1;
            break;

        case 'Z':
            globalRotateZ += 1;
            break;

        case ',':
            globalAngle -= globalAngleStep;
            break;

        case '.':
            globalAngle += globalAngleStep;
            break;

        case '<':
            globalAngleStep /= 2;
            break;

        case '>':
            globalAngleStep *= 2;
            break;

            /*translation*/

            /*z-axis*/
        case 'w':
            globalTranslateZ += globalTranslateStep;
            break;
        case 's':
            globalTranslateZ -= globalTranslateStep;
            break;

            /*x-axis*/
        case 'a':
            globalTranslateX -= globalTranslateStep;
            break;
        case 'd':
            globalTranslateX += globalTranslateStep;
            break;

            /*y-axis*/
        case 'q':
            globalTranslateY -= globalTranslateStep;
            break;
        case 'e':
            globalTranslateY += globalTranslateStep;
            break;
            /*end of translation*/

            /*scaling*/

            /*z-axis*/
        case 'i':
            globalScaleZ += globalScaleStep;
            break;
        case 'k':
            globalScaleZ -= globalScaleStep;
            break;

            /*x-axis*/
        case 'j':
            globalScaleX -= globalScaleStep;
            break;
        case 'l':
            globalScaleX += globalScaleStep;
            break;

            /*y-axis*/
        case 'u':
            globalScaleY -= globalScaleStep;
            break;
        case 'o':
            globalScaleY += globalScaleStep;
            break;
            /*end of scaling*/

            /*coloring*/
        case 'r':
            globalBackgroundColorR -= globalBackgroundColorStep;
            break;
        case 'R':
            globalBackgroundColorR += globalBackgroundColorStep;
            break;
        case 'g':
            globalBackgroundColorG -= globalBackgroundColorStep;
            break;
        case 'G':
            globalBackgroundColorG += globalBackgroundColorStep;
            break;
        case 'b':
            globalBackgroundColorB -= globalBackgroundColorStep;
            break;
        case 'B':
            globalBackgroundColorB += globalBackgroundColorStep;
            break;

            /*camera*/
        case '1':
            globalEyeX -= globalEyeStep;
            break;
        case '!':
            globalEyeX += globalEyeStep;
            break;
        case '2':
            globalEyeY -= globalEyeStep;
            break;
        case '@':
            globalEyeY += globalEyeStep;
            break;
        case '3':
            globalEyeZ -= globalEyeStep;
            break;
        case '#':
            globalEyeZ += globalEyeStep;
            break;

        case '4':
            globalCenterX -= globalCenterStep;
            break;
        case '$':
            globalCenterX += globalCenterStep;
            break;
        case '5':
            globalCenterY -= globalCenterStep;
            break;
        case '%':
            globalCenterY += globalCenterStep;
            break;
        case '6':
            globalCenterZ -= globalCenterStep;
            break;
        case '^':
            globalCenterZ += globalCenterStep;
            break;
        case '7':
            globalUpX -= globalUpStep;
            break;
        case '&':
            globalUpX += globalUpStep;
            break;
        case '8':
            globalUpY -= globalUpStep;
            break;
        case '*':
            globalUpY += globalUpStep;
            break;
        case '9':
            globalUpZ -= globalUpStep;
            break;
        case '(':
            globalUpZ += globalUpStep;
            break;
        case '[':
            globalStack += globalStackSliceStep;
            break;
        case '{':
            globalStack -= globalStackSliceStep;
            break;
        case ']':
            globalSlice += globalStackSliceStep;
            break;
        case '}':
            globalSlice -= globalStackSliceStep;
            break;

            /*reset environment*/
        case '0':
            /*Reset Rotation*/
            globalAngle = 0;
            globalRotateX = 0;
            globalRotateY = 0;
            globalRotateZ = 0;
            globalAngleStep = DEFAULT_GLOBAL_ANGLE_STEP;

            /*Reset Translation*/
            globalTranslateStep = DEFAULT_GLOBAL_TRANSLATE_STEP;
            globalTranslateX = 0;
            globalTranslateY = 0;
            globalTranslateZ = 0;

            /*Reset Scale*/
            globalScaleStep = DEFAULT_GLOBAL_SCALE_STEP;
            globalScaleX = 1;
            globalScaleY = 1;
            globalScaleZ = 1;

            /*Reset Background Color*/
            globalBackgroundColorR = 0;
            globalBackgroundColorG = 0;
            globalBackgroundColorB = 0;

            /*Reset Camera*/
            globalEyeStep = DEFAULT_GLOBAL_EYE_STEP;
            globalEyeX = DEFAULT_GLOBAL_EYE_X;
            globalEyeY = DEFAULT_GLOBAL_EYE_Y;
            globalEyeZ = DEFAULT_GLOBAL_EYE_Z;

            globalCenterStep = DEFAULT_GLOBAL_CENTER_STEP;
            globalCenterX = DEFAULT_GLOBAL_CENTER_X;
            globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
            globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;

            globalUpStep = DEFAULT_GLOBAL_UP_STEP;
            globalUpX = DEFAULT_GLOBAL_UP_X;
            globalUpY = DEFAULT_GLOBAL_UP_Y;
            globalUpZ = DEFAULT_GLOBAL_UP_Z;

            globalStack = DEFAULT_GLOBAL_STACK;
            globalSlice = DEFAULT_GLOBAL_SLICE;
            break;

        default:
            break;
    }
    glClearColor(globalBackgroundColorR, globalBackgroundColorG, globalBackgroundColorB, 1.0); //update background color
    glutPostRedisplay(); //request display() call ASAP
}


void processSpecialKeys(int key, int xx, int yy) {

// put code to process special keys in here

}

int main2() {
    tinyxml2::XMLDocument doc;
    doc.LoadFile("test.xml");

    const float title = doc.FirstChildElement("world")->FirstChildElement("camera")->FirstChildElement(
            "position")->FloatAttribute("x");

    const float time = doc.FirstChildElement("world")->FirstChildElement("group")->FirstChildElement(
            "translate")->FloatAttribute("time");

    const bool alignAttr = doc.FirstChildElement("world")->FirstChildElement("group")->FirstChildElement(
            "translate")->BoolAttribute("align");
    const bool alignText = doc.FirstChildElement("world")->FirstChildElement("group")->FirstChildElement(
            "translate")->BoolText("align");

    printf("%f", title);
    return 1;
}


int render_pyramid(int argc, char **argv) {

// init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("proj");

// Required callback registry
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

// Callback registration for keyboard processing
    glutKeyboardFunc(keyboardFunc);
    glutSpecialFunc(processSpecialKeys);

//  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

// enter GLUT's main cycle
    glutMainLoop();

    return 1;
}

int main(int argc, char **argv) {
  main2();
  return 0;
}
