#ifndef MESH_H
#define MESH_H

#include "../cenario/hittable.h"
#include "../cenario/hittable_list.h"
#include "../vectors/vec3.h"
#include "triangle.h"
#include <memory>
#include <vector>


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
  box_mesh(const point3 &p0, const point3 &p1, std::shared_ptr<material> m,
           const std::string &obj_name = "Box")
      : min_corner(p0), max_corner(p1), mat(m), name(obj_name) {
    build_faces();
  }

  // Construtor com centro, largura, altura, profundidade
  static box_mesh from_center(const point3 &center, double width, double height,
                              double depth, std::shared_ptr<material> m,
                              const std::string &obj_name = "Box") {
    point3 half(width / 2, height / 2, depth / 2);
    return box_mesh(center - half, center + half, m, obj_name);
  }

  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    if (faces.hit(r, t_min, t_max, rec)) {
      rec.object_name = name; // Sobrescrever o nome do triângulo
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
        point3(p0.x(), p0.y(), p0.z()), // 0: front-bottom-left
        point3(p1.x(), p0.y(), p0.z()), // 1: front-bottom-right
        point3(p1.x(), p1.y(), p0.z()), // 2: front-top-right
        point3(p0.x(), p1.y(), p0.z()), // 3: front-top-left
        point3(p0.x(), p0.y(), p1.z()), // 4: back-bottom-left
        point3(p1.x(), p0.y(), p1.z()), // 5: back-bottom-right
        point3(p1.x(), p1.y(), p1.z()), // 6: back-top-right
        point3(p0.x(), p1.y(), p1.z()), // 7: back-top-left
    };

    // 6 faces = 12 triângulos
    // Frente (Z-)
    faces.add(
        std::make_shared<triangle>(v[0], v[1], v[2], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[0], v[2], v[3], mat, name + "_face"));

    // Trás (Z+)
    faces.add(
        std::make_shared<triangle>(v[5], v[4], v[7], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[5], v[7], v[6], mat, name + "_face"));

    // Esquerda (X-)
    faces.add(
        std::make_shared<triangle>(v[4], v[0], v[3], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[4], v[3], v[7], mat, name + "_face"));

    // Direita (X+)
    faces.add(
        std::make_shared<triangle>(v[1], v[5], v[6], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[1], v[6], v[2], mat, name + "_face"));

    // Cima (Y+)
    faces.add(
        std::make_shared<triangle>(v[3], v[2], v[6], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[3], v[6], v[7], mat, name + "_face"));

    // Baixo (Y-)
    faces.add(
        std::make_shared<triangle>(v[4], v[5], v[1], mat, name + "_face"));
    faces.add(
        std::make_shared<triangle>(v[4], v[1], v[0], mat, name + "_face"));
  }
};

// Lâmina de espada melhorada: quase retangular, afina só no final
class blade_mesh : public hittable {
public:
  hittable_list faces;
  std::shared_ptr<material> mat;
  std::string name;

  blade_mesh() {}

