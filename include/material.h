#ifndef MATERIAL_H
#define MATERIAL_H

#include "color.h"
#include "texture.h"
#include <memory>

// Material com modelo de Phong (Requisito 1.3.2 - pelo menos 4 materiais)
class material {
public:
    std::shared_ptr<texture> kd;  // Textura difusa
    color ka;                      // Cor ambiente
    color ks;                      // Cor especular
    double shininess;              // Expoente de brilho
    std::string name;              // Nome do material (para pick)

    material() 
        : kd(std::make_shared<solid_color>(colors::gray))
        , ka(0.1, 0.1, 0.1)
        , ks(0.3, 0.3, 0.3)
        , shininess(32.0)
        , name("default") {}

    // Construtor com cor sólida
    material(const color& diffuse, double ambient_factor = 0.1, 
             double specular_factor = 0.3, double shine = 32.0,
             const std::string& mat_name = "unnamed")
        : kd(std::make_shared<solid_color>(diffuse))
        , ka(diffuse * ambient_factor)
        , ks(specular_factor, specular_factor, specular_factor)
        , shininess(shine)
        , name(mat_name) {}

    // Construtor com textura
    material(std::shared_ptr<texture> tex, double ambient_factor = 0.1,
             double specular_factor = 0.3, double shine = 32.0,
             const std::string& mat_name = "textured")
        : kd(tex)
        , ka(ambient_factor, ambient_factor, ambient_factor)
        , ks(specular_factor, specular_factor, specular_factor)
        , shininess(shine)
        , name(mat_name) {}

    // Construtor completo
    material(std::shared_ptr<texture> tex, const color& ambient,
             const color& specular, double shine,
             const std::string& mat_name = "custom")
        : kd(tex), ka(ambient), ks(specular), shininess(shine), name(mat_name) {}

    color get_diffuse(double u, double v, const point3& p) const {
        return kd->value(u, v, p);
    }
};

// Materiais pré-definidos para a cena (Requisito: 4 materiais distintos)
namespace materials {
    // Material 1: Metal da espada (brilhante)
    inline std::shared_ptr<material> sword_metal() {
        auto tex = std::make_shared<metal_texture>(colors::steel);
        return std::make_shared<material>(
            tex,
            color(0.1, 0.1, 0.12),    // ka - ambiente escuro
            color(0.9, 0.9, 0.95),    // ks - brilho alto
            128.0,                     // shininess alto
            "Sword Metal"
        );
    }

    // Material 2: Pedra (texturizada, fosca)
    inline std::shared_ptr<material> stone() {
        auto tex = std::make_shared<stone_texture>(colors::stone_gray, 0.4);
        return std::make_shared<material>(
            tex,
            color(0.15, 0.14, 0.12),  // ka
            color(0.1, 0.1, 0.1),     // ks - pouco brilho
            8.0,                       // shininess baixo
            "Stone"
        );
    }

    // Material 3: Couro do cabo (marrom fosco)
    inline std::shared_ptr<material> leather() {
        return std::make_shared<material>(
            colors::leather_brown,
            0.15,                      // ka
            0.05,                      // ks - quase sem brilho
            4.0,                       // shininess muito baixo
            "Leather"
        );
    }

    // Material 4: Gema (rubi) - muito brilhante
    inline std::shared_ptr<material> ruby_gem() {
        return std::make_shared<material>(
            colors::ruby,
            0.2,                       // ka
            0.95,                      // ks - muito brilhante
            256.0,                     // shininess muito alto
            "Ruby Gem"
        );
    }

    // Material extra: Ouro (para decorações)
    inline std::shared_ptr<material> gold() {
        return std::make_shared<material>(
            colors::gold,
            0.15,
            0.8,
            200.0,
            "Gold"
        );
    }

    // Material extra: Chão
    inline std::shared_ptr<material> floor() {
        auto tex = std::make_shared<checker_texture>(
            color(0.3, 0.25, 0.2),    // Marrom escuro
            color(0.4, 0.35, 0.3),    // Marrom claro
            0.5
        );
        return std::make_shared<material>(tex, 0.2, 0.1, 16.0, "Floor");
    }

    // Material extra: Parede
    inline std::shared_ptr<material> wall() {
        return std::make_shared<material>(
            color(0.6, 0.55, 0.5),
            0.2,
            0.05,
            4.0,
            "Wall"
        );
    }
}

#endif
