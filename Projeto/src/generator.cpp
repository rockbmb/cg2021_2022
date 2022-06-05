#include <cmath>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <string>
#include <fstream>
#include <tuple>
#include <iostream>
#include <csignal>

#include "curves.h"

using glm::mat4, glm::vec4, glm::vec3, glm::vec2, glm::mat4x3;
using glm::normalize, glm::cross;

using std::vector, std::tuple, std::array;

using std::string, std::ifstream, std::ios, std::stringstream;
using std::cerr, std::endl, glm::to_string;

template<class T>
concept arithmetic =  std::is_integral<T>::value or std::is_floating_point<T>::value;

const char *SPHERE = "sphere";
const char *CUBE = "box";
const char *CONE = "cone";
const char *PLANE = "plane";
const char *BEZIER = "bezier";
/*! @addtogroup generator
* @{*/

struct baseModel {
  int nVertices;
  float *vertices;
  float *normals;
  float *texture_coordinates;
};

/*! @addtogroup points
 * @{*/
void points_vertex (const float x, const float y, const float z, unsigned int *pos, float points[])
{
  points[*pos] = x;
  points[*pos + 1] = y;
  points[*pos + 2] = z;
  *pos += 3;
}

void points_write (const char *filename, const unsigned int nVertices, const float points[])
{
  FILE *fp = fopen (filename, "w");
  if (!fp)
    {
      fprintf (stderr, "failed to open file: %s", filename);
      exit (1);
    }

  fwrite (&nVertices, sizeof (unsigned int), 1, fp);
  fwrite (points, 3 * sizeof (float), nVertices, fp);

  fclose (fp);
}

void
model_write (const char *const filename,
             const vector<vec3> &vertices,
             const vector<vec3> &normals,
             const vector<vec2> &texture)
{
  FILE *fp = fopen (filename, "w");

  if (!fp)
    {
      fprintf (stderr, "failed to open file: %s", filename);
      exit (1);
    }

  assert(vertices.size () < INT_MAX);
  const int nVertices = vertices.size ();
  const int nNormals = normals.size ();
  const int nTextures = texture.size ();
  fwrite (&nVertices, sizeof (nVertices), 1, fp);
  fwrite (vertices.data (), sizeof (vertices.back ()), nVertices, fp);
  fwrite (normals.data (), sizeof (normals.back ()), nNormals, fp);
  fwrite (texture.data (), sizeof (texture.back ()), nTextures, fp);

  fclose (fp);

  cerr << "[generator] Wrote "
       << nVertices << " vertices, "
       << nNormals << " normals, "
       << nTextures << " textures to "
       << filename << endl;
}

//!@} end of group points

/*! @addtogroup model
 * @{*/

/*! @addtogroup plane
* @{*/
void model_plane_vertices (const float length,
                           const unsigned int divisions,
                           vector<vec3> &vertices,
                           vector<vec3> &normals,
                           vector<vec2> &texture)
{
  const float o = -length / 2.0f;
  const float d = length / (float) divisions;

  for (unsigned int uidiv1 = 1; uidiv1 <= divisions; ++uidiv1)
    {
      for (unsigned int uidiv2 = 1; uidiv2 <= divisions; ++uidiv2)
        {
          auto const fdiv1 = (float) uidiv1;
          auto const fdiv2 = (float) uidiv2;
          auto const fdivisions = (float) divisions;

          for (auto e : {
              array<float, 2>{-1, -1},//P1
              array<float, 2>{-1, 0},//P1'z
              array<float, 2>{0, -1},//P1'x

              array<float, 2>{0, -1},//P1'x
              array<float, 2>{-1, 0},//P1'z
              array<float, 2>{0, 0}})//P2
            {
              vertices.emplace_back (o + d * (fdiv1 + e[0]), 0, o + d * (fdiv2 + e[1]));
              normals.emplace_back (0, 1, 0);
              texture.emplace_back ((fdiv1 + e[0]) / fdivisions, (fdiv2 + e[1]) / fdivisions);
            }


          /*Cull face*/
          for (auto e : {
              array<float, 2>{-1.0f, 0.0f},
              array<float, 2>{-1.0f, -1.0f},
              array<float, 2>{0.0f, -1.0f},

              array<float, 2>{-1.0f, 0.0f},
              array<float, 2>{0.0f, -1.0f},
              array<float, 2>{0.0f, 0.0f}})
            {
              vertices.emplace_back (o + d * (fdiv1 + e[0]), 0, o + d * (fdiv2 + e[1]));
              normals.emplace_back (0, -1, 0);
              texture.emplace_back ((fdiv1 + e[0]) / fdivisions, (fdiv2 + e[1]) / fdivisions);
            }
        }
    }
}

