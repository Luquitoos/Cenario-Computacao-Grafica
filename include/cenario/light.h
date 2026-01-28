#ifndef LIGHT_H
#define LIGHT_H

#include "../colors/color.h"
#include "../vectors/vec3.h"
#include <cmath>
#include <string>

class light {
public:
  color intensity;
  std::string name;
  bool enabled;

  light(std::string n = "Light") : intensity(1, 1, 1), name(n), enabled(true) {}
  light(const color &i, std::string n = "Light")
      : intensity(i), name(n), enabled(true) {}
  virtual ~light() = default;
  virtual vec3 get_direction(const point3 &point) const = 0;
  virtual double get_distance(const point3 &point) const = 0;
  virtual color get_intensity(const point3 &point) const = 0;
  virtual point3 get_position() const = 0;

  // Setters for GUI
  virtual void set_position(const point3 &p) {}
  virtual void set_direction(const vec3 &d) {}
};

class ambient_light {
public:
  color intensity;
  std::string name;
  bool enabled;

  ambient_light(std::string n = "Ambient")
      : intensity(0.1, 0.1, 0.1), name(n), enabled(true) {}
  ambient_light(const color &i, std::string n = "Ambient")
      : intensity(i), name(n), enabled(true) {}
  ambient_light(double r, double g, double b, std::string n = "Ambient")
      : intensity(r, g, b), name(n), enabled(true) {}
};

class point_light : public light {
public:
  point3 position;
  double c1, c2, c3;

  point_light(std::string n = "Point Light")
      : light(n), position(0, 0, 0), c1(1), c2(0), c3(0) {}

  point_light(const point3 &pos, const color &i, double att_c1 = 1.0,
              double att_c2 = 0.0, double att_c3 = 0.0,
              std::string n = "Point Light")
      : light(i, n), position(pos), c1(att_c1), c2(att_c2), c3(att_c3) {}

  vec3 get_direction(const point3 &point) const override {
    return unit_vector(position - point);
  }

  double get_distance(const point3 &point) const override {
    return (position - point).length();
  }

  color get_intensity(const point3 &point) const override {
    if (!enabled)
      return color(0, 0, 0);
    double d = get_distance(point);
    double attenuation = 1.0 / (c1 + c2 * d + c3 * d * d);
    return intensity * attenuation;
  }

  point3 get_position() const override { return position; }
  void set_position(const point3 &p) override { position = p; }
};

class spot_light : public light {
public:
  point3 position;
  vec3 direction;
  double inner_angle;
  double outer_angle;
  double c1, c2, c3;

  spot_light(std::string n = "Spot Light")
      : light(n), position(0, 0, 0), direction(0, -1, 0), inner_angle(0.3),
        outer_angle(0.5), c1(1), c2(0), c3(0) {}

  spot_light(const point3 &pos, const vec3 &dir, const color &i,
             double inner_ang, double outer_ang, double att_c1 = 1.0,
             double att_c2 = 0.0, double att_c3 = 0.0,
             std::string n = "Spot Light")
      : light(i, n), position(pos), direction(unit_vector(dir)),
        inner_angle(inner_ang), outer_angle(outer_ang), c1(att_c1), c2(att_c2),
        c3(att_c3) {}

  vec3 get_direction(const point3 &point) const override {
    return unit_vector(position - point);
  }

  double get_distance(const point3 &point) const override {
    return (position - point).length();
  }

  color get_intensity(const point3 &point) const override {
    if (!enabled)
      return color(0, 0, 0);
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
  void set_position(const point3 &p) override { position = p; }
  void set_direction(const vec3 &d) override { direction = unit_vector(d); }
};

class directional_light : public light {
public:
  vec3 direction;

  directional_light(std::string n = "Directional Light")
      : light(n), direction(0, -1, 0) {}

  directional_light(const vec3 &dir, const color &i,
                    std::string n = "Directional Light")
      : light(i, n), direction(unit_vector(dir)) {}

  vec3 get_direction(const point3 &point) const override { return -direction; }

  double get_distance(const point3 &point) const override { return 1e30; }

  color get_intensity(const point3 &point) const override {
    if (!enabled)
      return color(0, 0, 0);
    return intensity;
  }

  point3 get_position() const override {
    return point3(0, 0, 0) - direction * 1e10;
  }
  void set_direction(const vec3 &d) override { direction = unit_vector(d); }
};

#endif
