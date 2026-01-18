#ifndef TEXTURE_H
#define TEXTURE_H

#include "color.h"
#include "utils.h" // Para clamp
#include "vec3.h"
#include <cmath>
#include <memory>

// Interface base para texturas (Requisito 1.3.3)
class texture {
public:
  virtual ~texture() = default;
  virtual color value(double u, double v, const point3 &p) const = 0;
};

// Textura de cor sólida
class solid_color : public texture {
public:
  color c;

  solid_color(const color &c) : c(c) {}
  solid_color(double r, double g, double b) : c(r, g, b) {}

  color value(double u, double v, const point3 &p) const override { return c; }
};

// Textura checker (xadrez procedural)
class checker_texture : public texture {
public:
  std::shared_ptr<texture> even;
  std::shared_ptr<texture> odd;
  double scale;

  checker_texture(const color &c1, const color &c2, double scale = 10.0)
      : even(std::make_shared<solid_color>(c1)),
        odd(std::make_shared<solid_color>(c2)), scale(scale) {}

  color value(double u, double v, const point3 &p) const override {
    double sines = std::sin(scale * p.x()) * std::sin(scale * p.y()) *
                   std::sin(scale * p.z());
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

  stone_texture(const color &base = colors::stone_gray, double noise = 0.3)
      : base_color(base), noise_scale(noise) {}

  // Noise simples baseado em posição
  double simple_noise(double x, double y, double z) const {
    // Pseudo-noise baseado em funções trigonométricas
    double n = std::sin(x * 12.9898 + y * 78.233 + z * 45.164) * 43758.5453;
    return n - std::floor(n);
  }

  color value(double u, double v, const point3 &p) const override {
    double noise = simple_noise(p.x(), p.y(), p.z());
    double variation = 1.0 - noise_scale + noise * noise_scale * 2;
    return color(base_color.r * variation, base_color.g * variation,
                 base_color.b * variation)
        .clamp();
  }
};

// Textura de imagem (JPG/PNG) usando stb_image
// Requer stb_image.h no include path (User must provide it)
#include "stb_image.h"
#include <iostream>

class image_texture : public texture {
private:
  unsigned char *data;
  int width, height;
  int bytes_per_scanline;

public:
  const static int bytes_per_pixel = 3;

  image_texture(const char *filename) {
    auto components_per_pixel = bytes_per_pixel;

    data = stbi_load(filename, &width, &height, &components_per_pixel,
                     components_per_pixel);

    if (!data) {
      std::cerr << "ERROR: Could not load texture image file '" << filename
                << "'.\n";
      // Debug suggestion:
      std::cerr << "Ensure stb_image.h is in include/ and file exists relative "
                   "to executable.\n";
      width = height = 0;
    } else {
      std::cout << "Loaded texture: " << filename << " (" << width << "x"
                << height << ")\n";
    }

    bytes_per_scanline = bytes_per_pixel * width;
  }

  ~image_texture() {
    if (data)
      stbi_image_free(data);
  }

  color value(double u, double v, const point3 &p) const override {
    // Se não carregou, retorna roxo debug
    if (data == nullptr)
      return color(1, 0, 1);

    // Clamp u,v to [0,1]
    u = clamp(u, 0.0, 1.0);
    v = 1.0 - clamp(v, 0.0, 1.0); // Flip V

    int i = static_cast<int>(u * width);
    int j = static_cast<int>(v * height);

    if (i >= width)
      i = width - 1;
    if (j >= height)
      j = height - 1;

    const double color_scale = 1.0 / 255.0;
    auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;

    return color(color_scale * pixel[0], color_scale * pixel[1],
                 color_scale * pixel[2]);
  }
};

// Textura de imagem com repetição (tiling) para superfícies grandes
class tiled_image_texture : public texture {
private:
  unsigned char *data;
  int width, height;
  int bytes_per_scanline;
  double scale; // Fator de escala para repetição

public:
  const static int bytes_per_pixel = 3;

  tiled_image_texture(const char *filename, double tile_scale = 50.0)
      : scale(tile_scale) {
    auto components_per_pixel = bytes_per_pixel;

    data = stbi_load(filename, &width, &height, &components_per_pixel,
                     components_per_pixel);

    if (!data) {
      std::cerr << "ERROR: Could not load tiled texture '" << filename
                << "'.\n";
      width = height = 0;
    } else {
      std::cout << "Loaded tiled texture: " << filename << " (" << width << "x"
                << height << ")\n";
    }

    bytes_per_scanline = bytes_per_pixel * width;
  }

  ~tiled_image_texture() {
    if (data)
      stbi_image_free(data);
  }

  color value(double u, double v, const point3 &p) const override {
    if (data == nullptr)
      return color(1, 0, 1);

    // Usa coordenadas de mundo para tile, ignorando UV
    // Escala e repete usando fmod
    double tile_u = std::fmod(std::abs(p.x() / scale), 1.0);
    double tile_v = std::fmod(std::abs(p.z() / scale), 1.0);

    int i = static_cast<int>(tile_u * width);
    int j = static_cast<int>((1.0 - tile_v) * height);

    if (i >= width)
      i = width - 1;
    if (j >= height)
      j = height - 1;
    if (i < 0)
      i = 0;
    if (j < 0)
      j = 0;

    const double color_scale = 1.0 / 255.0;
    auto pixel = data + j * bytes_per_scanline + i * bytes_per_pixel;

    return color(color_scale * pixel[0], color_scale * pixel[1],
                 color_scale * pixel[2]);
  }
};

// Textura de metal com variação
class metal_texture : public texture {
public:
  color base_color;

  metal_texture(const color &base = colors::steel) : base_color(base) {}

  color value(double u, double v, const point3 &p) const override {
    // Pequena variação baseada na posição
    double variation = 0.95 + 0.1 * std::sin(p.y() * 50.0);
    return color(base_color.r * variation, base_color.g * variation,
                 base_color.b * variation)
        .clamp();
  }
};

#endif
