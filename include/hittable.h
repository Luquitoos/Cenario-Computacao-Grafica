#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "vec3.h"
#include "material.h"
#include <memory>
#include <string>

// Registro de hit (interseção raio-objeto)
struct hit_record {
    point3 p;                        // Ponto de interseção
    vec3 normal;                     // Normal da superfície
    std::shared_ptr<material> mat;   // Material do objeto
    double t;                        // Parâmetro t do raio
    double u, v;                     // Coordenadas de textura
    bool front_face;                 // Se atingiu face frontal
    std::string object_name;         // Nome do objeto (para pick)

    void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

// Interface base para objetos que podem ser atingidos por raios
class hittable {
public:
    virtual ~hittable() = default;
    
    // Testa interseção do raio com o objeto
    // t_min e t_max definem o intervalo válido
    virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
    
    // Nome do objeto (para função pick)
    virtual std::string get_name() const = 0;
};

#endif
