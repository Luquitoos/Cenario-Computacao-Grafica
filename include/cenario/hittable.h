#ifndef HITTABLE_H
#define HITTABLE_H

#include "../material/material.h"
#include "../ray/ray.h"
#include "../vectors/vec3.h"
#include "aabb.h"
#include <memory>
#include <string>

struct hit_record {
  point3 p;
  vec3 normal;
  std::shared_ptr<material> mat;
  double t;
  double u, v;
  bool front_face;
  std::string object_name;

  void set_face_normal(const ray &r, const vec3 &outward_normal) {
    front_face = dot(r.direction(), outward_normal) < 0;
    normal = front_face ? outward_normal : -outward_normal;
  }
};

class hittable {
public:
  virtual ~hittable() = default;

  virtual bool hit(const ray &r, double t_min, double t_max,
                   hit_record &rec) const = 0;
  virtual std::string get_name() const = 0;

  virtual bool bounding_box(aabb &output_box) const = 0;
};

#endif
