#ifndef MESH_H
#define MESH_H

#include "hittable.h"
#include "hittable_list.h"
#include "triangle.h"
#include "vec3.h"
#include <vector>
#include <memory>

// MALHA - Paralelepípedo (Box) composto de triângulos (Requisito 1.3.1)
class box_mesh : public hittable {
public:
    point3 min_corner;
    point3 max_corner;
    hittable_list faces;
    std::shared_ptr<material> mat;
    std::string name;

    box_mesh() {}
    
    // Construtor com ponto mínimo e máximo
    box_mesh(const point3& p0, const point3& p1, std::shared_ptr<material> m,
             const std::string& obj_name = "Box")
        : min_corner(p0), max_corner(p1), mat(m), name(obj_name) {
        build_faces();
    }

    // Construtor com centro, largura, altura, profundidade
    static box_mesh from_center(const point3& center, double width, double height, double depth,
                                std::shared_ptr<material> m, const std::string& obj_name = "Box") {
        point3 half(width/2, height/2, depth/2);
        return box_mesh(center - half, center + half, m, obj_name);
    }

    bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        if (faces.hit(r, t_min, t_max, rec)) {
            rec.object_name = name;  // Sobrescrever o nome do triângulo
            return true;
        }
        return false;
    }

    std::string get_name() const override { return name; }

private:
    void build_faces() {
        point3 p0 = min_corner;
        point3 p1 = max_corner;

        // 8 vértices do box
        point3 v[8] = {
            point3(p0.x(), p0.y(), p0.z()),  // 0: front-bottom-left
            point3(p1.x(), p0.y(), p0.z()),  // 1: front-bottom-right
            point3(p1.x(), p1.y(), p0.z()),  // 2: front-top-right
            point3(p0.x(), p1.y(), p0.z()),  // 3: front-top-left
            point3(p0.x(), p0.y(), p1.z()),  // 4: back-bottom-left
            point3(p1.x(), p0.y(), p1.z()),  // 5: back-bottom-right
            point3(p1.x(), p1.y(), p1.z()),  // 6: back-top-right
            point3(p0.x(), p1.y(), p1.z()),  // 7: back-top-left
        };

        // 6 faces = 12 triângulos
        // Frente (Z-)
        faces.add(std::make_shared<triangle>(v[0], v[1], v[2], mat, name + "_face"));
        faces.add(std::make_shared<triangle>(v[0], v[2], v[3], mat, name + "_face"));
        
        // Trás (Z+)
        faces.add(std::make_shared<triangle>(v[5], v[4], v[7], mat, name + "_face"));
        faces.add(std::make_shared<triangle>(v[5], v[7], v[6], mat, name + "_face"));
        
        // Esquerda (X-)
        faces.add(std::make_shared<triangle>(v[4], v[0], v[3], mat, name + "_face"));
        faces.add(std::make_shared<triangle>(v[4], v[3], v[7], mat, name + "_face"));
        
        // Direita (X+)
        faces.add(std::make_shared<triangle>(v[1], v[5], v[6], mat, name + "_face"));
        faces.add(std::make_shared<triangle>(v[1], v[6], v[2], mat, name + "_face"));
        
        // Cima (Y+)
        faces.add(std::make_shared<triangle>(v[3], v[2], v[6], mat, name + "_face"));
        faces.add(std::make_shared<triangle>(v[3], v[6], v[7], mat, name + "_face"));
        
        // Baixo (Y-)
        faces.add(std::make_shared<triangle>(v[4], v[5], v[1], mat, name + "_face"));
        faces.add(std::make_shared<triangle>(v[4], v[1], v[0], mat, name + "_face"));
    }
};

// Prisma triangular (para lâmina da espada)
class blade_mesh : public hittable {
public:
    hittable_list faces;
    std::shared_ptr<material> mat;
    std::string name;

    blade_mesh() {}

    // Cria uma lâmina de espada: base larga que afina até a ponta
    // base_center: centro da base
    // tip: ponta da lâmina
    // width: largura da base
    // thickness: espessura
    blade_mesh(const point3& base_center, const point3& tip, 
               double width, double thickness,
               std::shared_ptr<material> m, const std::string& obj_name = "Blade")
        : mat(m), name(obj_name) {
        
        vec3 dir = unit_vector(tip - base_center);
        double length = (tip - base_center).length();
        
        // Criar vetores perpendiculares para a base
        vec3 up(0, 1, 0);
        if (std::abs(dot(dir, up)) > 0.9) {
            up = vec3(1, 0, 0);
        }
        vec3 right = unit_vector(cross(dir, up));
        vec3 forward = unit_vector(cross(right, dir));

        // Vértices da base (retângulo)
        point3 b0 = base_center - right * (width/2) - forward * (thickness/2);
        point3 b1 = base_center + right * (width/2) - forward * (thickness/2);
        point3 b2 = base_center + right * (width/2) + forward * (thickness/2);
        point3 b3 = base_center - right * (width/2) + forward * (thickness/2);

        // Ponta (um único ponto ou linha fina)
        point3 t0 = tip - forward * (thickness/4);
        point3 t1 = tip + forward * (thickness/4);

        // Faces laterais (da base até a ponta)
        faces.add(std::make_shared<triangle>(b0, b1, t0, mat, name));
        faces.add(std::make_shared<triangle>(b1, t1, t0, mat, name));
        
        faces.add(std::make_shared<triangle>(b2, b3, t1, mat, name));
        faces.add(std::make_shared<triangle>(b3, t0, t1, mat, name));
        
        // Lados
        faces.add(std::make_shared<triangle>(b1, b2, t1, mat, name));
        faces.add(std::make_shared<triangle>(b3, b0, t0, mat, name));

        // Base (fechamento)
        faces.add(std::make_shared<triangle>(b0, b3, b2, mat, name));
        faces.add(std::make_shared<triangle>(b0, b2, b1, mat, name));
    }

    bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override {
        if (faces.hit(r, t_min, t_max, rec)) {
            rec.object_name = name;
            return true;
        }
        return false;
    }

    std::string get_name() const override { return name; }
};

#endif
