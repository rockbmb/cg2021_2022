#ifndef _CURVES_H_
#define _CURVES_H_
#include <array>
#include <vector>
#include <glm/glm.hpp>
using glm::mat4, glm::vec4, glm::vec3, std::array, std::vector;
extern const mat4 Mcr, Mb;
void renderCurve (mat4 M, const vector<vec3> &control_points, unsigned int tesselation = 100);
vector<vec3> get_bezier_surface (const vector<array<vec3, 16>> &control_set, int int_tesselation);
void get_curve (float time, const mat4 &M, const array<vec3, 4> &control_points, glm::vec3 &pos, glm::vec3 &deriv);
void advance_in_curve (float translation_time, bool align, const mat4 &M, const vector<vec3> &global_control_points);
#endif //_CURVES_H_
