/*! @addtogroup engine
 * @{*/

#ifdef __APPLE__
#include <GLUT/glut.h>
#else

#include <GL/glew.h>
#include <GL/glut.h>

#endif

#include <tinyxml2.h>
#include <stdio.h>
#include <vector>

#include "parsing.h"

#include <cmath>
#include "curves.h"
#include <glm/glm.hpp>
#include <iostream>

/* Required!
We probably don't need all this brings into scope, but it's not relevant.
*/
using namespace std;

/*rotation*/
const unsigned int DEFAULT_GLOBAL_ANGLE_STEP = 16;
static float globalAngleStep = DEFAULT_GLOBAL_ANGLE_STEP;
static float globalAngle = 0;
static float globalRotateX = 0;
static float globalRotateY = 0;
static float globalRotateZ = 0;

/*translation*/
const unsigned int DEFAULT_GLOBAL_TRANSLATE_STEP = 1;
static float globalTranslateStep = DEFAULT_GLOBAL_TRANSLATE_STEP;
static float globalTranslateX = 0;
static float globalTranslateY = 0;
static float globalTranslateZ = 0;

/*scaling*/
const unsigned int DEFAULT_GLOBAL_SCALE_STEP = 1;
float globalScaleStep = DEFAULT_GLOBAL_SCALE_STEP;
static float globalScaleX = 1;
static float globalScaleY = 1;
static float globalScaleZ = 1;

/*
VBO related variables.
*/

/*
glGenBuffer will place a buffer object name for VBO mode (no index) in this variable.
Each model needs an index, so there must be a globally accessible vector for all models.
*/
vector<GLuint> vbo_indices (100);
unsigned int VBO;


/*! @addtogroup camera
 * @{*/

/*! @addtogroup position
 * @{*/
const unsigned int DEFAULT_GLOBAL_EYE_STEP = 1;
static float DEFAULT_GLOBAL_EYE_X = 0;
static float DEFAULT_GLOBAL_EYE_Y = 0;
static float DEFAULT_GLOBAL_EYE_Z = 0;

static double globalEyeStep = DEFAULT_GLOBAL_EYE_STEP;
static double globalEyeX = DEFAULT_GLOBAL_EYE_X;
static double globalEyeY = DEFAULT_GLOBAL_EYE_Y;
static double globalEyeZ = DEFAULT_GLOBAL_EYE_Z;
//!@} end of group position

/*! @addtogroup lookAt
 * @{*/
const unsigned int DEFAULT_GLOBAL_CENTER_STEP = 1.0;
static float DEFAULT_GLOBAL_CENTER_X = 0;
static float DEFAULT_GLOBAL_CENTER_Y = 0;
static float DEFAULT_GLOBAL_CENTER_Z = 0;

float globalCenterStep = DEFAULT_GLOBAL_CENTER_STEP;
static float globalCenterX = DEFAULT_GLOBAL_CENTER_X;
static float globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
static float globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;
//!@} end of group lookAt

/*! @addtogroup up
 * @{*/
const unsigned int DEFAULT_GLOBAL_UP_STEP = 1;
static float DEFAULT_GLOBAL_UP_X = 0;
static float DEFAULT_GLOBAL_UP_Y = 1;
static float DEFAULT_GLOBAL_UP_Z = 0;

float globalUpStep = DEFAULT_GLOBAL_UP_STEP;
static float globalUpX = DEFAULT_GLOBAL_UP_X;
static float globalUpY = DEFAULT_GLOBAL_UP_Y;
static float globalUpZ = DEFAULT_GLOBAL_UP_Z;
//!@} end of group up

/*! @addtogroup projection
 * @{*/
static float DEFAULT_GLOBAL_FOV = 60;
static float DEFAULT_GLOBAL_NEAR = 1;
static float DEFAULT_GLOBAL_FAR = 1000;

static float globalFOV = DEFAULT_GLOBAL_FOV;
static float globalNear = DEFAULT_GLOBAL_NEAR;
static float globalFar = DEFAULT_GLOBAL_FAR;
//!@} end of group projection
//!@} end of group camera

