#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/freeglut_std.h>
#include "curves.h"

using glm::mat4, glm::mat4x3;
using glm::vec4, glm::vec3, glm::vec2, glm::value_ptr;
using std::array, std::vector;

//! Catmull-rom matrix
const mat4 Mcr = {
    -0.5f, 1.5f, -1.5f, 0.5f,
    1.0f, -2.5f, 2.0f, -0.5f,
    -0.5f, 0.0f, 0.5f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f};

//! Bezier matrix
const mat4 Mb = {
    -1, 3, -3, 1,
    3, -6, 3, 0,
    -3, 3, 0, 0,
    1, 0, 0, 0};

/*!
 *
 * @param[in] t line coordinate of the point.
 * @param[in] M matrix used to define the curve (e.g. Catmull-rom or Bezier).
 * @param[in] control_points four three-dimensional points used to define the curve.
 * @param[out] coordinate_in_3d_space three-dimensional coordinate of the point.
 * @param[out] derivative_at_t derivative of the curve
 */
void get_curve_point_at (
    const float t,
    const mat4 &M,
    const array<vec3, 4> &control_points,
    vec3 &coordinate_in_3d_space,
    vec3 &derivative_at_t)
{
  // T = [t³,t²,t,1]; T_prime = [3t²,2t,1,0]

  const auto T = vec4 (pow (t, 3), pow (t, 2), t, 1);
  const auto T_prime = vec4 (3 * pow (t, 2), 2 * t, 1, 0);
  mat4x3 C (control_points[0], control_points[1], control_points[2], control_points[3]);

  const auto CM = C * M;
  coordinate_in_3d_space = CM * T;
  derivative_at_t = CM * T_prime;
}

void
get_curve_info (const float global_time,
                const vector<vec3> &control_points,
                float &local_time,
                array<vec3, 4> &curve_points)
{
  local_time = global_time * (float) control_points.size (); // this is the real global t
  int index = floor (local_time);  // which segment
  local_time = local_time - (float) index; // where within  the segment
  // indices store the points
  size_t indices[4];
  indices[0] = (index + control_points.size () - 1) % control_points.size ();
  indices[1] = (indices[0] + 1) % control_points.size ();
  indices[2] = (indices[1] + 1) % control_points.size ();
  indices[3] = (indices[2] + 1) % control_points.size ();

  curve_points = {
      control_points[indices[0]],
      control_points[indices[1]],
      control_points[indices[2]],
      control_points[indices[3]]};
}

void
get_curve_global_point (const float gt, const mat4 &M, const vector<vec3> &control_points, vec3 &pos, vec3 &deriv)
{
  float local_t;
  array<vec3, 4> Q{};
  get_curve_info (gt, control_points, local_t, Q);
  get_curve_point_at (local_t, M, Q, pos, deriv);
}

void
align_global_pos_mat (const float global_time,
                      const mat4 &M,
                      const vector<vec3> &global_control_points,
                      vec3 &pos,
                      mat4 &rot)
{
  vec3 X_i;
  vec3 Y_0 = {0, 1, 0};
  get_curve_global_point (global_time, M, global_control_points, pos, X_i);
  X_i = normalize (X_i);
  auto Z_i = normalize (cross (X_i, Y_0));
  auto Y_i = normalize ((cross (Z_i, X_i)));
  Z_i = normalize (cross (X_i, Y_i));

  vec4 zero_one = {0, 0, 0, 1};
  rot = {{X_i, 0}, {Y_i, 0}, {Z_i, 0}, zero_one};
}

void renderCurve (const mat4 M, const vector<vec3> &control_points, const unsigned int tesselation)
{
  const auto step = 1.0 / tesselation;
  glm::vec3 pos;
  glm::vec3 deriv;
  glPushMatrix ();
  glBegin (GL_LINE_STRIP);
  for (auto t = 0; t <= tesselation; ++t)
    {

      get_curve_global_point (t * step, M, control_points, pos, deriv);
      glVertex3f (pos.x, pos.y, pos.z);
    }
  glEnd ();

  //  for (auto p : control_points)
  //    {
  //      glPushMatrix ();
  //      glTranslatef (p.x, p.y, p.z);
  //      glutSolidSphere (.02, 10, 10);
  //      glPopMatrix ();
  //    }
  glPopMatrix ();
}

void advance_in_curve (const float translation_time,
                       const bool align,
                       const mat4 &M,
                       const vector<vec3> &global_control_points)
{
  // (x%N)/N
  float gt = fmodf ((float) glutGet (GLUT_ELAPSED_TIME), (float) (translation_time * 1000)) / (translation_time * 1000);
  vec3 pos;
  mat4 rot;
  align_global_pos_mat (gt, M, global_control_points, pos, rot);
  glTranslatef (pos.x, pos.y, pos.z);
  if (align)
    glMultMatrixf (value_ptr (rot));
}
