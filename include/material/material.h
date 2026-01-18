#ifndef MATERIAL_H
#define MATERIAL_H

#include "../colors/color.h"
#include "../textures/texture.h"
#include <memory>

// Material com modelo de Phong (Requisito 1.3.2 - pelo menos 4 materiais)
class material {
public:
  std::shared_ptr<texture> kd; // Textura difusa
  color ka;                    // Cor ambiente
  color ks;                    // Cor especular
  double shininess;            // Expoente de brilho
  std::string name;            // Nome do material (para pick)

  material()
      : kd(std::make_shared<solid_color>(colors::gray)), ka(0.1, 0.1, 0.1),
        ks(0.3, 0.3, 0.3), shininess(32.0), name("default") {}

  // Construtor com cor sólida
  material(const color &diffuse, double ambient_factor = 0.1,
           double specular_factor = 0.3, double shine = 32.0,
           const std::string &mat_name = "unnamed")
      : kd(std::make_shared<solid_color>(diffuse)),
        ka(diffuse * ambient_factor),
        ks(specular_factor, specular_factor, specular_factor), shininess(shine),
        name(mat_name) {}

  // Construtor com textura
  material(std::shared_ptr<texture> tex, double ambient_factor = 0.1,
           double specular_factor = 0.3, double shine = 32.0,
           const std::string &mat_name = "textured")
      : kd(tex), ka(ambient_factor, ambient_factor, ambient_factor),
        ks(specular_factor, specular_factor, specular_factor), shininess(shine),
        name(mat_name) {}

  // Construtor completo
  material(std::shared_ptr<texture> tex, const color &ambient,
           const color &specular, double shine,
           const std::string &mat_name = "custom")
      : kd(tex), ka(ambient), ks(specular), shininess(shine), name(mat_name) {}

  color get_diffuse(double u, double v, const point3 &p) const {
    return kd->value(u, v, p);
  }
};

// Materiais pré-definidos para a cena (Requisito: 4 materiais distintos)
namespace materials {
// Material 1: Metal da espada (brilhante)
// Material 1: Metal da espada (brilhante)
inline std::shared_ptr<material> sword_metal() {
  auto tex = std::make_shared<image_texture>("textures/metal_lamina.jpg");
  return std::make_shared<material>(tex, color(0.1, 0.1, 0.12), // ka
                                    color(0.9, 0.9, 0.95), // ks - brilho alto
                                    128.0,                 // shininess alto
                                    "Sword Metal");
}

// Material 2: Pedra (texturizada, fosca)
// Material 2: Pedra (texturizada, fosca)
inline std::shared_ptr<material> stone() {
  auto tex = std::make_shared<image_texture>("textures/rochas.jpg");
  return std::make_shared<material>(tex, color(0.15, 0.14, 0.12), // ka
                                    color(0.1, 0.1, 0.1),         // ks
                                    8.0,                          // shininess
                                    "Stone");
}

// Material 3: Couro do cabo (marrom fosco)
// Material 3: Couro do cabo
inline std::shared_ptr<material> leather() {
  auto tex = std::make_shared<image_texture>("textures/couro_cabo.jpg");
  return std::make_shared<material>(tex,
                                    0.15, // ka
                                    0.05, // ks
                                    4.0,  // shininess
                                    "Leather");
}

// Material 4: Gema (rubi) - muito brilhante
inline std::shared_ptr<material> ruby_gem() {
  return std::make_shared<material>(colors::ruby,
                                    0.2,   // ka
                                    0.95,  // ks - muito brilhante
                                    256.0, // shininess muito alto
                                    "Ruby Gem");
}

// Material extra: Ouro (para decorações)
// Material extra: Ouro (guarda)
inline std::shared_ptr<material> gold() {
  auto tex = std::make_shared<image_texture>("textures/guarda_espada.jpg");
  return std::make_shared<material>(tex, 0.15, 0.8, 200.0, "Gold");
}

// Material extra: Madeira (arvores)
inline std::shared_ptr<material> wood() {
  auto tex = std::make_shared<image_texture>("textures/madeira.jpg");
  return std::make_shared<material>(tex, 0.2, 0.1, 10.0, "Wood");
}

// Material extra: Chão
inline std::shared_ptr<material> floor() {
  auto tex =
      std::make_shared<checker_texture>(color(0.3, 0.25, 0.2), // Marrom escuro
                                        color(0.4, 0.35, 0.3), // Marrom claro
                                        0.5);
  return std::make_shared<material>(tex, 0.2, 0.1, 16.0, "Floor");
}

// Material extra: Parede
inline std::shared_ptr<material> wall() {
  return std::make_shared<material>(color(0.6, 0.55, 0.55), 0.2, 0.05, 4.0,
                                    "Wall");
}

// Material Novo: Musgo (chão grama) - TILED para cobrir todo o chão
inline std::shared_ptr<material> moss() {
  auto tex =
      std::make_shared<tiled_image_texture>("textures/chao_grama.jpg", 80.0);
  return std::make_shared<material>(tex, 0.25, 0.05, 2.0, "Moss");
}

// Material Novo: Parede Caverna Escura
inline std::shared_ptr<material> dark_stone() {
  auto tex = std::make_shared<image_texture>("textures/rochas.jpg");
  // Usamos rochas.jpg mas com ambiente bem escuro
  return std::make_shared<material>(tex, color(0.05, 0.05, 0.05),
                                    color(0.05, 0.05, 0.05), 4.0, "Cave Wall");
}

// Material Novo: Água (Poça cristalina com brilho azulado)
inline std::shared_ptr<material> water() {
  return std::make_shared<material>(color(0.4, 0.6, 0.9), // Cor base azulada
                                    0.2,   // ka (transparência simulada)
                                    0.9,   // ks (muito reflexivo)
                                    150.0, // shininess alto
                                    "Water");
}

// Material Novo: Cogumelo (Chapéu marrom)
inline std::shared_ptr<material> mushroom_cap() {
  return std::make_shared<material>(color(0.5, 0.3, 0.1), 0.7, 0.1, 4.0,
                                    "Mushroom Cap");
}

// Material Novo: Cogumelo (Caule bege)
inline std::shared_ptr<material> mushroom_stem() {
  return std::make_shared<material>(color(0.85, 0.8, 0.7), 0.8, 0.05, 2.0,
                                    "Mushroom Stem");
}

// Material Novo: Rochas das Paredes
inline std::shared_ptr<material> wall_stone() {
  auto tex = std::make_shared<image_texture>("textures/rocha das paredes.jpg");
  return std::make_shared<material>(tex, 0.1, 0.1, 5.0, "Wall Stone");
}

// Material Novo: Folhas
inline std::shared_ptr<material> leaves() {
  auto tex = std::make_shared<image_texture>("textures/Folhas.png");
  return std::make_shared<material>(tex, 0.1, 0.05, 1.0, "Leaves");
}

// Material Novo: Rochas do Lago
inline std::shared_ptr<material> lake_rock() {
  auto tex = std::make_shared<image_texture>("textures/rochas_lago.jpg");
  return std::make_shared<material>(tex, 0.2, 0.3, 10.0, "Lake Rock");
}
} // namespace materials

#endif