void env_load_defaults ()
{
  /*rotation*/
  globalAngleStep = DEFAULT_GLOBAL_ANGLE_STEP;
  globalAngle = 0;
  globalRotateX = 0;
  globalRotateY = 0;
  globalRotateZ = 0;

  /*translation*/
  globalTranslateStep = DEFAULT_GLOBAL_TRANSLATE_STEP;
  globalTranslateX = 0;
  globalTranslateY = 0;
  globalTranslateZ = 0;

  /*scaling*/
  globalScaleStep = DEFAULT_GLOBAL_SCALE_STEP;
  globalScaleX = 1;
  globalScaleY = 1;
  globalScaleZ = 1;

  /*position*/
  globalEyeStep = DEFAULT_GLOBAL_EYE_STEP;
  globalEyeX = DEFAULT_GLOBAL_EYE_X;
  globalEyeY = DEFAULT_GLOBAL_EYE_Y;
  globalEyeZ = DEFAULT_GLOBAL_EYE_Z;

  globalRadius = DEFAULT_GLOBAL_RADIUS;
  globalAzimuth = DEFAULT_GLOBAL_AZIMUTH;
  globalElevation = DEFAULT_GLOBAL_ELEVATION;

  /*lookAt*/
  globalCenterStep = DEFAULT_GLOBAL_CENTER_STEP;
  globalCenterX = DEFAULT_GLOBAL_CENTER_X;
  globalCenterY = DEFAULT_GLOBAL_CENTER_Y;
  globalCenterZ = DEFAULT_GLOBAL_CENTER_Z;

  /*up*/

  globalUpStep = DEFAULT_GLOBAL_UP_STEP;
  globalUpX = DEFAULT_GLOBAL_UP_X;
  globalUpY = DEFAULT_GLOBAL_UP_Y;
  globalUpZ = DEFAULT_GLOBAL_UP_Z;

  /*projection*/
  globalFOV = DEFAULT_GLOBAL_FOV;
  globalNear = DEFAULT_GLOBAL_NEAR;
  globalFar = DEFAULT_GLOBAL_FAR;
}

//! @defgroup modelEngine Model

/*! @addtogroup modelEngine
 * @{*/
typedef struct model {
  float *vertices;
  unsigned int nVertices;
} *Model;

static std::vector<Model> globalModels;
static std::vector<float> globalOperations;
static bool operations_hasBeenInitialized = false;

Model allocModel (const char *path)
{
  FILE *fp = fopen (path, "r");
  if (!fp)
    {
      fprintf (stderr, "failed to open model: %s\n", path);
      exit (1);
    }

  unsigned int nVertices;
  fread (&nVertices, sizeof (unsigned int), 1, fp);
  float *modelBuf = (float *) malloc (3 * nVertices * sizeof (float));

  fread (modelBuf, 3 * sizeof (float), nVertices, fp);
  fclose (fp);

  Model model = (Model) malloc (sizeof (Model));
  if (model == NULL)
    {
      fprintf (stderr, "Failed loading model\n");
      exit (1);
    }

  model->nVertices = nVertices;
  model->vertices = modelBuf;

  return model;
}

void renderModel (Model model, GLuint vbo_ix)
{
  if (!model->nVertices % 3)
    {
      fprintf (stderr, "Number of coordinates is not divisible by 3");
      exit (1);
    }

  glBindBuffer (GL_ARRAY_BUFFER, vbo_ix);
  glVertexPointer (3, GL_FLOAT, 0, 0);
  glDrawArrays (GL_TRIANGLES, 0, model->nVertices);
  glBindBuffer (GL_ARRAY_BUFFER, 0); //unbind
}

void renderAllModels ()
{
  for (int i = 0; i < globalModels.size (); i++)
    {
      renderModel (globalModels[i], vbo_indices[i]);
    }
}

size_t readModelToBuffer (const char *path, float *p, unsigned int n)
{
  FILE *fp = fopen (path, "r");
  size_t r = fread (p, sizeof (float), n, fp);
  fclose (fp);
  return r;
}
//!@} end of group modelEngine

