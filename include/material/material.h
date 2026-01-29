#ifndef MATERIAL_H
#define MATERIAL_H

#include "../colors/color.h"
#include "../textures/texture.h"
#include <memory>

class material {
public:
  std::shared_ptr<texture> kd;
  color ka;
  color ks;
  double shininess;
  std::string name;
  color emission;

  // [Requisito 1.3.2] Materiais (Obrigatório: pelo menos 4 materiais distintos)
  // define propriedades como difusa (kd), ambiente (ka), especular (ks), brilho
  // etc. Suporta cor sólida ou textura.
  material()
      : kd(std::make_shared<solid_color>(colors::gray)), ka(0.1, 0.1, 0.1),
        ks(0.3, 0.3, 0.3), shininess(32.0), name("default"), emission(0, 0, 0) {
  }

  material(const color &diffuse, double ambient_factor = 0.1,
           double specular_factor = 0.3, double shine = 32.0,
           const std::string &mat_name = "unnamed")
      : kd(std::make_shared<solid_color>(diffuse)),
        ka(diffuse * ambient_factor),
        ks(specular_factor, specular_factor, specular_factor), shininess(shine),
        name(mat_name), emission(0, 0, 0) {}

  material(std::shared_ptr<texture> tex, double ambient_factor = 0.1,
           double specular_factor = 0.3, double shine = 32.0,
           const std::string &mat_name = "textured")
      : kd(tex), ka(ambient_factor, ambient_factor, ambient_factor),
        ks(specular_factor, specular_factor, specular_factor), shininess(shine),
        name(mat_name), emission(0, 0, 0) {}

  material(std::shared_ptr<texture> tex, const color &ambient,
           const color &specular, double shine,
           const std::string &mat_name = "custom")
      : kd(tex), ka(ambient), ks(specular), shininess(shine), name(mat_name),
        emission(0, 0, 0) {}

  color get_diffuse(double u, double v, const point3 &p) const {
    return kd->value(u, v, p);
  }
};

namespace materials {

// [Requisito 1.3.3] Textura (Obrigatório: pelo menos 1 textura aplicada)
// Aplica a textura 'metal_lamina.jpg' ao material 'Sword Metal'.
inline std::shared_ptr<material> sword_metal() {
  auto tex = std::make_shared<image_texture>("textures/metal_lamina.jpg");
  return std::make_shared<material>(
      tex, color(0.1, 0.1, 0.12), color(0.9, 0.9, 0.95), 128.0, "Sword Metal");
}

inline std::shared_ptr<material> stone() {
  auto tex = std::make_shared<image_texture>("textures/rochas.jpg");
  return std::make_shared<material>(tex, color(0.15, 0.14, 0.12),
                                    color(0.1, 0.1, 0.1), 8.0, "Stone");
}

inline std::shared_ptr<material> leather() {
  auto tex = std::make_shared<image_texture>("textures/couro_cabo.jpg");
  return std::make_shared<material>(tex, 0.15, 0.05, 4.0, "Leather");
}

inline std::shared_ptr<material> ruby_gem() {
  return std::make_shared<material>(colors::ruby, 0.2, 0.95, 256.0, "Ruby Gem");
}

inline std::shared_ptr<material> gold() {
  auto tex = std::make_shared<image_texture>("textures/guarda_espada.jpg");
  return std::make_shared<material>(tex, 0.15, 0.8, 200.0, "Gold");
}

inline std::shared_ptr<material> wood() {
  auto tex = std::make_shared<image_texture>("textures/madeira.jpg");
  return std::make_shared<material>(tex, 0.2, 0.1, 10.0, "Wood");
}

inline std::shared_ptr<material> floor() {
  auto tex = std::make_shared<checker_texture>(color(0.3, 0.25, 0.2),
                                               color(0.4, 0.35, 0.3), 0.5);
  return std::make_shared<material>(tex, 0.2, 0.1, 16.0, "Floor");
}

inline std::shared_ptr<material> wall() {
  return std::make_shared<material>(color(0.6, 0.55, 0.55), 0.2, 0.05, 4.0,
                                    "Wall");
}

inline std::shared_ptr<material> moss() {
  auto tex =
      std::make_shared<tiled_image_texture>("textures/chao_grama.jpg", 80.0);
  return std::make_shared<material>(tex, 0.25, 0.05, 2.0, "Moss");
}

inline std::shared_ptr<material> dark_stone() {
  auto tex = std::make_shared<image_texture>("textures/rochas.jpg");

  return std::make_shared<material>(tex, color(0.05, 0.05, 0.05),
                                    color(0.05, 0.05, 0.05), 4.0, "Cave Wall");
}

inline std::shared_ptr<material> water() {
  return std::make_shared<material>(color(0.4, 0.6, 0.9), 0.2, 0.9, 150.0,
                                    "Water");
}

inline std::shared_ptr<material> mushroom_cap() {
  return std::make_shared<material>(color(0.5, 0.3, 0.1), 0.7, 0.1, 4.0,
                                    "Mushroom Cap");
}

inline std::shared_ptr<material> mushroom_stem() {
  return std::make_shared<material>(color(0.85, 0.8, 0.7), 0.8, 0.05, 2.0,
                                    "Mushroom Stem");
}

inline std::shared_ptr<material> wall_stone() {
  auto tex = std::make_shared<image_texture>("textures/rocha das paredes.jpg");
  return std::make_shared<material>(tex, 0.1, 0.1, 5.0, "Wall Stone");
}

inline std::shared_ptr<material> leaves() {
  auto tex = std::make_shared<image_texture>("textures/Folhas.png");
  return std::make_shared<material>(tex, 0.1, 0.05, 1.0, "Leaves");
}

inline std::shared_ptr<material> lake_rock() {
  auto tex = std::make_shared<image_texture>("textures/rochas_lago.jpg");
  return std::make_shared<material>(tex, 0.2, 0.3, 10.0, "Lake Rock");
}
} // namespace materials

#endif
