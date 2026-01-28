#ifndef AABB_H
#define AABB_H

#include "../ray/ray.h"
#include "../vectors/vec3.h"
#include <algorithm>
#include <cmath>


class aabb {
public:
  point3 minimum;
  point3 maximum;

  aabb() {}
  aabb(const point3 &a, const point3 &b) : minimum(a), maximum(b) {}

  point3 min() const { return minimum; }
  point3 max() const { return maximum; }

  bool hit(const ray &r, double t_min, double t_max) const {
    for (int a = 0; a < 3; a++) {
      double invD = 1.0 / r.direction()[a];
      double t0 = (minimum[a] - r.origin()[a]) * invD;
      double t1 = (maximum[a] - r.origin()[a]) * invD;

      if (invD < 0.0)
        std::swap(t0, t1);

      t_min = t0 > t_min ? t0 : t_min;
      t_max = t1 < t_max ? t1 : t_max;

      if (t_max <= t_min)
        return false;
    }
    return true;
  }

  static aabb surrounding_box(const aabb &box0, const aabb &box1) {
    point3 small(std::fmin(box0.minimum.x(), box1.minimum.x()),
                 std::fmin(box0.minimum.y(), box1.minimum.y()),
                 std::fmin(box0.minimum.z(), box1.minimum.z()));
    point3 big(std::fmax(box0.maximum.x(), box1.maximum.x()),
               std::fmax(box0.maximum.y(), box1.maximum.y()),
               std::fmax(box0.maximum.z(), box1.maximum.z()));
    return aabb(small, big);
  }
};

#endif