static inline unsigned int model_plane_nVertices (const unsigned int divisions)
{ return divisions * divisions * 12; }

void model_plane_write (const char *filepath, const float length, const unsigned int divisions)
{
  const unsigned int nVertices = model_plane_nVertices (divisions);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);
  model_plane_vertices (length, divisions, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
}
//!@} end of group plane

/*! @addtogroup cube
* @{*/
void model_cube_vertices (const float length,
                          const unsigned int divisions,
                          vector<vec3> &vertices,
                          vector<vec3> &normals,
                          vector<vec2> &texture)
{
  const float o = -length / 2.0f;
  const float d = length / (float) divisions;

  for (unsigned int uidiv1 = 1; uidiv1 <= divisions; uidiv1++)
    {
      for (unsigned int uidiv2 = 1; uidiv2 <= divisions; uidiv2++)
        {
          auto const fdiv1 = (float) uidiv1;
          auto const fdiv2 = (float) uidiv2;
          auto const fdivisions = (float) divisions;

          // y+
          for (auto e : {
              array<float, 2>{-1, -1}, //P1
              array<float, 2>{-1, 0}, //P1'z
              array<float, 2>{0, -1}, //P1'x

              array<float, 2>{0, -1}, //P1'x
              array<float, 2>{-1, 0}, //P1'z
              array<float, 2>{0, 0}   //P2
          })
            {
              vertices.emplace_back (o + d * (fdiv1 + e[0]), -o, o + d * (fdiv2 + e[1]));
              normals.emplace_back (0, 1, 0);
              texture.emplace_back ((fdiv1 + e[0]) / fdivisions, (fdiv2 + e[1]) / fdivisions);
            }

          // y-
          vertices.emplace_back (o + d * (fdiv1 - 1), o, o + d * fdiv2); //P1'z
          vertices.emplace_back (o + d * (fdiv1 - 1), o, o + d * (fdiv2 - 1)); //P1
          vertices.emplace_back (o + d * fdiv1, o, o + d * (fdiv2 - 1)); //P1'x

          vertices.emplace_back (o + d * (fdiv1 - 1), o, o + d * fdiv2); //P1'z
          vertices.emplace_back (o + d * fdiv1, o, o + d * (fdiv2 - 1)); //P1'x
          vertices.emplace_back (o + d * fdiv1, o, o + d * fdiv2); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (0, -1, 0);
          for (auto e : {
              vec2 (-1.0f, 0.0f),
              vec2 (-1.0f, -1.0f),
              vec2 (0.0f, -1.0f),

              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 + e[0]) / fdivisions, (fdiv2 + e[1]) / fdivisions);


          // x-
          vertices.emplace_back (o, o + d * (fdiv1 - 1), o + d * (fdiv2 - 1)); //P1
          vertices.emplace_back (o, o + d * (fdiv1 - 1), o + d * fdiv2); //P1'z
          vertices.emplace_back (o, o + d * fdiv1, o + d * (fdiv2 - 1)); //P1'x

          vertices.emplace_back (o, o + d * fdiv1, o + d * (fdiv2 - 1)); //P1'x
          vertices.emplace_back (o, o + d * (fdiv1 - 1), o + d * fdiv2); //P1'z
          vertices.emplace_back (o, o + d * fdiv1, o + d * fdiv2); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (-1, 0, 0);

          for (auto e : {
              vec2 (-1.0f, -1.0f),
              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),

              vec2 (0.0f, -1.0f),
              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 + e[0]) / fdivisions, (fdiv2 + e[1]) / fdivisions);


          // x+
          vertices.emplace_back (-o, o + d * (fdiv1 - 1), o + d * fdiv2); //P1'z
          vertices.emplace_back (-o, o + d * (fdiv1 - 1), o + d * (fdiv2 - 1)); //P1
          vertices.emplace_back (-o, o + d * fdiv1, o + d * (fdiv2 - 1)); //P1'x

          vertices.emplace_back (-o, o + d * (fdiv1 - 1), o + d * fdiv2); //P1'z
          vertices.emplace_back (-o, o + d * fdiv1, o + d * (fdiv2 - 1)); //P1'x
          vertices.emplace_back (-o, o + d * fdiv1, o + d * fdiv2); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (1, 0, 0);

          for (auto e : {
              vec2 (-1.0f, 0.0f),
              vec2 (-1.0f, -1.0f),
              vec2 (0.0f, -1.0f),

              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),
              vec2 (0.0f, 0.0f)})
            texture.emplace_back ((fdiv1 + e[0]) / fdivisions, (fdiv2 + e[1]) / fdivisions);


          // z-
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * (fdiv2 - 1), o); //P1
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * fdiv2, o); //P1'z
          vertices.emplace_back (o + d * fdiv1, o + d * (fdiv2 - 1), o); //P1'x

          vertices.emplace_back (o + d * fdiv1, o + d * (fdiv2 - 1), o); //P1'x
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * fdiv2, o); //P1'z
          vertices.emplace_back (o + d * fdiv1, o + d * fdiv2, o); //P2

          for (auto e : {
              vec2 (-1.0f, -1.0f),//P1
              vec2 (-1.0f, 0.0f),//P1'z
              vec2 (0.0f, -1.0f),//P1'x

              vec2 (0.0f, -1.0f),//P1'x
              vec2 (-1.0f, 0.0f),//P1'z
              vec2 (0.0f, 0.0f)//P2
          })
            {
              normals.emplace_back (0, 0, -1);
              texture.emplace_back ((fdiv1 + e[0]) / fdivisions, (fdiv2 + e[1]) / fdivisions);
            }



          // z+
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * fdiv2, -o); //P1'z
          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * (fdiv2 - 1), -o); //P1
          vertices.emplace_back (o + d * fdiv1, o + d * (fdiv2 - 1), -o); //P1'x

          vertices.emplace_back (o + d * (fdiv1 - 1), o + d * fdiv2, -o); //P1'z
          vertices.emplace_back (o + d * fdiv1, o + d * (fdiv2 - 1), -o); //P1'x
          vertices.emplace_back (o + d * fdiv1, o + d * fdiv2, -o); //P2

          for (int k = 0; k < 6; ++k)
            normals.emplace_back (0, 0, 1);

          for (auto e : {
              vec2 (-1.0f, 0.0f),
              vec2 (-1.0f, -1.0f),
              vec2 (0.0f, -1.0f),

              vec2 (-1.0f, 0.0f),
              vec2 (0.0f, -1.0f),
              vec2 (0.0f, 0.0f)
          })
            texture.emplace_back ((fdiv1 + e[0]) / fdivisions, (fdiv2 + e[1]) / fdivisions);
        }
    }
}

