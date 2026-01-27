#ifndef CYLINDER_H
#define CYLINDER_H

#include "../cenario/hittable.h"
#include "../vectors/vec3.h"
#include <algorithm>
#include <cmath>

class cylinder : public hittable {
public:
  point3 base_center;
  vec3 axis;
  double radius;
  double height;
  std::shared_ptr<material> mat;
  std::string name;

  cylinder() {}
  cylinder(const point3 &base, const vec3 &ax, double r, double h,
           std::shared_ptr<material> m,
           const std::string &obj_name = "Cylinder")
      : base_center(base), axis(unit_vector(ax)), radius(r), height(h), mat(m),
        name(obj_name) {}

  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    double best_t = t_max + 1;
    vec3 best_normal;
    bool found = false;

    vec3 D = r.direction();
    vec3 L = r.origin() - base_center;

    vec3 D_perp = D - dot(D, axis) * axis;
    vec3 L_perp = L - dot(L, axis) * axis;

    double a = dot(D_perp, D_perp);
    double b = 2.0 * dot(D_perp, L_perp);
    double c = dot(L_perp, L_perp) - radius * radius;

    double discriminant = b * b - 4 * a * c;

    if (discriminant >= 0) {
      double sqrt_d = std::sqrt(discriminant);

      for (int i = 0; i < 2; i++) {
        double t =
            (i == 0) ? (-b - sqrt_d) / (2.0 * a) : (-b + sqrt_d) / (2.0 * a);

        if (t >= t_min && t < best_t) {
          point3 p = r.at(t);
          double h_point = dot(p - base_center, axis);

          if (h_point >= 0 && h_point <= height) {
            best_t = t;

            point3 proj = base_center + h_point * axis;
            best_normal = unit_vector(p - proj);
            found = true;
          }
        }
      }
    }

    double t_base = hit_cap(r, base_center, -axis, t_min, best_t);
    if (t_base >= t_min && t_base < best_t) {
      best_t = t_base;
      best_normal = -axis;
      found = true;
    }

    point3 top_center = base_center + height * axis;
    double t_top = hit_cap(r, top_center, axis, t_min, best_t);
    if (t_top >= t_min && t_top < best_t) {
      best_t = t_top;
      best_normal = axis;
      found = true;
    }

    if (!found || best_t > t_max) {
      return false;
    }

    rec.t = best_t;
    rec.p = r.at(best_t);
    rec.set_face_normal(r, best_normal);
    rec.mat = mat;
    rec.object_name = name;

    vec3 local = rec.p - base_center;
    double h_point = dot(local, axis);
    rec.v = h_point / height;

    vec3 radial = local - h_point * axis;
    rec.u = std::atan2(radial.z(), radial.x()) / (2.0 * 3.14159265358979) + 0.5;

    return true;
  }

  std::string get_name() const override { return name; }

private:
  double hit_cap(const ray &r, const point3 &cap_center, const vec3 &cap_normal,
                 double t_min, double t_max) const {
    double denom = dot(r.direction(), cap_normal);

    if (std::abs(denom) < 1e-8) {
      return t_max + 1;
    }

    double t = dot(cap_center - r.origin(), cap_normal) / denom;

    if (t < t_min || t > t_max) {
      return t_max + 1;
    }

    point3 p = r.at(t);
    vec3 v = p - cap_center;
    double dist_sq = dot(v, v) - std::pow(dot(v, cap_normal), 2);

    if (dist_sq > radius * radius) {
      return t_max + 1;
    }

    return t;
  }

public:
  bool bounding_box(aabb& output_box) const override {
    // Calcula AABB conservadora para cilindro
    point3 top_center = base_center + height * axis;
    
    double min_x = std::fmin(base_center.x() - radius, top_center.x() - radius);
    double min_y = std::fmin(base_center.y() - radius, top_center.y() - radius);
    double min_z = std::fmin(base_center.z() - radius, top_center.z() - radius);
    
    double max_x = std::fmax(base_center.x() + radius, top_center.x() + radius);
    double max_y = std::fmax(base_center.y() + radius, top_center.y() + radius);
    double max_z = std::fmax(base_center.z() + radius, top_center.z() + radius);
    
    output_box = aabb(point3(min_x, min_y, min_z), point3(max_x, max_y, max_z));
    return true;
  }
};

#endif
