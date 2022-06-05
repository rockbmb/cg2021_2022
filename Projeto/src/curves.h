#ifndef _CURVES_H_
#define _CURVES_H_
#include <array>
#include <vector>
#include <glm/glm.hpp>

extern const glm::mat4 Mcr, Mb;
void renderCurve (glm::mat4 M, const std::vector<glm::vec3> &control_points, unsigned int tesselation = 100);
void advance_in_curve (float translation_time, bool align, const glm::mat4 &M, const std::vector<glm::vec3> &global_control_points);
void get_curve_point_at (
    float t,
    const glm::mat4 &M,
    const std::array<glm::vec3, 4> &control_points,
    glm::vec3 &coordinate_in_3d_space,
    glm::vec3 &derivative_at_t);
#endif //_CURVES_H_