static inline unsigned int model_cube_nVertices (const unsigned int divisions)
{ return divisions * divisions * 36; }

void model_cube_write (const char *const filepath,
                       const float length,
                       const unsigned int divisions)
{
  const unsigned int nVertices = model_cube_nVertices (divisions);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);

  model_cube_vertices (length, divisions, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
}

//!@} end of group cube

/*! @addtogroup cone
* @{*/

/*!
 * \f{aligned}{
 * x &= r⋅\frac{h}{\textrm{height}} ⋅ \cos(θ)\\[2em]
 * y &= h + \textrm{height}\\[2em]
 * z &= r⋅\frac{h}{\textrm{height}} ⋅ \sin(θ)
 * \f}\n
 *
 * \f{aligned}{
 *  r &≥ 0\\
 *  θ &∈ \left\{-π      + i⋅s : s = \frac{2π}{\textrm{slices}}      ∧ i ∈ \{0,...,\textrm{slices}\} \right\}\\
 *  h &∈ \left\{- \textrm{height} + j⋅t : t =  \frac{\textrm{height}}{\textrm{stacks}} ∧ j ∈ \{0,...,\textrm{stacks}\} \right\}
 *  \f}
 *
 *  See the [3d model](https://www.math3d.org/7oeSkmuns).
 */

template<typename T>
    requires arithmetic<T>
