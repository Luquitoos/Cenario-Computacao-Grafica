#include "../include/globals.h"

using namespace std;

const int IMAGE_WIDTH = 800;
const int IMAGE_HEIGHT = 800;

unsigned char *PixelBuffer = nullptr;

hittable_list world;
camera cam;
ambient_light ambient;
vector<shared_ptr<light>> lights;

int current_projection = 0;
int vanishing_points = 3;
bool need_redraw = true;
string picked_object = "";

bool is_night_mode = false;
color sky_color_top(0.15, 0.2, 0.4);
color sky_color_bottom(0.5, 0.4, 0.6);

bool blade_shine_enabled = false;
shared_ptr<material> mat_metal_ptr = nullptr;
shared_ptr<light> sword_light_ptr = nullptr;

point3 cam_eye(1050, 200, 750);
point3 cam_at(900, 100, 900);
vec3 cam_up(0, 1, 0);
double cam_speed = 30.0;

map<string, TransformState> object_states;
map<string, shared_ptr<class transform>> object_transforms;
string selected_transform_name = "";
