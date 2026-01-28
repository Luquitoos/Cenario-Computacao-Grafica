#ifndef MESH_H
#define MESH_H

#include "../cenario/hittable.h"
#include "../cenario/hittable_list.h"
#include "../vectors/vec3.h"
#include "triangle.h"
#include <memory>
#include <vector>

class box_mesh : public hittable {
public:
  point3 min_corner;
  point3 max_corner;
  hittable_list faces;
  std::shared_ptr<material> mat;
  std::string name;

  box_mesh() {}

  box_mesh(const point3 &p0, const point3 &p1, std::shared_ptr<material> m,
           const std::string &obj_name = "Box")
      : min_corner(p0), max_corner(p1), mat(m), name(obj_name) {
    build_faces();
  }

  static box_mesh from_center(const point3 &center, double width, double height,
                              double depth, std::shared_ptr<material> m,
                              const std::string &obj_name = "Box") {
    point3 half(width / 2, height / 2, depth / 2);
    return box_mesh(center - half, center + half, m, obj_name);
  }

  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    if (faces.hit(r, t_min, t_max, rec)) {
      rec.object_name = name;
      return true;
    }
    return false;
  }

  std::string get_name() const override { return name; }

private:
  void build_faces() {
    point3 p0 = min_corner;
    point3 p1 = max_corner;

    point3 v[8] = {
        point3(p0.x(), p0.y(), p0.z()), point3(p1.x(), p0.y(), p0.z()),
        point3(p1.x(), p1.y(), p0.z()), point3(p0.x(), p1.y(), p0.z()),
        point3(p0.x(), p0.y(), p1.z()), point3(p1.x(), p0.y(), p1.z()),
        point3(p1.x(), p1.y(), p1.z()), point3(p0.x(), p1.y(), p1.z()),
    };

    faces.add(
        std::make_shared<triangle>(v[0], v[1], v[2], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[0], v[2], v[3], mat, name + "_face"));

    faces.add(
        std::make_shared<triangle>(v[5], v[4], v[7], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[5], v[7], v[6], mat, name + "_face"));

    faces.add(
        std::make_shared<triangle>(v[4], v[0], v[3], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[4], v[3], v[7], mat, name + "_face"));

    faces.add(
        std::make_shared<triangle>(v[1], v[5], v[6], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[1], v[6], v[2], mat, name + "_face"));

    faces.add(
        std::make_shared<triangle>(v[3], v[2], v[6], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[3], v[6], v[7], mat, name + "_face"));

    faces.add(
        std::make_shared<triangle>(v[4], v[5], v[1], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[4], v[1], v[0], mat, name + "_face"));
  }

public:
  bool bounding_box(aabb &output_box) const override {
    output_box = aabb(min_corner, max_corner);
    return true;
  }
};

class blade_mesh : public hittable {
public:
  hittable_list faces;
  std::shared_ptr<material> mat;
  std::string name;

  blade_mesh() {}

  blade_mesh(const point3 &base_center, const point3 &tip, double width,
             double thickness, std::shared_ptr<material> m,
             const std::string &obj_name = "Blade", double taper_start = 0.75)
      : mat(m), name(obj_name) {

    vec3 blade_vec = tip - base_center;
    double blade_length = blade_vec.length();
    vec3 dir = unit_vector(blade_vec);

    vec3 up(0, 1, 0);
    if (std::abs(dot(dir, up)) > 0.9) {
      up = vec3(1, 0, 0);
    }
    vec3 right = unit_vector(cross(dir, up));
    vec3 forward = unit_vector(cross(right, dir));

    point3 p_base = base_center;
    point3 p_taper = base_center + dir * (blade_length * taper_start);
    point3 p_tip = tip;

    double w_base = width;
    double w_taper = width * 0.9;
    double w_tip = width * 0.05;

    double t_base = thickness;
    double t_taper = thickness * 0.85;
    double t_tip = thickness * 0.15;

    point3 b0 = p_base - right * (w_base / 2) - forward * (t_base / 2);
    point3 b1 = p_base + right * (w_base / 2) - forward * (t_base / 2);
    point3 b2 = p_base + right * (w_base / 2) + forward * (t_base / 2);
    point3 b3 = p_base - right * (w_base / 2) + forward * (t_base / 2);

    point3 m0 = p_taper - right * (w_taper / 2) - forward * (t_taper / 2);
    point3 m1 = p_taper + right * (w_taper / 2) - forward * (t_taper / 2);
    point3 m2 = p_taper + right * (w_taper / 2) + forward * (t_taper / 2);
    point3 m3 = p_taper - right * (w_taper / 2) + forward * (t_taper / 2);

    point3 t0 = p_tip - right * (w_tip / 2) - forward * (t_tip / 2);
    point3 t1 = p_tip + right * (w_tip / 2) - forward * (t_tip / 2);
    point3 t2 = p_tip + right * (w_tip / 2) + forward * (t_tip / 2);
    point3 t3 = p_tip - right * (w_tip / 2) + forward * (t_tip / 2);

    faces.add(std::make_shared<triangle>(b0, b1, m1, mat, name));
    faces.add(std::make_shared<triangle>(b0, m1, m0, mat, name));

    faces.add(std::make_shared<triangle>(b3, m3, m2, mat, name));
    faces.add(std::make_shared<triangle>(b3, m2, b2, mat, name));

    faces.add(std::make_shared<triangle>(b1, b2, m2, mat, name));
    faces.add(std::make_shared<triangle>(b1, m2, m1, mat, name));

    faces.add(std::make_shared<triangle>(b3, b0, m0, mat, name));
    faces.add(std::make_shared<triangle>(b3, m0, m3, mat, name));

    faces.add(std::make_shared<triangle>(m0, m1, t1, mat, name));
    faces.add(std::make_shared<triangle>(m0, t1, t0, mat, name));

    faces.add(std::make_shared<triangle>(m3, t3, t2, mat, name));
    faces.add(std::make_shared<triangle>(m3, t2, m2, mat, name));

    faces.add(std::make_shared<triangle>(m1, m2, t2, mat, name));
    faces.add(std::make_shared<triangle>(m1, t2, t1, mat, name));

    faces.add(std::make_shared<triangle>(m3, m0, t0, mat, name));
    faces.add(std::make_shared<triangle>(m3, t0, t3, mat, name));

    faces.add(std::make_shared<triangle>(b0, b3, b2, mat, name));
    faces.add(std::make_shared<triangle>(b0, b2, b1, mat, name));

    faces.add(std::make_shared<triangle>(t0, t1, t2, mat, name));
    faces.add(std::make_shared<triangle>(t0, t2, t3, mat, name));
  }

  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    if (faces.hit(r, t_min, t_max, rec)) {
      rec.object_name = name;
      return true;
    }
    return false;
  }

  std::string get_name() const override { return name; }

  bool bounding_box(aabb &output_box) const override {
    aabb temp_box;
    bool first = true;
    for (const auto &obj : faces.objects) {
      aabb obj_box;
      if (obj->bounding_box(obj_box)) {
        output_box =
            first ? obj_box : aabb::surrounding_box(output_box, obj_box);
        first = false;
      }
    }
    return !first;
  }
};

#endif