static inline void
model_cone_vertex (const T r,
                   const T height,
                   const T theta,
                   const T h,
                   vector<vec3> &vertices)
{
  /*
     x = r ⋅ (h/height) ⋅ cos(θ)
     y = 2 ⋅ (height + h)
     z = r ⋅ (h/height) ⋅ sin(θ)

     r ≥ 0
     θ ∈ {-π      + i⋅s : s = 2π/slices      ∧ i ∈ {0,...,slices} }
     h ∈ {-height + j⋅t : t = height/stacks ∧ j ∈ {0,...,stacks} }

     check:
         1. https://www.math3d.org/7oeSkmuns
   */
  vertices.emplace_back (r * h / height * cos (theta), height + h, r * h / height * sin (theta));
}

template<typename T>
    requires arithmetic<T>
void model_cone_vertices (const T radius,
                          const T height,
                          const unsigned int slices,
                          const unsigned int stacks,
                          vector<vec3> &vertices,
                          vector<vec3> &normals,
                          vector<vec2> &texture)
{

  const T s = 2 * M_PI / (float) slices;
  const T t = height / (float) stacks;

  const T theta_0 = -M_PI;
  const T h_0 = -height;

  auto const fslices = (float) slices;
  auto const fstacks = (float) stacks;

  for (unsigned int slice = 1; slice <= slices; ++slice)
    {
      for (unsigned int stack = 1; stack <= stacks; ++stack)
        {
          auto const fslice = (float) slice;
          auto const fstack = (float) stack;

          //base
          vertices.emplace_back (0, 0, 0); //O
          texture.emplace_back (0, 0);
          normals.emplace_back (0, -1, 0);
          for (auto e : {
              -1.0f,//P1
              .0f //P2
          })
            {
              model_cone_vertex (radius, height, theta_0 + s * (fslice + e), h_0, vertices); //P1
              normals.emplace_back (0, -1, 0);
            }

          texture.emplace_back (-1, 0);
          texture.emplace_back (0, 0);

          int q = 0;
          for (auto e : {
              array<float, 2>{0, -1},
              array<float, 2>{-1, 0},
              array<float, 2>{0, 0},

              array<float, 2>{0, -1},
              array<float, 2>{-1, -1},
              array<float, 2>{-1, 0},
          })
            {
              model_cone_vertex (radius, height,
                                 theta_0 + s * (fslice + e[0]),
                                 h_0 + t * (fstack + e[1]), vertices);
              if (q % 3 == 2)
                {
                  const auto P1 = vertices.end ()[-2];
                  const auto P2 = vertices.end ()[-3];
                  const auto P1_prime = vertices.end ()[-1];
                  for (auto _ = 0; _ < 3; ++_)
                    normals.emplace_back (normalize (cross (P2 - P1_prime, P1 - P1_prime)));
                }

              texture.emplace_back (fslice / fslices, fstack / fstacks);
              ++q;
            }
        }
    }
}

static inline unsigned int model_cone_nVertices (const unsigned int stacks, const unsigned int slices)
{
  return slices * stacks * 9;
}

void model_cone_write (const char *const filepath,
                       const float radius,
                       const float height,
                       const unsigned int slices,
                       const unsigned int stacks)
{
  const unsigned int nVertices = model_cone_nVertices (stacks, slices);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);
  model_cone_vertices (radius, height, slices, stacks, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
}

//!@} end of group cone

/*! @addtogroup sphere
* @{*/

static inline unsigned int model_sphere_nVertices (const unsigned int slices, const unsigned int stacks)
{
  return slices * stacks * 6;
}

static inline void
model_sphere_vertex (const float r,
                     const float theta,
                     const float phi,
                     vector<vec3> &vertices,
                     vector<vec3> &normals)
{
  /*
      x = r ⋅ sin(θ)cos(φ)
      y = r ⋅ sin(φ)
      z = r ⋅ cos(θ)cos(φ)

      r ≥ 0
      θ ∈ {-π +   i⋅s : s = 2π/slices ∧ i ∈ {0,...,slices} }
      ϕ ∈ {-π/2 + j⋅t : t =  π/stacks ∧ j ∈ {0,...,stacks} }

      check
          1. https://www.math3d.org/EumEEZBKe
          2. https://www.math3d.org/zE4n6xayX
   */
  vertices.emplace_back (r * sin (theta) * cos (phi), r * sin (phi), r * cos (theta) * cos (phi));
  normals.emplace_back (sin (theta) * cos (phi), sin (phi), cos (theta) * cos (phi));
}

