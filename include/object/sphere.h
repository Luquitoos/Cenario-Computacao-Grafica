#ifndef SPHERE_H
#define SPHERE_H

#include "../cenario/hittable.h"
#include "../vectors/vec3.h"
#include <cmath>
#include <string>

class sphere : public hittable {
public:
  point3 center;
  double radius;
  std::shared_ptr<material> mat;
  std::string name;

  sphere() {}
  sphere(const point3 &c, double r, std::shared_ptr<material> m,
         const std::string &obj_name = "Sphere")
      : center(c), radius(r), mat(m), name(obj_name) {}

  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {

    vec3 L = r.origin() - center;

    double a = dot(r.direction(), r.direction());
    double b = 2.0 * dot(L, r.direction());
    double c = dot(L, L) - radius * radius;

    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
      return false;
    }

    double sqrt_d = std::sqrt(discriminant);

    double t = (-b - sqrt_d) / (2.0 * a);
    if (t < t_min || t > t_max) {
      t = (-b + sqrt_d) / (2.0 * a);
      if (t < t_min || t > t_max) {
        return false;
      }
    }

    rec.t = t;
    rec.p = r.at(t);
    vec3 outward_normal = (rec.p - center) / radius;
    rec.set_face_normal(r, outward_normal);
    rec.mat = mat;
    rec.object_name = name;

    double theta = std::acos(-outward_normal.y());
    double phi =
        std::atan2(-outward_normal.z(), outward_normal.x()) + 3.14159265358979;
    rec.u = phi / (2.0 * 3.14159265358979);
    rec.v = theta / 3.14159265358979;

    return true;
  }

  std::string get_name() const override { return name; }
};

#endif
