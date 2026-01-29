#ifndef RENDERER_H
#define RENDERER_H

#include "../include/cenario/hittable_list.h"
#include "../include/colors/color.h"
#include "../include/ray/ray.h"

color calculate_lighting(const hit_record &rec, const ray &r,
                         const hittable_list &world);
color ray_color(const ray &r, const hittable_list &world);

void render();
void render_preview();
void upscale_preview();

#endif