static void model_sphere_vertices (const float r,
                                   const unsigned int slices,
                                   const unsigned int stacks,
                                   vector<vec3> &vertices,
                                   vector<vec3> &normals,
                                   vector<vec2> &texture)
{
  // https://www.math3d.org/EumEEZBKe
  // https://www.math3d.org/zE4n6xayX

  const float s = 2.0f * (float) M_PI / (float) slices;
  const float t = M_PI / (float) stacks;
  const float theta = -M_PI;
  const float phi = -M_PI / 2.0f;

  auto fslices = (float) slices;
  auto fstacks = (float) stacks;

  for (unsigned int slice = 1; slice <= slices; ++slice)
    {
      for (unsigned int stack = 1; stack <= stacks; ++stack)
        {
          auto fslice = (float) slice;
          auto fstack = (float) stack;

          texture.emplace_back ((fslice - 1) / fslices, fstack / fstacks); // P1'
          texture.emplace_back (fslice / fslices, (fstack - 1) / fstacks); // P2
          texture.emplace_back (fslice / fslices, fstack / fstacks); // P2'

          texture.emplace_back ((fslice - 1) / fslices, (fstack - 1) / fstacks); // P1
          texture.emplace_back (fslice / fslices, (fstack - 1) / fstacks); // P2
          texture.emplace_back ((fslice - 1) / fslices, fstack / fstacks); // P1'

          model_sphere_vertex (r, theta + s * (fslice - 1), phi + t * fstack, vertices, normals); // P1'
          model_sphere_vertex (r, theta + s * fslice, phi + t * (fstack - 1), vertices, normals); // P2
          model_sphere_vertex (r, theta + s * fslice, phi + t * fstack, vertices, normals); // P2'

          model_sphere_vertex (r, theta + s * (fslice - 1), phi + t * (fstack - 1), vertices, normals); // P1
          model_sphere_vertex (r, theta + s * fslice, phi + t * (fstack - 1), vertices, normals); // P2
          model_sphere_vertex (r, theta + s * (fslice - 1), phi + t * fstack, vertices, normals); // P1'
        }
    }
}

void model_sphere_write (const char *const filepath,
                         const float radius,
                         const unsigned int slices,
                         const unsigned int stacks)
{

  const unsigned int nVertices = model_sphere_nVertices (slices, stacks);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);
  model_sphere_vertices (radius, slices, stacks, vertices, normals, texture);
  model_write (filepath, vertices, normals, texture);
}
//!@} end of group sphere

//!@} end of group model

/*! @addtogroup bezier
 * @{ */
vector<array<vec3, 16>> read_Bezier (const char *const patch)
{
  string buffer;
  ifstream myFile;

  myFile.open (patch, ios::in | ios::out);
  getline (myFile, buffer);
  // Número de patches presentes no ficheiro.
  const int n_patches = stoi (buffer);

  // Vetor de vetores de índices.
  vector<vector<int>> patches;

  // Ciclo externo lê uma linha (patch) de cada vez
  for (int j = 0; j < n_patches; j++)
    {
      vector<int> patchIndexes;
      /*
      Ciclo interno lê os índices dos pontos de controlo de cada patch, sabendo que cada
      patch terá 16 pontos de controlo.
      */
      for (int i = 0; i < 15; i++)
        {
          getline (myFile, buffer, ',');
          patchIndexes.push_back (stoi (buffer));
        }
      getline (myFile, buffer);
      patchIndexes.push_back (stoi (buffer));
      patches.push_back (patchIndexes);
    }

  getline (myFile, buffer);
  // Número de pontos presentes no ficheiro.
  const int pts = stoi (buffer);

  // Vetor que guardará as coordenadas de pontos de controlo para superfície de Bézier.
  vector<vec3> control;
  for (int j = 0; j < pts; j++)
    {
      vec3 v;
      getline (myFile, buffer, ',');
      v[0] = stof (buffer);
      getline (myFile, buffer, ',');
      v[1] = stof (buffer);
      getline (myFile, buffer);
      v[2] = stof (buffer);
      control.push_back (v);
    }

  /*
  Percorrem-se os vetores que, para cada patch, guardam os seus índices de pontos de controlo.
  Para cada patch, constroi-se um vetor com as coordenadas dos seus pontos de controlo.
  */
  vector<array<vec3, 16>> pointsInPatches;
  for (auto &patche : patches)
    {
      array<vec3, 16> pointsInPatch{};
      for (int j = 0; j < 16; j++)
        {
          pointsInPatch[j] = control[patche[j]];
        }
      pointsInPatches.push_back (pointsInPatch);
    }

  myFile.close ();

  /*std::cout << "read_Bezier read:" << std::endl;
  for (auto arr : pointsInPatches)
    for (auto p : arr)
      std::cout << glm::to_string (p) << std::endl;*/
  return pointsInPatches;
}

