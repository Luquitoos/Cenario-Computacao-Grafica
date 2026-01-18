#ifndef TRIANGLE_H
#define TRIANGLE_H

#include "../cenario/hittable.h"
#include "../vectors/vec3.h"
#include <cmath>

// TRIÂNGULO (base para malhas)
class triangle : public hittable {
public:
  point3 v0, v1, v2; // Vértices
  vec3 normal;       // Normal do triângulo
  std::shared_ptr<material> mat;
  std::string name;

  triangle() {}
  triangle(const point3 &a, const point3 &b, const point3 &c,
           std::shared_ptr<material> m,
           const std::string &obj_name = "Triangle")
      : v0(a), v1(b), v2(c), mat(m), name(obj_name) {
    // Calcular normal (sentido anti-horário)
    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;
    normal = unit_vector(cross(e1, e2));
  }

  // Interseção usando algoritmo de Möller-Trumbore
  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    const double EPSILON = 1e-8;

    vec3 e1 = v1 - v0;
    vec3 e2 = v2 - v0;

    vec3 h = cross(r.direction(), e2);
    double a = dot(e1, h);

    if (std::abs(a) < EPSILON) {
      return false; // Raio paralelo ao triângulo
    }

    double f = 1.0 / a;
    vec3 s = r.origin() - v0;
    double u = f * dot(s, h);

    if (u < 0.0 || u > 1.0) {
      return false;
    }

    vec3 q = cross(s, e1);
    double v = f * dot(r.direction(), q);

    if (v < 0.0 || u + v > 1.0) {
      return false;
    }

    double t = f * dot(e2, q);

    if (t < t_min || t > t_max) {
      return false;
    }

    rec.t = t;
    rec.p = r.at(t);
    rec.set_face_normal(r, normal);
    rec.mat = mat;
    rec.object_name = name;
    rec.u = u;
    rec.v = v;

    return true;
  }

  std::string get_name() const override { return name; }
};

#endif
