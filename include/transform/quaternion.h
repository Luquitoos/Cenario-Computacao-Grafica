#ifndef QUATERNION_H
#define QUATERNION_H

#include "../vectors/mat4.h"
#include "../vectors/vec3.h"
#include <cmath>

class quaternion {
public:
  double w, x, y, z;

  quaternion() : w(1), x(0), y(0), z(0) {}
  quaternion(double w, double x, double y, double z) : w(w), x(x), y(y), z(z) {}

  static quaternion from_axis_angle(const vec3 &axis, double angle_rad) {
    vec3 n = unit_vector(axis);
    double half_angle = angle_rad / 2.0;
    double s = std::sin(half_angle);
    return quaternion(std::cos(half_angle), n.x() * s, n.y() * s, n.z() * s);
  }

  quaternion operator*(const quaternion &q) const {
    return quaternion(w * q.w - x * q.x - y * q.y - z * q.z,
                      w * q.x + x * q.w + y * q.z - z * q.y,
                      w * q.y - x * q.z + y * q.w + z * q.x,
                      w * q.z + x * q.y - y * q.x + z * q.w);
  }

  quaternion conjugate() const { return quaternion(w, -x, -y, -z); }

  double norm() const { return std::sqrt(w * w + x * x + y * y + z * z); }

  quaternion normalize() const {
    double n = norm();
    return quaternion(w / n, x / n, y / n, z / n);
  }

  vec3 rotate(const vec3 &v) const {
    quaternion p(0, v.x(), v.y(), v.z());
    quaternion result = (*this) * p * conjugate();
    return vec3(result.x, result.y, result.z);
  }

  mat4 to_matrix() const {
    double xx = x * x, yy = y * y, zz = z * z;
    double xy = x * y, xz = x * z, yz = y * z;
    double wx = w * x, wy = w * y, wz = w * z;

    return mat4(1 - 2 * (yy + zz), 2 * (xy - wz), 2 * (xz + wy), 0,
                2 * (xy + wz), 1 - 2 * (xx + zz), 2 * (yz - wx), 0,
                2 * (xz - wy), 2 * (yz + wx), 1 - 2 * (xx + yy), 0, 0, 0, 0, 1);
  }

  mat4 to_matrix_inverse() const { return conjugate().to_matrix(); }
};

inline mat4 rotate_axis(const vec3 &axis, double angle_rad) {
  quaternion q = quaternion::from_axis_angle(axis, angle_rad);
  return q.to_matrix();
}

inline mat4 rotate_axis_inverse(const vec3 &axis, double angle_rad) {
  quaternion q = quaternion::from_axis_angle(axis, angle_rad);
  return q.to_matrix_inverse();
}

#endif
