#ifndef TEXTURE_H
#define TEXTURE_H

#include "../colors/color.h"
#include "../vectors/vec3.h"
#include "stb_image.h"
#include "utils.h"
#include <cmath>
#include <iostream>
#include <memory>


class texture {
public:
  virtual ~texture() = default;
  virtual color value(double u, double v, const point3 &p) const = 0;
};

class solid_color : public texture {
public:
  color c;

  solid_color(const color &c) : c(c) {}
  solid_color(double r, double g, double b) : c(r, g, b) {}

  color value(double u, double v, const point3 &p) const override { return c; }
};

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

class stone_texture : public texture {
public:
  color base_color;
  double noise_scale;

  stone_texture(const color &base = colors::stone_gray, double noise = 0.3)
      : base_color(base), noise_scale(noise) {}

  double simple_noise(double x, double y, double z) const {

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

    if (data == nullptr)
      return color(1, 0, 1);

    u = clamp(u, 0.0, 1.0);
    v = 1.0 - clamp(v, 0.0, 1.0);

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

class tiled_image_texture : public texture {
private:
  unsigned char *data;
  int width, height;
  int bytes_per_scanline;
  double scale;

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

class metal_texture : public texture {
public:
  color base_color;

  metal_texture(const color &base = colors::steel) : base_color(base) {}

  color value(double u, double v, const point3 &p) const override {

    double variation = 0.95 + 0.1 * std::sin(p.y() * 50.0);
    return color(base_color.r * variation, base_color.g * variation,
                 base_color.b * variation)
        .clamp();
  }
};

#endif