  // Cria uma lâmina de espada realista
  // base_center: centro da base (onde conecta na guarda)
  // tip: ponta da lâmina
  // width: largura da lâmina
  // thickness: espessura da lâmina
  // taper_start: porcentagem onde começa a afinar (0.7 = 70% do comprimento)
  blade_mesh(const point3 &base_center, const point3 &tip, double width,
             double thickness, std::shared_ptr<material> m,
             const std::string &obj_name = "Blade", double taper_start = 0.75)
      : mat(m), name(obj_name) {

    vec3 blade_vec = tip - base_center;
    double blade_length = blade_vec.length();
    vec3 dir = unit_vector(blade_vec);

    // Criar vetores perpendiculares
    vec3 up(0, 1, 0);
    if (std::abs(dot(dir, up)) > 0.9) {
      up = vec3(1, 0, 0);
    }
    vec3 right = unit_vector(cross(dir, up));
    vec3 forward = unit_vector(cross(right, dir));

    // Pontos ao longo da lâmina: base, meio (onde começa afinar), ponta
    point3 p_base = base_center;
    point3 p_taper = base_center + dir * (blade_length * taper_start);
    point3 p_tip = tip;

    // Larguras em cada ponto
    double w_base = width;
    double w_taper = width * 0.9; // Quase a mesma largura
    double w_tip = width * 0.05;  // Bem fina na ponta

    double t_base = thickness;
    double t_taper = thickness * 0.85;
    double t_tip = thickness * 0.15;

    // Vértices da base (retângulo)
    point3 b0 = p_base - right * (w_base / 2) - forward * (t_base / 2);
    point3 b1 = p_base + right * (w_base / 2) - forward * (t_base / 2);
    point3 b2 = p_base + right * (w_base / 2) + forward * (t_base / 2);
    point3 b3 = p_base - right * (w_base / 2) + forward * (t_base / 2);

    // Vértices no ponto de transição (ainda largo)
    point3 m0 = p_taper - right * (w_taper / 2) - forward * (t_taper / 2);
    point3 m1 = p_taper + right * (w_taper / 2) - forward * (t_taper / 2);
    point3 m2 = p_taper + right * (w_taper / 2) + forward * (t_taper / 2);
    point3 m3 = p_taper - right * (w_taper / 2) + forward * (t_taper / 2);

    // Vértices da ponta (muito fina)
    point3 t0 = p_tip - right * (w_tip / 2) - forward * (t_tip / 2);
    point3 t1 = p_tip + right * (w_tip / 2) - forward * (t_tip / 2);
    point3 t2 = p_tip + right * (w_tip / 2) + forward * (t_tip / 2);
    point3 t3 = p_tip - right * (w_tip / 2) + forward * (t_tip / 2);

    // === SEÇÃO 1: Base até transição (quase retangular) ===
    // Face frontal
    faces.add(std::make_shared<triangle>(b0, b1, m1, mat, name));
    faces.add(std::make_shared<triangle>(b0, m1, m0, mat, name));
    // Face traseira
    faces.add(std::make_shared<triangle>(b3, m3, m2, mat, name));
    faces.add(std::make_shared<triangle>(b3, m2, b2, mat, name));
    // Face direita
    faces.add(std::make_shared<triangle>(b1, b2, m2, mat, name));
    faces.add(std::make_shared<triangle>(b1, m2, m1, mat, name));
    // Face esquerda
    faces.add(std::make_shared<triangle>(b3, b0, m0, mat, name));
    faces.add(std::make_shared<triangle>(b3, m0, m3, mat, name));

    // === SEÇÃO 2: Transição até ponta (afunilando) ===
    // Face frontal
    faces.add(std::make_shared<triangle>(m0, m1, t1, mat, name));
    faces.add(std::make_shared<triangle>(m0, t1, t0, mat, name));
    // Face traseira
    faces.add(std::make_shared<triangle>(m3, t3, t2, mat, name));
    faces.add(std::make_shared<triangle>(m3, t2, m2, mat, name));
    // Face direita
    faces.add(std::make_shared<triangle>(m1, m2, t2, mat, name));
    faces.add(std::make_shared<triangle>(m1, t2, t1, mat, name));
    // Face esquerda
    faces.add(std::make_shared<triangle>(m3, m0, t0, mat, name));
    faces.add(std::make_shared<triangle>(m3, t0, t3, mat, name));

    // === Tampa da base ===
    faces.add(std::make_shared<triangle>(b0, b3, b2, mat, name));
    faces.add(std::make_shared<triangle>(b0, b2, b1, mat, name));

    // === Tampa da ponta ===
    faces.add(std::make_shared<triangle>(t0, t1, t2, mat, name));
    faces.add(std::make_shared<triangle>(t0, t2, t3, mat, name));
  }

  bool hit(const ray &r, double t_min, double t_max,
           hit_record &rec) const override {
    if (faces.hit(r, t_min, t_max, rec)) {
      rec.object_name = name;
      return true;
    }
    return false;
  }

  std::string get_name() const override { return name; }
};

#endif
