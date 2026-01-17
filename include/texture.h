#ifndef TEXTURE_H
#define TEXTURE_H

#include "color.h"
#include "vec3.h"
#include <cmath>
#include <memory>

// Interface base para texturas (Requisito 1.3.3)
class texture {
public:
    virtual ~texture() = default;
    virtual color value(double u, double v, const point3& p) const = 0;
};

// Textura de cor sólida
class solid_color : public texture {
public:
    color c;

    solid_color(const color& c) : c(c) {}
    solid_color(double r, double g, double b) : c(r, g, b) {}

    color value(double u, double v, const point3& p) const override {
        return c;
    }
};

// Textura checker (xadrez procedural)
class checker_texture : public texture {
public:
    std::shared_ptr<texture> even;
    std::shared_ptr<texture> odd;
    double scale;

    checker_texture(const color& c1, const color& c2, double scale = 10.0)
        : even(std::make_shared<solid_color>(c1))
        , odd(std::make_shared<solid_color>(c2))
        , scale(scale) {}

    color value(double u, double v, const point3& p) const override {
        double sines = std::sin(scale * p.x()) 
                     * std::sin(scale * p.y()) 
                     * std::sin(scale * p.z());
        if (sines < 0)
            return odd->value(u, v, p);
        else
            return even->value(u, v, p);
    }
};

// Textura de pedra procedural (para a rocha)
class stone_texture : public texture {
public:
    color base_color;
    double noise_scale;

    stone_texture(const color& base = colors::stone_gray, double noise = 0.3)
        : base_color(base), noise_scale(noise) {}

    // Noise simples baseado em posição
    double simple_noise(double x, double y, double z) const {
        // Pseudo-noise baseado em funções trigonométricas
        double n = std::sin(x * 12.9898 + y * 78.233 + z * 45.164) * 43758.5453;
        return n - std::floor(n);
    }

    color value(double u, double v, const point3& p) const override {
        double noise = simple_noise(p.x(), p.y(), p.z());
        double variation = 1.0 - noise_scale + noise * noise_scale * 2;
        return color(
            base_color.r * variation,
            base_color.g * variation,
            base_color.b * variation
        ).clamp();
    }
};

// Textura de metal com variação
class metal_texture : public texture {
public:
    color base_color;

    metal_texture(const color& base = colors::steel) : base_color(base) {}

    color value(double u, double v, const point3& p) const override {
        // Pequena variação baseada na posição
        double variation = 0.95 + 0.1 * std::sin(p.y() * 50.0);
        return color(
            base_color.r * variation,
            base_color.g * variation,
            base_color.b * variation
        ).clamp();
    }
};

#endif