void renderTriangleVertexSeq (float *p, unsigned int nVertices)
{
  if (!nVertices % 3)
    {
      fprintf (stderr, "Number of coordinates is not divisible by 3");
      exit (-1);
    }
  glBegin (GL_TRIANGLES);
  for (unsigned int i = 0; i < nVertices; i += 3)
    glVertex3f (p[i], p[i + 1], p[i + 2]);
  glEnd ();
}

void changeSize (int w, int h)
{

  // Prevent a divide by zero, when window is too short
  // (you cant make a window with zero width).
  if (h == 0)
    h = 1;

  // compute window's aspect ratio
  float ratio = (float) w * 1.0f / (float) h;

  // Set the projection matrix as current
  glMatrixMode (GL_PROJECTION);
  // Load Identity Matrix
  glLoadIdentity ();

  // Set the viewport to be the entire window
  glViewport (0, 0, w, h);

  // Set perspective
  gluPerspective (globalFOV, ratio, globalNear, globalFar); //fox,near,far

  // return to the model view matrix mode
  glMatrixMode (GL_MODELVIEW);
}
//!@} end of group engine

void advance_in_rotation (const float rotation_time, const vec3 &axis_of_rotation)
{
  const float gt = fmodf ((float) glutGet (GLUT_ELAPSED_TIME), (float) (rotation_time * 1000)) / (rotation_time * 1000);
  const float angle = 360 * gt;
  glRotatef (angle, axis_of_rotation.x, axis_of_rotation.y, axis_of_rotation.z);
}

//! @ingroup Operations
void operations_render (std::vector<float> *operations)
{
  unsigned int i = 0;
  static bool hasPushedModels = false;
  static bool hasLoadedCurves = false;
  static vector<vector<vec3>> curves;

  DEFAULT_GLOBAL_EYE_X = operations->at (i);
  DEFAULT_GLOBAL_EYE_Y = operations->at (i + 1);
  DEFAULT_GLOBAL_EYE_Z = operations->at (i + 2);

  cartesian2Spherical (
      DEFAULT_GLOBAL_EYE_X, DEFAULT_GLOBAL_EYE_Y, DEFAULT_GLOBAL_EYE_Z,
      &DEFAULT_GLOBAL_RADIUS, &DEFAULT_GLOBAL_AZIMUTH, &DEFAULT_GLOBAL_ELEVATION);
  i += 3;

  DEFAULT_GLOBAL_CENTER_X = operations->at (i);
  DEFAULT_GLOBAL_CENTER_Y = operations->at (i + 1);
  DEFAULT_GLOBAL_CENTER_Z = operations->at (i + 2);
  i += 3;

  DEFAULT_GLOBAL_UP_X = operations->at (i);
  DEFAULT_GLOBAL_UP_Y = operations->at (i + 1);
  DEFAULT_GLOBAL_UP_Z = operations->at (i + 2);
  i += 3;

  DEFAULT_GLOBAL_FOV = operations->at (i);
  DEFAULT_GLOBAL_NEAR = operations->at (i + 1);
  DEFAULT_GLOBAL_FAR = operations->at (i + 2);

  unsigned int model_num = 0;

  for (i += 3; i < operations->size (); i++)
    {
      switch ((int) operations->at (i))
        {
          case ROTATE:
            glRotatef (operations->at (i + 1),
                       operations->at (i + 2),
                       operations->at (i + 3),
                       operations->at (i + 4));
          i += 4;
          continue;
          case EXTENDED_ROTATE:
            {
              advance_in_rotation (
                  operations->at (i + 1),
                  {
                      operations->at (i + 2),
                      operations->at (i + 3),
                      operations->at (i + 4)
                  });
              i += 4; //time, axis_of_rotation
              continue;
            }
          case TRANSLATE:
            glTranslatef (operations->at (i + 1),
                          operations->at (i + 2),
                          operations->at (i + 3));
          i += 3;
          continue;
          case EXTENDED_TRANSLATE:
            {
              int number_of_points = (int) operations->at (i + 3);
              vector<vec3> new_curve (number_of_points);
              if (!hasLoadedCurves)
                {
                  for (int j = 0; j < number_of_points; ++j)
                    {
                      int idx = 3*j;
                      new_curve.at (j)[0] = operations->at (i + 4 + idx);
                      new_curve.at (j)[1] = operations->at (i + 4 + idx + 1);
                      new_curve.at (j)[2] = operations->at (i + 4 + idx + 2);
                    }
                  curves.push_back (new_curve);
                }
              auto curve = curves.back ();
              renderCurve (Mcr, curve);
              advance_in_curve (
                  operations->at (i + 1),
                  (bool) operations->at (i + 2),
                  Mcr,
                  curves.back ());
              i += 3 + number_of_points * 3; // 3 - time,align,number_of_points,points...
              continue;
            }
          case SCALE:
            glScalef (operations->at (i + 1),
                      operations->at (i + 2),
                      operations->at (i + 3));
          i += 3;
          continue;
          case BEGIN_GROUP:
            glPushMatrix ();
          continue;
          case END_GROUP:
            glPopMatrix ();
          continue;
          case LOAD_MODEL:
            {
              int stringSize = (int) operations->at (i + 1);
              if (!hasPushedModels)
                {
                  char modelName[stringSize + 1];

                  int j;
                  for (j = 0; j < stringSize; j++)
                    modelName[j] = (char) operations->at (i + 2 + j);

                  modelName[j] = 0;

                  globalModels.push_back (allocModel (modelName));
                  glGenBuffers (1, &vbo_indices[model_num]);
                  glBindBuffer (GL_ARRAY_BUFFER, vbo_indices[model_num]);
                  Model m = globalModels.back ();
                  glBufferData (GL_ARRAY_BUFFER, sizeof (float) * 3 * m->nVertices, m->vertices, GL_STATIC_DRAW);
                  glBindBuffer (GL_ARRAY_BUFFER, 0); //unbind
                  free (m->vertices);
                }

              renderModel (globalModels[model_num], vbo_indices[model_num]);
              model_num++;
              i += stringSize + 1; //just to be explicit
              continue;
            }
        }
    }
  hasPushedModels = true;
  hasLoadedCurves = true;
}