template<typename T> static inline vec4 monic_3rd_polynomial_at (T n)
{
  return {pow (n, 3), pow (n, 2), n, 1};
}

template<typename T> static inline vec4 deriv_of_monic_3rd_polynomial_at (T n)
{
  return {3 * pow (n, 2), 2 * n, 1, 0};
}

mat4x3 mat (const array<vec3, 4> C)
{
  return {C[0], C[1], C[2], C[3]};
}

/*!
 *
 * @param[in] u first component of the 2d point.
 * @param[in] v second component of the 2d point.
 * @param[in] cp a set of control points that define the bezier patch.
 * @param[out] coordinate_in_3d_space cartesian three-dimensional coordinate of the
 *                                    point specified by Beziér patch coordinate.
 * @param[out] normal vector normal to the patch specified.
 */
void get_bezier_point_at (
    const float u,
    const float v,
    const array<vec3, 16> &cp,
    vec3 &coordinate_in_3d_space,
    vec3 &normal)
{

  // P_u notation at (slide 17)[Curves and Surfaces]

  // P(u,v) = UM(Pi0(P0u) + Pi1(P1u) + Pi2(P2u) Pi3(P3u)) based on (page 6)[CURVES AND SURFACES]

  // 4 sets of control points for 4 Bézier curves
  array<vec3, 4> C_i0 = {cp[0], cp[1], cp[2], cp[3]};
  array<vec3, 4> C_i1 = {cp[4], cp[5], cp[6], cp[7]};
  array<vec3, 4> C_i2 = {cp[8], cp[9], cp[10], cp[11]};
  array<vec3, 4> C_i3 = {cp[12], cp[13], cp[14], cp[15]};

  // calculate P_i(u) at each Bézier curve defined by Ci
  vec3 P0u, P1u, P2u, P3u, _;
  get_curve_point_at (u, Mb, C_i0, P0u, _);
  get_curve_point_at (u, Mb, C_i1, P1u, _);
  get_curve_point_at (u, Mb, C_i2, P2u, _);
  get_curve_point_at (u, Mb, C_i3, P3u, _);

  // Calculate vector tangent to u⃗
  const auto VM = monic_3rd_polynomial_at (v) * Mb;
  const auto U_prime = deriv_of_monic_3rd_polynomial_at (u);
  const auto tangent_u =
      (mat (C_i0) * VM[0] + mat (C_i1) * VM[1] + mat (C_i2) * VM[2] + mat (C_i3) * VM[3]) * Mb * U_prime;

  vec3 Puv, tangent_v;
  get_curve_point_at (v, Mb, {P0u, P1u, P2u, P3u}, Puv, tangent_v);

  normal = normalize (cross (tangent_u, tangent_v));
  coordinate_in_3d_space = Puv;
}

static inline unsigned int model_bezier_patch_nVertices (const unsigned int tesselation)
{
  return tesselation * tesselation * 6;
}

static inline unsigned int model_bezier_surface_nVertices (
    const unsigned int number_of_patches,
    const unsigned int tesselation)
{
  return model_bezier_patch_nVertices (tesselation) * number_of_patches;
}

