#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/freeglut_std.h>
#include "curves.h"

using glm::mat4x3, glm::value_ptr;

const mat4 Mcr = {
    -0.5f, 1.5f, -1.5f, 0.5f,
    1.0f, -2.5f, 2.0f, -0.5f,
    -0.5f, 0.0f, 0.5f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f};

const mat4 Mb =  {
    -1, 3, -3, 1,
    3, -6, 3, 0,
    -3, 3, 0, 0,
    1, 0, 0, 0};

template<typename T> inline vec4 dec_polynomial (T n)
{
  return {pow (n, 3), pow (n, 2), n, 1};
}
/*!
 * @param a a[m][p]
 * @param b b[p][q]
 * @param[out] r r[m][q]
 * @param m
 * @param q
 * @param p
 */
template<typename T1, typename T2, typename T3>
void mult (const T1 &a, const T2 &b, T3 &r, const int m, const int p, const int q)
{
  for (auto i = 0; i < m; ++i)
    {
      for (auto j = 0; j < q; ++j)
        {
          r[m * i + j] *= 0;
          for (auto k = 0; k < p; k++)
            {
              r[m * i + j] += a[m * i + k] * b[p * k + j];
            }
        }
    }
}

void get_curve (const float time, const mat4 &M, const array<vec3, 4> &control_points, vec3 &pos, vec3 &deriv)
{
  const auto t = glm::vec4 (pow (time, 3), pow (time, 2), time, 1);
  const auto t_prime = glm::vec4 (3 * pow (time, 2), 2 * time, 1, 0);
  mat4x3 K (control_points[0], control_points[1], control_points[2], control_points[3]);

  const auto KM = K * M;
  pos = KM * t;
  deriv = KM * t_prime;
}

vec3 get_curve (const float time, const mat4 &M, const array<vec3, 4> &control_points)
{
  vec3 pos;
  vec3 _;
  get_curve (time, M, control_points, pos, _);
  return pos;
}

auto get_curve (const mat4 &M)
{
  return [&M] (const array<vec3, 4> &control_points)
  {
    return [&M, &control_points] (const float time) -> vec3
    {
      return get_curve (time, M, control_points);
    };
  };
}

void
get_curve_info (const float global_time, const vector<vec3> &control_points, float &local_time, array<vec3,4> &curve_points)
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
get_curve_info2 (const float global_time, const vector<vec3> &global_control_points, float &local_time, array<vec3,
                                                                                                             4> &local_control_points)
{
  local_time = global_time * (float) global_control_points.size (); // this is the real global t
  int index = floor (local_time);  // which segment
  local_time = local_time - (float) index; // where within  the segment
  // indices store the points
  size_t indices[4];
  indices[0] = (index + global_control_points.size () - 1) % global_control_points.size ();
  indices[1] = (indices[0] + 1) % global_control_points.size ();
  indices[2] = (indices[1] + 1) % global_control_points.size ();
  indices[3] = (indices[2] + 1) % global_control_points.size ();

  local_control_points = {
      global_control_points[indices[0]],
      global_control_points[indices[1]],
      global_control_points[indices[2]],
      global_control_points[indices[3]]};
}

void
get_curve_global_point (const float gt, const mat4 &M, const vector<vec3> &control_points, vec3 &pos, vec3 &deriv)
{
  float local_t;
  array<vec3, 4> Q{};
  get_curve_info (gt, control_points, local_t, Q);
  get_curve (local_t, M, Q, pos, deriv);
}

void
align_global_pos_mat (float global_time, const mat4 &M, const vector<vec3> &global_control_points, vec3 &pos, mat4 &rot)
{
  vec3 X_i;
  vec3 Y_0 = {0, 1, 0};
  get_curve_global_point (global_time, M, global_control_points, pos, X_i);
  X_i = normalize (X_i);
  auto Z_i = normalize(cross ( X_i,Y_0));
  auto Y_i = normalize((cross ( Z_i,X_i)));
  Z_i = normalize(cross (X_i,Y_i));

  vec4 zero_one = {0, 0, 0, 1};
  rot = {{X_i, 0}, {Y_i, 0}, {Z_i, 0}, zero_one};
}

void renderCurve (const mat4 M, const vector<vec3> &control_points, const unsigned int tesselation)
{
  const auto step = 1.0 / tesselation;
  glm::vec3 pos;
  glm::vec3 deriv;
  glPushMatrix();
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
    glPopMatrix();
}