void draw_axes ()
{
  /*draw absolute (before any transformation) axes*/
  glBegin (GL_LINES);
  /*X-axis in red*/
  glColor3f (1.0f, 0.0f, 0.0f);
  glVertex3f (
      -100.0f, 0.0f, 0.0f);
  glVertex3f (100.0f, 0.0f, 0.0f);

  /*Y-Axis in Green*/
  glColor3f (0.0f, 1.0f, 0.0f);
  glVertex3f (0.0f,
              -100.0f, 0.0f);
  glVertex3f (0.0f, 100.0f, 0.0f);

  /*Z-Axis in Blue*/
  glColor3f (0.0f, 0.0f, 1.0f);
  glVertex3f (0.0f, 0.0f,
              -100.0f);
  glVertex3f (0.0f, 0.0f, 100.0f);
  glColor3d (1, 1, 1);
  glEnd ();
  /*end of draw absolute (before any transformation) axes*/
}

/*!@addtogroup engine
 * @{*/
void redisplay ()
{
  spherical2Cartesian (globalRadius, globalElevation, globalAzimuth, &globalEyeX, &globalEyeY, &globalEyeZ);
  glutPostRedisplay ();
}

void renderScene ()
{
  float fps;
  int time;
  char s[64];

  // clear buffers
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // helps in avoiding rounding mistakes, discards old matrix, read more
  glLoadIdentity ();


  // set the camera
  gluLookAt (globalEyeX, globalEyeY, globalEyeZ,
             globalCenterX, globalCenterY, globalCenterZ,
             globalUpX, globalUpY, globalUpZ);

  // put the geometric transformations here
  glRotatef (globalAngle, globalRotateX, globalRotateY, globalRotateZ);
  glTranslatef (globalTranslateX, globalTranslateY, globalTranslateZ);
  glScalef (globalScaleX, globalScaleY, globalScaleZ);

  operations_render (&globalOperations);

  frame++;
  time = glutGet (GLUT_ELAPSED_TIME);
  if (time - timebase > 1000)
    {
      fps = frame * 1000.0 / (time - timebase);
      timebase = time;
      frame = 0;
      sprintf (s, "FPS: %f6.2", fps);
      glutSetWindowTitle (s);
    }

  // End of frame
  redisplay ();
  glutSwapBuffers ();
}

