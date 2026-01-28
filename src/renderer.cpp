#include "../include/renderer.h"
#include "../include/globals.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <omp.h>

using namespace std;

color calculate_lighting_bvh(const hit_record &rec, const ray &r) {
  color result(0, 0, 0);

  result = result + rec.mat->emission;

  color diffuse_color = rec.mat->get_diffuse(rec.u, rec.v, rec.p);
  result = result + rec.mat->ka * diffuse_color;

  for (const auto &light_ptr : lights) {
    vec3 L = light_ptr->get_direction(rec.p);
    double light_dist = light_ptr->get_distance(rec.p);

    ray shadow_ray(rec.p + 0.001 * rec.normal, L);
    hit_record shadow_rec;

    if (scene_bvh.hit(shadow_ray, 0.001, light_dist - 0.001, shadow_rec)) {
      continue;
    }

    color light_intensity = light_ptr->get_intensity(rec.p);

    double diff = max(0.0, dot(rec.normal, L));
    result = result + diffuse_color * light_intensity * diff;

    vec3 V = unit_vector(-r.direction());
    vec3 H = unit_vector(L + V);
    double spec = pow(max(0.0, dot(rec.normal, H)), rec.mat->shininess);
    result = result + rec.mat->ks * light_intensity * spec;
  }

  return result.clamp();
}

color ray_color_bvh(const ray &r) {
  hit_record rec;

  if (scene_bvh.hit(r, 0.001, infinity, rec)) {
    return calculate_lighting_bvh(rec, r);
  }

  vec3 unit_direction = unit_vector(r.direction());
  double t = 0.5 * (unit_direction.y() + 1.0);
  return sky_color_bottom * (1.0 - t) + sky_color_top * t;
}

color calculate_lighting(const hit_record &rec, const ray &r,
                         const hittable_list &world) {
  color result(0, 0, 0);

  result = result + rec.mat->emission;

  color diffuse_color = rec.mat->get_diffuse(rec.u, rec.v, rec.p);
  result = result + rec.mat->ka * diffuse_color;

  for (const auto &light_ptr : lights) {
    vec3 L = light_ptr->get_direction(rec.p);
    double light_dist = light_ptr->get_distance(rec.p);

    ray shadow_ray(rec.p + 0.001 * rec.normal, L);
    hit_record shadow_rec;
    if (world.hit(shadow_ray, 0.001, light_dist - 0.001, shadow_rec)) {
      continue;
    }

    color light_intensity = light_ptr->get_intensity(rec.p);

    double diff = max(0.0, dot(rec.normal, L));
    result = result + diffuse_color * light_intensity * diff;

    vec3 V = unit_vector(-r.direction());
    vec3 H = unit_vector(L + V);
    double spec = pow(max(0.0, dot(rec.normal, H)), rec.mat->shininess);
    result = result + rec.mat->ks * light_intensity * spec;
  }

  return result.clamp();
}

color ray_color(const ray &r, const hittable_list &world) {
  hit_record rec;

  if (world.hit(r, 0.001, infinity, rec)) {
    return calculate_lighting(rec, r, world);
  }

  vec3 unit_direction = unit_vector(r.direction());
  double t = 0.5 * (unit_direction.y() + 1.0);
  return sky_color_bottom * (1.0 - t) + sky_color_top * t;
}

void render() {
  cout << "Renderizando " << IMAGE_WIDTH << "x" << IMAGE_HEIGHT
       << " pixels (OpenMP: " << omp_get_max_threads()
       << " threads, BVH ativado)...\n";

#pragma omp parallel for schedule(dynamic, 8)
  for (int j = 0; j < IMAGE_HEIGHT; j++) {
    for (int i = 0; i < IMAGE_WIDTH; i++) {
      double u = double(i) / (IMAGE_WIDTH - 1);
      double v = double(j) / (IMAGE_HEIGHT - 1);

      ray r = cam.get_ray(u, v);
      color pixel_color = ray_color_bvh(r);

      int idx = (j * IMAGE_WIDTH + i) * 3;

      PixelBuffer[idx] = pixel_color.r_byte();
      PixelBuffer[idx + 1] = pixel_color.g_byte();
      PixelBuffer[idx + 2] = pixel_color.b_byte();
    }
  }

  cout << "Renderizacao concluida!                    \n";
  need_redraw = false;
  frame_cached = true;
}

void render_preview() {
  if (!PreviewBuffer) {
    PreviewBuffer = new unsigned char[PREVIEW_WIDTH * PREVIEW_HEIGHT * 3];
  }

#pragma omp parallel for schedule(dynamic, 4)
  for (int j = 0; j < PREVIEW_HEIGHT; j++) {
    for (int i = 0; i < PREVIEW_WIDTH; i++) {
      double u = double(i) / (PREVIEW_WIDTH - 1);
      double v = double(j) / (PREVIEW_HEIGHT - 1);

      ray r = cam.get_ray(u, v);
      color pixel_color = ray_color_bvh(r);

      int idx = (j * PREVIEW_WIDTH + i) * 3;
      PreviewBuffer[idx] = pixel_color.r_byte();
      PreviewBuffer[idx + 1] = pixel_color.g_byte();
      PreviewBuffer[idx + 2] = pixel_color.b_byte();
    }
  }
}

void upscale_preview() {
  if (!PreviewBuffer)
    return;

  double scale_x = double(PREVIEW_WIDTH) / IMAGE_WIDTH;
  double scale_y = double(PREVIEW_HEIGHT) / IMAGE_HEIGHT;

  for (int j = 0; j < IMAGE_HEIGHT; j++) {
    int src_j = int(j * scale_y);
    if (src_j >= PREVIEW_HEIGHT)
      src_j = PREVIEW_HEIGHT - 1;

    for (int i = 0; i < IMAGE_WIDTH; i++) {
      int src_i = int(i * scale_x);
      if (src_i >= PREVIEW_WIDTH)
        src_i = PREVIEW_WIDTH - 1;

      int src_idx = (src_j * PREVIEW_WIDTH + src_i) * 3;
      int dst_idx = (j * IMAGE_WIDTH + i) * 3;

      PixelBuffer[dst_idx] = PreviewBuffer[src_idx];
      PixelBuffer[dst_idx + 1] = PreviewBuffer[src_idx + 1];
      PixelBuffer[dst_idx + 2] = PreviewBuffer[src_idx + 2];
    }
  }
}
