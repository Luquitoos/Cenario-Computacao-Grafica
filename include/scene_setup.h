#ifndef SCENE_SETUP_H
#define SCENE_SETUP_H

#include <string>

void create_scene();
void setup_lighting();
void setup_camera();

void update_object_transform(const std::string &name);
void update_sword_light();
void toggle_blade_shine(bool increase);
void toggle_day_night(bool set_to_night);

#endif