void get_bezier_patch (
    const array<vec3, 16> control_points,
    const int int_tesselation,
    vector<vec3> &vertices,
    vector<vec3> &normals,
    vector<vec2> &texture)
{
  auto float_tesselation = (float) int_tesselation;
  const auto step = 1.0 / float_tesselation;
  for (int v = 0; v < int_tesselation; ++v)
    {
      for (int u = 0; u < int_tesselation; ++u)
        {
          for (auto e : {
              // upper triangle
              array<float, 2>{0, 1},
              array<float, 2>{0, 0},
              array<float, 2>{1, 0},
              // lower triangle
              array<float, 2>{1, 0},
              array<float, 2>{1, 1},
              array<float, 2>{0, 1},
          })
            {
              vec3 vertex, normal;
              get_bezier_point_at (u * step + step * e[0], v * step + step * e[1], control_points, vertex, normal);

              auto fu = (float) u;
              auto fv = (float) v;
              vertices.push_back (vertex);
              normals.push_back (normal);
              texture.emplace_back (-(u * step + step * e[0]), -(v * step + step * e[1]));
            }

          //          // upper triangle
          //          vertices.push_back (get_bezier_point_at (u * step, v * step, control_points));
          //          vertices.push_back (get_bezier_point_at (u * step, v * step + step, control_points));
          //          vertices.push_back (get_bezier_point_at (u * step + step, v * step, control_points));
          //          // lower triangle
          //          vertices.push_back (get_bezier_point_at (u * step + step, v * step, control_points));
          //          vertices.push_back (get_bezier_point_at (u * step + step, v * step + step, control_points));
          //          vertices.push_back (get_bezier_point_at (u * step, v * step + step, control_points));
        }
    }
  /*std::cout << "get_bezier_patch got:" << std::endl;
  for (auto p : pontos)
    std::cout << glm::to_string (p) << std::endl;*/
}

/*!
 *
 * @param control_elements 4 vertices define a bezier curve and 4 bezier curves define a bezier patch.
 *                         A set of bezier patches define a bezier surface.
 *                         Therefore, each control element of a bezier surface requires 16 vertices.
 */
void get_bezier_surface (
    const vector<array<vec3, 16>> &control_elements,
    const int tesselation,
    vector<vec3> &vertices,
    vector<vec3> &normals,
    vector<vec2> &texture)
{

  for (auto &control_element : control_elements)
    get_bezier_patch (control_element, tesselation, vertices, normals, texture);
}

void model_bezier_write (
    const int tesselation,
    const char *const in_patch_file,
    const char *const out_3d_file)
{
  vector<array<vec3, 16>> control_points = read_Bezier (in_patch_file);

  const unsigned int nVertices = model_bezier_surface_nVertices (control_points.size (), tesselation);
  vector<vec3> vertices;
  vertices.reserve (nVertices);
  vector<vec3> normals;
  normals.reserve (nVertices);
  vector<vec2> texture;
  texture.reserve (nVertices);

  get_bezier_surface (control_points, tesselation, vertices, normals, texture);
  if (nVertices != vertices.size ())
    {
      cerr << nVertices << " = nVertices != vertices.size () = " << vertices.size () << endl;
      exit (EXIT_FAILURE);
    }
  //assert (nVertices == vertices.size ());

  model_write (out_3d_file, vertices, normals, texture);
}
//!@} end of group bezier

//!@} end of group generator

/*!
 * ⟨command⟩ ::= (⟨plane⟩ | ⟨cube⟩ | ⟨sphere⟩ | ⟨cone⟩ | ⟨patch⟩) ⟨out_file⟩
 * ⟨patch⟩ ::= "bezier" ⟨patch_file⟩ ⟨tesselation⟩
 * ⟨plane⟩ ::= "plane" ⟨length⟩ ⟨divisions⟩
 * ⟨cube⟩ ::= "box" ⟨length⟩ ⟨divisions⟩
 * ⟨cone⟩ ::= "cone" ⟨base_radius⟩ ⟨height⟩ ⟨slices⟩ ⟨stacks⟩
 * ⟨sphere⟩ ::= "sphere" ⟨radius⟩ ⟨slices⟩ ⟨stacks⟩
 */
