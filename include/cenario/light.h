#ifndef LIGHT_H
#define LIGHT_H

#include "../colors/color.h"
#include "../vectors/vec3.h"
#include <cmath>

class light {
public:
  color intensity;

  light() : intensity(1, 1, 1) {}
  light(const color &i) : intensity(i) {}
  virtual ~light() = default;
  virtual vec3 get_direction(const point3 &point) const = 0;
  virtual double get_distance(const point3 &point) const = 0;
  virtual color get_intensity(const point3 &point) const = 0;
  virtual point3 get_position() const = 0;
};

class ambient_light {
public:
  color intensity;

  ambient_light() : intensity(0.1, 0.1, 0.1) {}
  ambient_light(const color &i) : intensity(i) {}
  ambient_light(double r, double g, double b) : intensity(r, g, b) {}
};

class point_light : public light {
public:
  point3 position;
  double c1, c2, c3;

  point_light() : position(0, 0, 0), c1(1), c2(0), c3(0) {}

  point_light(const point3 &pos, const color &i, double att_c1 = 1.0,
              double att_c2 = 0.0, double att_c3 = 0.0)
      : light(i), position(pos), c1(att_c1), c2(att_c2), c3(att_c3) {}

  vec3 get_direction(const point3 &point) const override {
    return unit_vector(position - point);
  }

  double get_distance(const point3 &point) const override {
    return (position - point).length();
  }

  color get_intensity(const point3 &point) const override {
    double d = get_distance(point);
    double attenuation = 1.0 / (c1 + c2 * d + c3 * d * d);
    return intensity * attenuation;
  }

  point3 get_position() const override { return position; }
};

class spot_light : public light {
public:
  point3 position;
  vec3 direction;
  double inner_angle;
  double outer_angle;
  double c1, c2, c3;

  spot_light()
      : position(0, 0, 0), direction(0, -1, 0), inner_angle(0.3),
        outer_angle(0.5), c1(1), c2(0), c3(0) {}

  spot_light(const point3 &pos, const vec3 &dir, const color &i,
             double inner_ang, double outer_ang, double att_c1 = 1.0,
             double att_c2 = 0.0, double att_c3 = 0.0)
      : light(i), position(pos), direction(unit_vector(dir)),
        inner_angle(inner_ang), outer_angle(outer_ang), c1(att_c1), c2(att_c2),
        c3(att_c3) {}

  vec3 get_direction(const point3 &point) const override {
    return unit_vector(position - point);
  }

  double get_distance(const point3 &point) const override {
    return (position - point).length();
  }

  color get_intensity(const point3 &point) const override {
    vec3 L = unit_vector(point - position);
    double cos_angle = dot(L, direction);
    double angle = std::acos(cos_angle);

    if (angle > outer_angle) {
      return color(0, 0, 0);
    }

    double d = get_distance(point);
    double attenuation = 1.0 / (c1 + c2 * d + c3 * d * d);

    if (angle < inner_angle) {
      return intensity * attenuation;
    }

    double falloff = (outer_angle - angle) / (outer_angle - inner_angle);
    return intensity * attenuation * falloff;
  }

  point3 get_position() const override { return position; }
};

class directional_light : public light {
public:
  vec3 direction;

  directional_light() : direction(0, -1, 0) {}

  directional_light(const vec3 &dir, const color &i)
      : light(i), direction(unit_vector(dir)) {}

  vec3 get_direction(const point3 &point) const override { return -direction; }

  double get_distance(const point3 &point) const override { return 1e30; }

  color get_intensity(const point3 &point) const override { return intensity; }

  point3 get_position() const override {
    return point3(0, 0, 0) - direction * 1e10;
  }
};

#endif
