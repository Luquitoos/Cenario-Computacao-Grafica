#ifndef VEC4_H
#define VEC4_H

#include "vec3.h"
#include <cmath>

class vec4 {
public:
  double e[4];

  vec4() : e{0, 0, 0, 0} {}
  vec4(double e0, double e1, double e2, double e3) : e{e0, e1, e2, e3} {}
  vec4(const vec3 &v, double w) : e{v.x(), v.y(), v.z(), w} {}

  double x() const { return e[0]; }
  double y() const { return e[1]; }
  double z() const { return e[2]; }
  double w() const { return e[3]; }

  
  vec3 to_vec3() const { return vec3(e[0], e[1], e[2]); }

  
  vec3 to_point3() const {
    if (std::abs(e[3]) > 1e-10) {
      return vec3(e[0] / e[3], e[1] / e[3], e[2] / e[3]);
    }
    return vec3(e[0], e[1], e[2]);
  }

  vec4 operator-() const { return vec4(-e[0], -e[1], -e[2], -e[3]); }
  double operator[](int i) const { return e[i]; }
  double &operator[](int i) { return e[i]; }

  vec4 &operator+=(const vec4 &v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    e[3] += v.e[3];
    return *this;
  }

  vec4 &operator*=(double t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    e[3] *= t;
    return *this;
  }

  double length() const {
    return std::sqrt(e[0] * e[0] + e[1] * e[1] + e[2] * e[2] + e[3] * e[3]);
  }
};

using point4 = vec4;

inline vec4 operator+(const vec4 &u, const vec4 &v) {
  return vec4(u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2],
              u.e[3] + v.e[3]);
}

inline vec4 operator-(const vec4 &u, const vec4 &v) {
  return vec4(u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2],
              u.e[3] - v.e[3]);
}

inline vec4 operator*(double t, const vec4 &v) {
  return vec4(t * v.e[0], t * v.e[1], t * v.e[2], t * v.e[3]);
}

inline vec4 operator*(const vec4 &v, double t) { return t * v; }

inline double dot(const vec4 &u, const vec4 &v) {
  return u.e[0] * v.e[0] + u.e[1] * v.e[1] + u.e[2] * v.e[2] + u.e[3] * v.e[3];
}

#endif