void advance_in_curve (float translation_time, bool align, const mat4 &M, const vector<vec3> &global_control_points)
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

vec3 get_bezier_point4 (const float u, const float v, const array<vec3, 16> &P)
{
  auto MU = Mb * dec_polynomial (u);
  auto B = get_curve (Mb); // bezier curve
  vec3 r = {0, 0, 0};
  for (int j = 0; j < 3; ++j)
    for (int i = 0; i < 3; ++i)
      r += B ({P[j], P[j + 1], P[j + 2], P[j + 3]}) (v)
           * P[4 * j + i]
           * B ({P[i], P[i + 1], P[i + 2], P[i + 3]}) (u);
  return r;
}

vec3 get_bezier_point (const float u, const float v, const array<vec3, 16> &P)
{

  vec3 P0u, P1u, P2u, P3u;
  vec3 _;
  get_curve (u, Mb, {P[0], P[1], P[2], P[3]}, P0u, _);
  get_curve (u, Mb, {P[4], P[5], P[6], P[7]}, P1u, _);
  get_curve (u, Mb, {P[8], P[9], P[10], P[11]}, P2u, _);
  get_curve (u, Mb, {P[12], P[13], P[14], P[15]}, P3u, _);

  vec3 Puv;
  get_curve (v, Mb, {P0u, P1u, P2u, P3u}, Puv, _);
  return Puv;
}

vec3 get_bezier_point3 (const float u, const float v, const array<vec3, 16> &P)
{
  auto MU = Mb * dec_polynomial (u);
  auto B = get_curve (Mb); // bezier curve
  return (
             B ({P[0], P[1], P[2], P[3]}) (v) * mat4x3 (P[0], P[4], P[8], P[12]) +
             B ({P[4], P[5], P[6], P[7]}) (v) * mat4x3 (P[1], P[5], P[9], P[13]) +
             B ({P[8], P[9], P[10], P[11]}) (v) * mat4x3 (P[2], P[6], P[10], P[14]) +
             B ({P[12], P[13], P[14], P[15]}) (v) * mat4x3 (P[3], P[7], P[11], P[15])
         ) * MU;
}

vec3 get_bezier_point2 (const float u, const float v, const array<vec3, 16> &P)
{
  // B(u,v) = U M P Mᵀ V
  vec4 U = dec_polynomial (u);
  vec4 V = dec_polynomial (v);

  vector<vec3> VMP;
  auto VM = V * glm::transpose (Mb); //1x4x1
  mult (VM, P, VMP, 1, 4, 4);

  vector<vec3> VMPMU;
  auto MU = Mb * U;
  mult (VMP, MU, VMPMU, 1, 4, 1);
  return VMPMU[0];
}

vector<vec3> get_bezier_patch (array<vec3, 16> control_points, int int_tesselation)
{
  vector<vec3> pontos;
  float float_tesselation = (float) int_tesselation;
  const auto step = 1.0 / float_tesselation;
  for (int v = 0; v < int_tesselation; v++)
    {
      for (int u = 0; u < int_tesselation; u++)
        {
          // triângulo superior
          pontos.push_back (get_bezier_point (u * step, v * step, control_points));
          pontos.push_back (get_bezier_point (u * step, v * step + step, control_points));
          pontos.push_back (get_bezier_point (u * step + step, v * step, control_points));
          // triângulo inferior
          pontos.push_back (get_bezier_point (u * step + step, v * step, control_points));
          pontos.push_back (get_bezier_point (u * step + step, v * step + step, control_points));
          pontos.push_back (get_bezier_point (u * step, v * step + step, control_points));
        }
    }
  /*std::cout << "get_bezier_patch got:" << std::endl;
  for (auto p : pontos)
    std::cout << glm::to_string (p) << std::endl;*/
  return pontos;
}

vector<glm::vec3> get_bezier_surface (const vector<array<glm::vec3, 16>> &control_set, int int_tesselation)
{
  vector<glm::vec3> pts;
  for (auto &i : control_set)
    {
      vector<glm::vec3> temp = get_bezier_patch (i, int_tesselation);
      pts.insert (pts.end (), temp.begin (), temp.end ());
    }

  return pts;
}