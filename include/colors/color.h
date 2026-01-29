#ifndef COLOR_H
#define COLOR_H

#include "../vectors/vec3.h"
#include <algorithm>

class color {
public:
  double r, g, b;

  color() : r(0), g(0), b(0) {}
  color(double r, double g, double b) : r(r), g(g), b(b) {}
  color(const vec3 &v) : r(v.x()), g(v.y()), b(v.z()) {}

  color operator+(const color &c) const {
    return color(r + c.r, g + c.g, b + c.b);
  }

  color operator-(const color &c) const {
    return color(r - c.r, g - c.g, b - c.b);
  }

  color operator*(const color &c) const {
    return color(r * c.r, g * c.g, b * c.b);
  }

  color operator*(double t) const { return color(r * t, g * t, b * t); }

  color operator/(double t) const { return color(r / t, g / t, b / t); }

  color &operator+=(const color &c) {
    r += c.r;
    g += c.g;
    b += c.b;
    return *this;
  }

  color &operator*=(double t) {
    r *= t;
    g *= t;
    b *= t;
    return *this;
  }

  color clamp() const {
    return color(std::max(0.0, std::min(1.0, r)),
                 std::max(0.0, std::min(1.0, g)),
                 std::max(0.0, std::min(1.0, b)));
  }

  int r_byte() const {
    return static_cast<int>(255.999 * std::max(0.0, std::min(1.0, r)));
  }
  int g_byte() const {
    return static_cast<int>(255.999 * std::max(0.0, std::min(1.0, g)));
  }
  int b_byte() const {
    return static_cast<int>(255.999 * std::max(0.0, std::min(1.0, b)));
  }
};

inline color operator*(double t, const color &c) { return c * t; }

namespace colors {
const color black(0, 0, 0);
const color white(1, 1, 1);
const color red(1, 0, 0);
const color green(0, 1, 0);
const color blue(0, 0, 1);
const color yellow(1, 1, 0);
const color cyan(0, 1, 1);
const color magenta(1, 0, 1);
const color gray(0.5, 0.5, 0.5);
const color silver(0.75, 0.75, 0.75);
const color gold(0.83, 0.68, 0.21);
const color bronze(0.8, 0.5, 0.2);
const color ruby(0.88, 0.07, 0.37);
const color emerald(0.31, 0.78, 0.47);
const color sapphire(0.06, 0.32, 0.73);
const color stone_gray(0.5, 0.5, 0.45);
const color leather_brown(0.55, 0.35, 0.17);
const color steel(0.67, 0.7, 0.73);
} // namespace colors

#endif
