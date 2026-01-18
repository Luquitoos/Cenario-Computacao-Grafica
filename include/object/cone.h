#ifndef CONE_H
#define CONE_H

#include "../cenario/hittable.h"
#include "../vectors/vec3.h"
#include <algorithm>
#include <cmath>

// CONE (Requisito 1.3.1)
class cone : public hittable {
public:
  point3 apex;   // Ápice (ponta) do cone
  vec3 axis;     // Vetor eixo normalizado (do ápice para a base)
  double angle;  // Ângulo de abertura (em radianos)
  double height; // Altura do cone
  std::shared_ptr<material> mat;
  std::string name;

  cone() {}
  cone(const point3 &ap, const vec3 &ax, double ang, double h,
       std::shared_ptr<material> m, const std::string &obj_name = "Cone")
      : apex(ap), axis(unit_vector(ax)), angle(ang), height(h), mat(m),
        name(obj_name) {}

  // Construtor alternativo: base center e raio
  static cone from_base(const point3 &base_center, const vec3 &ax,
                        double base_radius, double h,
                        std::shared_ptr<material> m,
                        const std::string &obj_name = "Cone") {
    cone c;
    c.axis = unit_vector(ax);
    c.apex = base_center - h * c.axis; // Ápice está acima da base
    c.angle = std::atan(base_radius / h);
    c.height = h;
    c.mat = m;
    c.name = obj_name;
    return c;
  }

  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    double best_t = t_max + 1;
    vec3 best_normal;
    bool found = false;

    double cos_a = std::cos(angle);
    double cos2_a = cos_a * cos_a;

    vec3 D = r.direction();
    vec3 CO = r.origin() - apex;

    double D_dot_A = dot(D, axis);
    double CO_dot_A = dot(CO, axis);

    // Coeficientes da equação quadrática para o cone
    double a = D_dot_A * D_dot_A - cos2_a;
    double b = 2.0 * (D_dot_A * CO_dot_A - dot(D, CO) * cos2_a);
    double c = CO_dot_A * CO_dot_A - dot(CO, CO) * cos2_a;

    double discriminant = b * b - 4 * a * c;

    if (discriminant >= 0) {
      double sqrt_d = std::sqrt(discriminant);

      for (int i = 0; i < 2; i++) {
        double t;
        if (std::abs(a) > 1e-8) {
          t = (i == 0) ? (-b - sqrt_d) / (2.0 * a) : (-b + sqrt_d) / (2.0 * a);
        } else {
          if (i == 0)
            continue;
          t = -c / b;
        }

        if (t >= t_min && t < best_t) {
          point3 p = r.at(t);
          double h_point = dot(p - apex, axis);

          // Verificar se está dentro da altura e na parte correta do cone
          if (h_point >= 0 && h_point <= height) {
            best_t = t;
            // Normal do cone
            vec3 cp = p - apex;
            vec3 proj = h_point * axis;
            vec3 radial = cp - proj;
            double tan_a = std::tan(angle);
            best_normal =
                unit_vector(radial - tan_a * h_point * unit_vector(radial));
            // Correção: normal apontando para fora
            best_normal =
                unit_vector(cp - (1.0 + tan_a * tan_a) * h_point * axis);
            best_normal = unit_vector(unit_vector(radial) - tan_a * axis);
            found = true;
          }
        }
      }
    }

    // Teste com a tampa circular da base
    point3 base_center = apex + height * axis;
    double base_radius = height * std::tan(angle);
    double t_base = hit_base(r, base_center, base_radius, t_min, best_t);
    if (t_base >= t_min && t_base < best_t) {
      best_t = t_base;
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

    // Coordenadas UV
    vec3 cp = rec.p - apex;
    double h_point = dot(cp, axis);
    rec.v = h_point / height;
    vec3 radial = cp - h_point * axis;
    rec.u = std::atan2(radial.z(), radial.x()) / (2.0 * 3.14159265358979) + 0.5;

    return true;
  }

  std::string get_name() const override { return name; }

private:
  double hit_base(const ray &r, const point3 &center, double radius,
                  double t_min, double t_max) const {
    double denom = dot(r.direction(), axis);

    if (std::abs(denom) < 1e-8) {
      return t_max + 1;
    }

    double t = dot(center - r.origin(), axis) / denom;

    if (t < t_min || t > t_max) {
      return t_max + 1;
    }

    point3 p = r.at(t);
    double dist = (p - center).length();

    if (dist > radius) {
      return t_max + 1;
    }

    return t;
  }
};

#endif
