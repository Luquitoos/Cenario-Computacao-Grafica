#ifndef PLANE_H
#define PLANE_H

#include "hittable.h"
#include "vec3.h"
#include <cmath>

// PLANO (para chão e paredes)
class plane : public hittable {
public:
    point3 point;      // Um ponto no plano
    vec3 normal;       // Normal do plano
    std::shared_ptr<material> mat;
    std::string name;

    plane() {}
    plane(const point3& p, const vec3& n, std::shared_ptr<material> m,
          const std::string& obj_name = "Plane")
        : point(p), normal(unit_vector(n)), mat(m), name(obj_name) {}

    bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        // t = (C - P0) · N / (D · N)
        double denom = dot(r.direction(), normal);
        
        // Se o raio é paralelo ao plano
        if (std::abs(denom) < 1e-8) {
            return false;
        }
        
        double t = dot(point - r.origin(), normal) / denom;
        
        if (t < t_min || t > t_max) {
            return false;
        }
        
        rec.t = t;
        rec.p = r.at(t);
        rec.set_face_normal(r, normal);
        rec.mat = mat;
        rec.object_name = name;
        
        // Coordenadas UV baseadas na posição XZ (para chão)
        rec.u = rec.p.x() * 0.1;
        rec.v = rec.p.z() * 0.1;
        
        return true;
    }

    std::string get_name() const override { return name; }
};

#endif