int main (const int argc, const char *const argv[])
{
  if (argc < 4)
    {
      cerr << "[generator] Not enough arguments" << endl;
      exit (EXIT_FAILURE);
    }
  else
    {
      const char *const out_file_path = argv[argc - 1];
      cerr << "[generator] output filepath: '" << out_file_path << "'" << endl;
      const char *const polygon = argv[1];
      cerr << "[generator] polygon to generate: " << polygon << endl;

      if (!strcmp (PLANE, polygon))
        {
          const float length = strtof (argv[2], nullptr);
          if (length <= 0.0)
            {
              cerr << "[generator] invalid length(" << length << ") for plane" << endl;
              exit (EXIT_FAILURE);
            }
          const int divisions = std::stoi (argv[3], nullptr, 10);
          if (divisions <= 0)
            {
              cerr << "[generator] invalid number of divisions(" << divisions << ") for plane" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "[generator] PLANE(length: " << length << ", divisions: " << divisions << ")" << endl;
          model_plane_write (out_file_path, length, divisions);
        }

      else if (!strcmp (CUBE, polygon))
        {
          const float length = strtof (argv[2], nullptr);
          if (length <= 0.0)
            {
              cerr << "[generator] invalid length(" << length << ") for cube" << endl;
              exit (EXIT_FAILURE);
            }
          const int divisions = std::stoi (argv[3], nullptr, 10);
          if (divisions <= 0)
            {
              cerr << "[generator] invalid number of divisions(" << divisions << ") for cube" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "[generator] CUBE(length: " << length << ", divisions: " << divisions << ")" << endl;
          model_cube_write (out_file_path, length, divisions);
        }
      else if (!strcmp (CONE, polygon))
        {
          const float radius = strtof (argv[2], nullptr);
          if (radius <= 0.0)
            {
              cerr << "[generator] invalid radius(" << radius << ") for cone" << endl;
              exit (EXIT_FAILURE);
            }
          const float height = strtof (argv[3], nullptr);
          if (height <= 0.0)
            {
              cerr << "[generator] invalid height(" << radius << ") for cone" << endl;
              exit (EXIT_FAILURE);
            }
          const int slices = std::stoi (argv[4], nullptr, 10);
          if (slices <= 0)
            {
              cerr << "[generator] invalid slices(" << slices << ") for cone" << endl;
              exit (EXIT_FAILURE);
            }
          const int stacks = std::stoi (argv[5], nullptr, 10);
          if (stacks <= 0)
            {
              cerr << "[generator] invalid stacks(" << stacks << ") for cone" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "[generator] CONE(radius: " << radius
               << ", height: " << height
               << ", slices: " << slices
               << ", stacks: " << stacks << ")" << endl;
          model_cone_write (out_file_path, radius, height, slices, stacks);
        }
      else if (!strcmp (SPHERE, polygon))
        {
          const float radius = strtof (argv[2], nullptr);
          if (radius <= 0.0)
            {
              cerr << "[generator] invalid radius(" << radius << ") for sphere" << endl;
              exit (EXIT_FAILURE);
            }
          const int slices = std::stoi (argv[3], nullptr, 10);
          if (slices <= 0)
            {
              cerr << "[generator] invalid slices(" << slices << ") for sphere" << endl;
              exit (EXIT_FAILURE);
            }
          const int stacks = std::stoi (argv[4], nullptr, 10);
          if (stacks <= 0)
            {
              cerr << "[generator] invalid stacks(" << stacks << ") for sphere" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "[generator] SPHERE(radius: " << radius
               << ", slices: " << slices
               << ", stacks: " << stacks << ")"
               << endl;
          model_sphere_write (out_file_path, radius, slices, stacks);
        }
      else if (!strcmp (BEZIER, polygon))
        {
          const int tesselation = std::stoi (argv[3], nullptr, 10);
          if (tesselation <= 0)
            {
              cerr << "[generator] invalid tesselation(" << tesselation << ") for bezier patch" << endl;
              exit (EXIT_FAILURE);
            }
          const char *const input_patch_file_path = argv[2];
          if (access (input_patch_file_path, F_OK))
            {
              cerr << "[generator] file " << input_patch_file_path << " for bezier patch not found" << endl;
              exit (EXIT_FAILURE);
            }
          cerr << "BEZIER(tesselation: " << tesselation << ", input file: " << input_patch_file_path << ")" << endl;
          model_bezier_write (tesselation, input_patch_file_path, out_file_path);
        }
      else
        {
          cerr << "[generator] Unkown object type: " << polygon << endl;
          exit (EXIT_FAILURE);
        }
    }
  return 0;
}