#ifndef CAMERA_H
#define CAMERA_H

#include "../ray/ray.h"
#include "../vectors/mat4.h"
#include "../vectors/vec3.h"
#include "../vectors/vec4.h"
#include <cmath>

enum class ProjectionType { PERSPECTIVE, ORTHOGRAPHIC, OBLIQUE };

class camera {
public:
  point3 eye;
  point3 at;
  vec3 up;

  double focal_distance;
  double xmin, xmax;
  double ymin, ymax;

  ProjectionType projection;

  double oblique_angle;
  double oblique_strength;

  vec3 w, u, v;
  mat4 Mwc;
  mat4 Mcw;

  camera() {
    setup(point3(0, 0, 5), point3(0, 0, 0), vec3(0, 1, 0), 1.0, -1, 1, -1, 1,
          ProjectionType::PERSPECTIVE);
  }

  camera(const point3 &lookfrom, const point3 &lookat, const vec3 &vup,
         double d, double x0, double x1, double y0, double y1,
         ProjectionType proj = ProjectionType::PERSPECTIVE) {
    setup(lookfrom, lookat, vup, d, x0, x1, y0, y1, proj);
  }

  void setup(const point3 &lookfrom, const point3 &lookat, const vec3 &vup,
             double d, double x0, double x1, double y0, double y1,
             ProjectionType proj = ProjectionType::PERSPECTIVE) {
    eye = lookfrom;
    at = lookat;
    up = vup;
    focal_distance = d;
    xmin = x0;
    xmax = x1;
    ymin = y0;
    ymax = y1;
    projection = proj;
    oblique_angle = 0.5;
    oblique_strength = 0.5;

    compute_basis();
    compute_matrices();
  }

  void compute_basis() {
    w = unit_vector(eye - at);
    u = unit_vector(cross(up, w));
    v = cross(w, u);
  }

  void compute_matrices() {

    Mwc = mat4(u.x(), u.y(), u.z(), -dot(u, eye), v.x(), v.y(), v.z(),
               -dot(v, eye), w.x(), w.y(), w.z(), -dot(w, eye), 0, 0, 0, 1);
    Mcw = mat4(u.x(), v.x(), w.x(), eye.x(), u.y(), v.y(), w.y(), eye.y(),
               u.z(), v.z(), w.z(), eye.z(), 0, 0, 0, 1);
  }

  ray get_ray(double s, double t) const {

    double x = xmin + s * (xmax - xmin);
    double y = ymin + t * (ymax - ymin);

    switch (projection) {
    case ProjectionType::PERSPECTIVE:
      return get_perspective_ray(x, y);
    case ProjectionType::ORTHOGRAPHIC:
      return get_orthographic_ray(x, y);
    case ProjectionType::OBLIQUE:
      return get_oblique_ray(x, y);
    default:
      return get_perspective_ray(x, y);
    }
  }

  ray get_perspective_ray(double x, double y) const {

    point3 pixel_pos = eye - focal_distance * w + x * u + y * v;
    vec3 direction = pixel_pos - eye;
    return ray(eye, direction);
  }

  ray get_orthographic_ray(double x, double y) const {

    point3 origin = eye + x * u + y * v;

    vec3 direction = -w;
    return ray(origin, direction);
  }

  ray get_oblique_ray(double x, double y) const {

    point3 origin = eye + x * u + y * v;

    double shear_x = oblique_strength * std::cos(oblique_angle);
    double shear_y = oblique_strength * std::sin(oblique_angle);
    vec3 direction = unit_vector(-w + shear_x * u + shear_y * v);
    return ray(origin, direction);
  }

  void zoom_in(double factor = 0.8) {

    double cx = (xmin + xmax) / 2;
    double cy = (ymin + ymax) / 2;
    double hw = (xmax - xmin) / 2 * factor;
    double hh = (ymax - ymin) / 2 * factor;
    xmin = cx - hw;
    xmax = cx + hw;
    ymin = cy - hh;
    ymax = cy + hh;
  }

  void zoom_out(double factor = 1.25) { zoom_in(factor); }

  void set_focal_distance(double d) { focal_distance = d; }

  void move_closer(double amount) {
    vec3 dir = unit_vector(at - eye);
    eye = eye + dir * amount;
    compute_basis();
    compute_matrices();
  }

  void move_away(double amount) { move_closer(-amount); }

  void setup_one_vanishing_point(const point3 &target, double distance) {

    at = target;
    eye = target + vec3(0, 0, distance);
    up = vec3(0, 1, 0);
    compute_basis();
    compute_matrices();
  }

  void setup_two_vanishing_points(const point3 &target, double distance,
                                  double angle_y) {
    at = target;
    double x = distance * std::sin(angle_y);
    double z = distance * std::cos(angle_y);
    eye = target + vec3(x, 0, z);
    up = vec3(0, 1, 0);
    compute_basis();
    compute_matrices();
  }

  void setup_three_vanishing_points(const point3 &target, double distance,
                                    double angle_y, double angle_elevation) {
    at = target;
    double y = distance * std::sin(angle_elevation);
    double horiz_dist = distance * std::cos(angle_elevation);
    double x = horiz_dist * std::sin(angle_y);
    double z = horiz_dist * std::cos(angle_y);
    eye = target + vec3(x, y, z);
    up = vec3(0, 1, 0);
    compute_basis();
    compute_matrices();
  }

  point3 world_to_camera(const point3 &p) const {
    vec4 p4(p, 1.0);
    vec4 result = Mwc * p4;
    return result.to_point3();
  }

  vec3 world_to_camera_dir(const vec3 &dir) const {

    return vec3(dot(u, dir), dot(v, dir), dot(w, dir));
  }
};

#endif