void xml_load_and_set_env (const char *filename)
{
  operations_load_xml (filename, &globalOperations);
  operations_render (&globalOperations);
  env_load_defaults ();
}
/*! @addtogroup input
 * @{*/
void keyboardFunc (unsigned char key, int xmouse, int ymouse)
{
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
  switch (key)
    {
      /*Rotation*/
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
      case 'T':
        globalTranslateStep *= 2;
      break;
      case 't':
        if (globalTranslateStep > 1)
          globalTranslateStep /= 2;
      break;

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

      /*camera*/
      case '`':
        globalEyeStep *= 2;
      break;
      case '~':
        if (globalEyeStep > 1)
          globalEyeStep /= 2;
      break;

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

      /*reset environment*/
      case '0':
        env_load_defaults ();
      break;

      default:
        break;
    }
  redisplay (); //request display() call ASAP
}

int globalXPrev = 0;
int globalYPrev = 0;

void mouseFunc (int button, int state, int x, int y)
{
  // Wheel reports as button 3(scroll up) and button 4(scroll down)
  if ((button == 3) || (button == 4)) // It's a wheel event
    {
      // Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
      if (state == GLUT_UP)
        return; // Disregard redundant GLUT_UP events

      if (button == 3)
        {
          globalRadius *= .8f;
          if (globalRadius < 1.0f)
            globalRadius = 1.0f;
        }
      else
        globalRadius *= 1.5;
      printf ("Scroll %s At %d %d\n", (button == 3) ? "Up" : "Down", x, y);
    }
  else
    {  // normal button event
      printf ("Button %s At %d %d\n", (state == GLUT_DOWN) ? "Down" : "Up", x, y);
      if (button == GLUT_LEFT_BUTTON)
        {
          if (state == GLUT_DOWN)
            {
              globalMouseLeftButton = true;
              globalXPrev = x;
              globalYPrev = y;
            }
          if (state == GLUT_UP)
            {
              globalMouseLeftButton = false;
            }
        }
      else if (button == GLUT_RIGHT_BUTTON)
        {
          if (state == GLUT_DOWN)
            {
              globalEyeX = x;
              globalEyeZ = y;
            }
        }
    }

  redisplay ();
}

void motionFunc (int x, int y)
{
  if (globalMouseLeftButton)
    {
      const double stepAlfa = 0.035;
      const double stepBeta = 0.035;

      if (x < globalXPrev)
        globalAzimuth += stepAlfa;
      else if (x > globalXPrev)
        globalAzimuth -= stepAlfa;

      if (y < globalYPrev)
        {
          globalElevation -= stepBeta;
          if (globalElevation < -1.5)
            globalElevation = -1.5;
        }
      else if (y > globalYPrev)
        {
          globalElevation += stepBeta;
          if (globalElevation > 1.5)
            globalElevation = 1.5;
        }

      globalXPrev = x;
      globalYPrev = y;
    }
  redisplay ();
}

//! @} end of group input
void engine_run (int argc, char **argv)
{

  if (argc != 2)
    {
      fprintf (stderr, "Number of arguments is not one\n");
      exit (1);
    }

  // init GLUT and the window
  glutInit (&argc, argv);
  glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
  glutInitWindowPosition (100, 100);
  glutInitWindowSize (800, 800);
  glutCreateWindow ("engine");

  // Required callback registry
  glutDisplayFunc (renderScene);
  glutReshapeFunc (changeSize);

  // Callback registration for keyboard processing
  glutKeyboardFunc (keyboardFunc);

  // Callback registration for mouse processing
  glutMouseFunc (mouseFunc);
  glutMotionFunc (motionFunc);

  //  OpenGL settings
  glEnable (GL_DEPTH_TEST);
  glEnableClientState (GL_VERTEX_ARRAY);
  glEnable (GL_CULL_FACE);
  glPolygonMode (GL_FRONT, GL_LINE);

  // enter GLUT's main cycle
  glewInit ();
  xml_load_and_set_env (argv[1]);
  glutMainLoop ();
}

int main (int argc, char **argv)
{
  engine_run (argc, argv);
  return 1;
}
//!@} end of group engine

