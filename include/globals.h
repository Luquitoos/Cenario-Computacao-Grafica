#ifndef GLOBALS_H
#define GLOBALS_H

#include "../include/camera/camera.h"
#include "../include/cenario/hittable_list.h"
#include "../include/cenario/light.h"
#include "../include/colors/color.h"
#include "../include/material/material.h"
#include "../include/transform/transform.h"
#include "../include/vectors/vec3.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

extern const int IMAGE_WIDTH;
extern const int IMAGE_HEIGHT;
extern unsigned char *PixelBuffer;

extern hittable_list world;
extern camera cam;
extern ambient_light ambient;
extern std::vector<std::shared_ptr<light>> lights;

extern int current_projection;
extern int vanishing_points;
extern int vanishing_points_preset;
extern bool need_redraw;
extern std::string picked_object;

extern bool is_night_mode;
extern color sky_color_top;
extern color sky_color_bottom;

extern bool blade_shine_enabled;
extern std::shared_ptr<material> mat_metal_ptr;
extern std::shared_ptr<light> sword_light_ptr;

extern point3 cam_eye;
extern point3 cam_at;
extern vec3 cam_up;
extern double cam_speed;

struct TransformState {
  vec3 scale;
  vec3 rotation;
  vec3 translation;

  TransformState() : scale(1, 1, 1), rotation(0, 0, 0), translation(0, 0, 0) {}
};

extern std::map<std::string, TransformState> object_states;
extern std::map<std::string, std::shared_ptr<transform>> object_transforms;
extern std::string selected_transform_name;

#endif
