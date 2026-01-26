#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "../cenario/hittable.h"
#include "../vectors/mat4.h"
#include "../vectors/vec4.h"
#include "quaternion.h"
#include <memory>

class transform : public hittable {
public:
  std::shared_ptr<hittable> object;
  mat4 forward;
  mat4 inverse;
  mat4 normal_mat;
  std::string name;

  transform() {}

  transform(std::shared_ptr<hittable> obj, const mat4 &fwd, const mat4 &inv)
      : object(obj), forward(fwd), inverse(inv) {
    normal_mat = inv.transpose();
    name = obj->get_name();
  }

  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {

    vec4 origin4(r.origin(), 1.0);
    vec4 dir4(r.direction(), 0.0);

    vec4 local_origin = inverse * origin4;
    vec4 local_dir = inverse * dir4;

    ray local_ray(local_origin.to_point3(), local_dir.to_vec3());

    if (!object->hit(local_ray, t_min, t_max, rec)) {
      return false;
    }

    vec4 world_p = forward * vec4(rec.p, 1.0);
    rec.p = world_p.to_point3();

    vec4 normal4 = normal_mat * vec4(rec.normal, 0.0);
    rec.normal = unit_vector(normal4.to_vec3());

    rec.object_name = name;
    return true;
  }

  std::string get_name() const override { return name; }
};

inline std::shared_ptr<transform>
translate_object(std::shared_ptr<hittable> obj, double tx, double ty,
                 double tz) {
  mat4 fwd = mat4::translate(tx, ty, tz);
  mat4 inv = mat4::translate_inverse(tx, ty, tz);
  return std::make_shared<transform>(obj, fwd, inv);
}

inline std::shared_ptr<transform> rotate_x_object(std::shared_ptr<hittable> obj,
                                                  double angle_rad) {
  mat4 fwd = mat4::rotate_x(angle_rad);
  mat4 inv = mat4::rotate_x_inverse(angle_rad);
  return std::make_shared<transform>(obj, fwd, inv);
}

inline std::shared_ptr<transform> rotate_y_object(std::shared_ptr<hittable> obj,
                                                  double angle_rad) {
  mat4 fwd = mat4::rotate_y(angle_rad);
  mat4 inv = mat4::rotate_y_inverse(angle_rad);
  return std::make_shared<transform>(obj, fwd, inv);
}

inline std::shared_ptr<transform> rotate_z_object(std::shared_ptr<hittable> obj,
                                                  double angle_rad) {
  mat4 fwd = mat4::rotate_z(angle_rad);
  mat4 inv = mat4::rotate_z_inverse(angle_rad);
  return std::make_shared<transform>(obj, fwd, inv);
}

inline std::shared_ptr<transform>
rotate_axis_object(std::shared_ptr<hittable> obj, const vec3 &axis,
                   double angle_rad) {
  mat4 fwd = rotate_axis(axis, angle_rad);
  mat4 inv = rotate_axis_inverse(axis, angle_rad);
  return std::make_shared<transform>(obj, fwd, inv);
}

inline std::shared_ptr<transform>
scale_object(std::shared_ptr<hittable> obj, double sx, double sy, double sz) {
  mat4 fwd = mat4::scale(sx, sy, sz);
  mat4 inv = mat4::scale_inverse(sx, sy, sz);
  return std::make_shared<transform>(obj, fwd, inv);
}

inline std::shared_ptr<transform> shear_object(std::shared_ptr<hittable> obj,
                                               double xy, double xz, double yx,
                                               double yz, double zx,
                                               double zy) {
  mat4 fwd = mat4::shear(xy, xz, yx, yz, zx, zy);
  mat4 inv = mat4::shear_inverse(xy, xz, yx, yz, zx, zy);
  return std::make_shared<transform>(obj, fwd, inv);
}

inline std::shared_ptr<transform> reflect_object(std::shared_ptr<hittable> obj,
                                                 const vec3 &plane_point,
                                                 const vec3 &plane_normal) {
  mat4 fwd = mat4::reflect_plane(plane_point, plane_normal);
  mat4 inv = fwd;
  return std::make_shared<transform>(obj, fwd, inv);
}

inline std::shared_ptr<transform>
compose_transform(std::shared_ptr<hittable> obj, const vec3 &translation,
                  const vec3 &rotation_axis, double rotation_angle,
                  const vec3 &scale_factors) {

  mat4 S = mat4::scale(scale_factors.x(), scale_factors.y(), scale_factors.z());
  mat4 R = rotate_axis(rotation_axis, rotation_angle);
  mat4 T = mat4::translate(translation.x(), translation.y(), translation.z());

  mat4 fwd = T * R * S;

  mat4 Sinv = mat4::scale_inverse(scale_factors.x(), scale_factors.y(),
                                  scale_factors.z());
  mat4 Rinv = rotate_axis_inverse(rotation_axis, rotation_angle);
  mat4 Tinv = mat4::translate_inverse(translation.x(), translation.y(),
                                      translation.z());

  mat4 inv = Sinv * Rinv * Tinv;

  return std::make_shared<transform>(obj, fwd, inv);
}

#endif
