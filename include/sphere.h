#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"
#include <cmath>
#include <string>

// ESFERA (Requisito 1.3.1)
class sphere : public hittable {
public:
    point3 center;
    double radius;
    std::shared_ptr<material> mat;
    std::string name;

    sphere() {}
    sphere(const point3& c, double r, std::shared_ptr<material> m, 
           const std::string& obj_name = "Sphere")
        : center(c), radius(r), mat(m), name(obj_name) {}

    bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        // Vetor L = origem do raio - centro da esfera
        vec3 L = r.origin() - center;
        
        // Coeficientes da equação quadrática
        // a = d·d = 1 (raio normalizado)
        double a = dot(r.direction(), r.direction());
        double b = 2.0 * dot(L, r.direction());
        double c = dot(L, L) - radius * radius;
        
        // Discriminante
        double discriminant = b * b - 4 * a * c;
        
        if (discriminant < 0) {
            return false;  // Não há interseção
        }
        
        double sqrt_d = std::sqrt(discriminant);
        
        // Encontrar a raiz mais próxima no intervalo válido
        double t = (-b - sqrt_d) / (2.0 * a);
        if (t < t_min || t > t_max) {
            t = (-b + sqrt_d) / (2.0 * a);
            if (t < t_min || t > t_max) {
                return false;
            }
        }
        
        // Preencher hit_record
        rec.t = t;
        rec.p = r.at(t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;
        rec.object_name = name;
        
        // Coordenadas UV esféricas
        double theta = std::acos(-outward_normal.y());
        double phi = std::atan2(-outward_normal.z(), outward_normal.x()) + 3.14159265358979;
        rec.u = phi / (2.0 * 3.14159265358979);
        rec.v = theta / 3.14159265358979;
        
        return true;
    }

    std::string get_name() const override { return name; }
};

#endif
