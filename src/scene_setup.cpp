#include "../include/scene_setup.h"
#include "../include/cenario/hittable_list.h"
#include "../include/cenario/light.h"
#include "../include/globals.h"
#include "../include/malha/mesh.h"
#include "../include/material/material.h"
#include "../include/object/cone.h"
#include "../include/object/cylinder.h"
#include "../include/object/plane.h"
#include "../include/object/sphere.h"
#include "../include/textures/utils.h"
#include "../include/transform/transform.h"
#include "../include/vectors/mat4.h"
#include <cmath>
#include <iostream>

using namespace std;

void update_object_transform(const string &name) {
  if (object_states.find(name) == object_states.end() ||
      object_transforms.find(name) == object_transforms.end()) {
    return;
  }

  TransformState &state = object_states[name];
  auto trans_ptr = object_transforms[name];

  mat4 Rx = mat4::rotate_x(degrees_to_radians(state.rotation.x()));
  mat4 Ry = mat4::rotate_y(degrees_to_radians(state.rotation.y()));
  mat4 Rz = mat4::rotate_z(degrees_to_radians(state.rotation.z()));
  mat4 R = Rz * Ry * Rx;

  mat4 T = mat4::translate(state.translation.x(), state.translation.y(),
                           state.translation.z());
  mat4 S = mat4::scale(state.scale.x(), state.scale.y(), state.scale.z());

  trans_ptr->forward = T * R * S;

  mat4 Rinv = mat4::rotate_x_inverse(degrees_to_radians(state.rotation.x())) *
              mat4::rotate_y_inverse(degrees_to_radians(state.rotation.y())) *
              mat4::rotate_z_inverse(degrees_to_radians(state.rotation.z()));

  mat4 Tinv = mat4::translate_inverse(
      state.translation.x(), state.translation.y(), state.translation.z());
  mat4 Sinv =
      mat4::scale_inverse(state.scale.x(), state.scale.y(), state.scale.z());

  trans_ptr->inverse = Sinv * Rinv * Tinv;

  trans_ptr->normal_mat = trans_ptr->inverse.transpose();

  need_redraw = true;
}

void update_sword_light() {

  if (sword_light_ptr) {
    auto it = find(lights.begin(), lights.end(), sword_light_ptr);
    if (it != lights.end()) {
      lights.erase(it);
    }
    sword_light_ptr = nullptr;
  }

  if (!blade_shine_enabled)
    return;

  vec3 t_vec(900, 195, 900);
  vec3 r_vec(0, 0, 0);
  vec3 s_vec(1, 1, 1);

  if (object_states.find("Espada Completa") != object_states.end()) {
    TransformState &state = object_states["Espada Completa"];
    t_vec = state.translation;
    r_vec = state.rotation;
    s_vec = state.scale;
  }

  mat4 Rx = mat4::rotate_x(degrees_to_radians(r_vec.x()));
  mat4 Ry = mat4::rotate_y(degrees_to_radians(r_vec.y()));
  mat4 Rz = mat4::rotate_z(degrees_to_radians(r_vec.z()));
  mat4 R = Rz * Ry * Rx;

  mat4 T = mat4::translate(t_vec.x(), t_vec.y(), t_vec.z());
  mat4 S = mat4::scale(s_vec.x(), s_vec.y(), s_vec.z());

  mat4 M = T * R * S;

  vec4 local_pos(0, -50, 15, 1.0);

  vec4 world_pos = M * local_pos;

  sword_light_ptr =
      make_shared<point_light>(point3(world_pos[0], world_pos[1], world_pos[2]),
                               color(0.5, 0.7, 1.0), 2.5, 0.001, 0.0001);

  lights.push_back(sword_light_ptr);
}

shared_ptr<class transform> register_transformable(
    shared_ptr<hittable> obj, const string &name, const vec3 &position,
    const vec3 &rotation = vec3(0, 0, 0), const vec3 &scale = vec3(1, 1, 1)) {

  TransformState state;
  state.translation = position;
  state.rotation = rotation;
  state.scale = scale;

  mat4 Rx = mat4::rotate_x(degrees_to_radians(rotation.x()));
  mat4 Ry = mat4::rotate_y(degrees_to_radians(rotation.y()));
  mat4 Rz = mat4::rotate_z(degrees_to_radians(rotation.z()));
  mat4 R = Rz * Ry * Rx;
  mat4 T = mat4::translate(position.x(), position.y(), position.z());
  mat4 S = mat4::scale(scale.x(), scale.y(), scale.z());

  mat4 Rinv = mat4::rotate_x_inverse(degrees_to_radians(rotation.x())) *
              mat4::rotate_y_inverse(degrees_to_radians(rotation.y())) *
              mat4::rotate_z_inverse(degrees_to_radians(rotation.z()));
  mat4 Tinv = mat4::translate_inverse(position.x(), position.y(), position.z());
  mat4 Sinv = mat4::scale_inverse(scale.x(), scale.y(), scale.z());

  auto trans = make_shared<class transform>(obj, T * R * S, Sinv * Rinv * Tinv);
  trans->name = name;

  world.add(trans);
  object_states[name] = state;
  object_transforms[name] = trans;

  return trans;
}

void toggle_blade_shine(bool increase) {
  if (increase && !blade_shine_enabled) {

    blade_shine_enabled = true;
    if (mat_metal_ptr) {

      mat_metal_ptr->emission = color(0.15, 0.25, 0.55);
      mat_metal_ptr->ks = color(0.5, 0.65, 1.0);
      mat_metal_ptr->shininess = 96.0;
      mat_metal_ptr->ka = color(0.12, 0.15, 0.28);
    }
    cout << "[Brilho] Lamina: ENERGIA HEROICA ATIVADA\n";
  } else if (!increase && blade_shine_enabled) {

    blade_shine_enabled = false;
    if (mat_metal_ptr) {
      mat_metal_ptr->emission = color(0, 0, 0);
      mat_metal_ptr->ks = color(0.1, 0.1, 0.1);
      mat_metal_ptr->shininess = 8.0;
      mat_metal_ptr->ka = color(0.1, 0.1, 0.12);
    }
    cout << "[Brilho] Lamina: DESLIGADO\n";
  }

  update_sword_light();
  need_redraw = true;
}

void setup_lighting() {
  lights.clear();
  const double CX = 900.0;
  const double CZ = 900.0;
  const double WX = 710.0;
  const double WZ = 1090.0;

  vec4 torch_base_pos = vec4(CX - 100, 0, CZ - 80, 1.0);
  double POLE_HEIGHT = 120.0;
  vec4 pole_top_vec4 = torch_base_pos + vec4(0, POLE_HEIGHT, 0, 0);
  point3 torch_light_pos = (pole_top_vec4 + vec4(0, 25, 0, 0)).to_point3();

  if (is_night_mode) {

    ambient.intensity = color(0.03, 0.035, 0.06);

    lights.push_back(make_shared<directional_light>(vec3(0, -1, 0),
                                                    color(0.85, 0.80, 0.70)));

    auto spot_dir = unit_vector(vec3(0.05, -1, 0));
    lights.push_back(make_shared<spot_light>(
        point3(CX, 400, CZ), spot_dir, color(0.3, 0.25, 0.4),
        degrees_to_radians(20), degrees_to_radians(50), 1.0, 0.000008,
        0.0000005));

    lights.push_back(make_shared<point_light>(
        torch_light_pos, color(1.0, 0.6, 0.2), 2.5, 0.001, 0.00004));

    lights.push_back(make_shared<point_light>(
        point3(WX, 50, WZ), color(0.4, 0.7, 1.0), 0.6, 0.003, 0.00005));

    lights.push_back(make_shared<point_light>(point3(CX - 80, 100, CZ + 80),
                                              color(0.8, 0.5, 0.2), 0.5, 0.001,
                                              0.0001));

    lights.push_back(make_shared<point_light>(point3(CX + 80, 120, CZ - 80),
                                              color(0.2, 0.4, 0.8), 0.4, 0.001,
                                              0.0001));

    sky_color_top = color(0.4, 0.6, 0.9);
    sky_color_bottom = color(0.7, 0.8, 0.95);

  } else {

    ambient.intensity = color(0.3, 0.3, 0.3);

    vec3 sun_dir = unit_vector(vec3(1.0, -1.0, -0.5));
    lights.push_back(
        make_shared<directional_light>(sun_dir, color(1.0, 0.95, 0.9) * 0.8));

    lights.push_back(make_shared<point_light>(
        torch_light_pos, color(1.0, 0.6, 0.2), 0.8, 0.001, 0.00004));

    sky_color_top = color(0.15, 0.2, 0.4);
    sky_color_bottom = color(0.5, 0.4, 0.6);
  }
}

void toggle_day_night(bool set_to_night) {
  is_night_mode = set_to_night;
  setup_lighting();
  cout << "[Ambiente] Modo alterado para: " << (is_night_mode ? "DIA" : "NOITE")
       << "\n";
  need_redraw = true;
}

void setup_camera() {
  double window_size = 200.0;

  switch (current_projection) {
  case 0:
    cam.setup(cam_eye, cam_at, vec3(0, 1, 0), 100.0, -window_size, window_size,
              -window_size, window_size, ProjectionType::PERSPECTIVE);
    break;

  case 1:
    cam.setup(cam_eye, cam_at, vec3(0, 1, 0), 100.0, -window_size, window_size,
              -window_size, window_size, ProjectionType::ORTHOGRAPHIC);
    break;

  case 2:
    cam.setup(cam_eye, cam_at, vec3(0, 1, 0), 100.0, -window_size, window_size,
              -window_size, window_size, ProjectionType::OBLIQUE);
    cam.oblique_angle = 0.5;
    cam.oblique_strength = 0.35;
    break;
  }

  cout << "Camera: Eye(" << cam_eye.x() << ", " << cam_eye.y() << ", "
       << cam_eye.z() << ")\n";
}

void create_scene() {
  world.clear();
  object_states.clear();
  object_transforms.clear();

  setup_lighting();

  auto mat_metal = materials::sword_metal();
  mat_metal_ptr = mat_metal;

  if (!blade_shine_enabled) {
    mat_metal->ks = color(0.1, 0.1, 0.1);
    mat_metal->shininess = 8.0;
  }

  auto mat_stone = materials::stone();
  auto mat_leather = materials::leather();
  auto mat_ruby = materials::ruby_gem();
  auto mat_gold = materials::gold();
  auto mat_wood = materials::wood();
  auto mat_moss = materials::moss();
  auto mat_dark = materials::dark_stone();
  auto mat_water = materials::water();
  auto mat_cap = materials::mushroom_cap();
  auto mat_stem = materials::mushroom_stem();
  auto mat_wall_stone = materials::wall_stone();
  auto mat_leaves = materials::leaves();
  auto mat_lake_rock = materials::lake_rock();

  const double CX = 900.0;
  const double CZ = 900.0;

  world.add(make_shared<plane>(point3(0, 0, 0), vec3(0, 1, 0), mat_moss,
                               "Chao Musgo"));

  auto stream_lake_obj = make_shared<cylinder>(
      point3(0, 0, 0), vec3(0, 1, 0), 180, 2, mat_water, "Stream Lake");
  register_transformable(stream_lake_obj, "Stream Lake", vec3(CX, 1.0, CZ));

  auto lake_rocks_group = make_shared<hittable_list>();
  double current_ang = 0;
  while (current_ang < 2 * pi) {
    double r = random_double(175, 185);
    double x = r * cos(current_ang);
    double z = r * sin(current_ang);

    double sz = random_double(12, 22);
    lake_rocks_group->add(make_shared<sphere>(point3(x, -1.0, z), sz,
                                              mat_lake_rock, "Lake Rock"));

    double ang_step = (sz * 0.7) / r;
    current_ang += ang_step;
  }
  register_transformable(lake_rocks_group, "Lake Rocks", vec3(CX, 1.0, CZ));

  double WX = 710.0;
  double WZ = 1090.0;

  auto wf_sheet = make_shared<box_mesh>(
      point3(-100, 0, -8), point3(100, 500, 8), mat_water, "Waterfall Sheet");
  register_transformable(wf_sheet, "Waterfall Sheet", vec3(WX, 20, WZ),
                         vec3(0, -45, 0));

  auto wf_pool = make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 120, 5,
                                       mat_water, "Waterfall Pool");
  register_transformable(wf_pool, "Waterfall Pool", vec3(WX, 2, WZ));

  vec3 river_dir = unit_vector(point3(WX, 0, WZ) - point3(CX, 0, CZ));
  for (int k = 0; k < 5; k++) {
    double t = k / 4.0;
    double start_x = CX + 130 * river_dir.x();
    double start_z = CZ + 130 * river_dir.z();
    point3 p1(start_x, 2, start_z);
    point3 p2(WX, 2, WZ);
    point3 pos = (1.0 - t) * p1 + t * p2;
    world.add(make_shared<cylinder>(pos, vec3(0, 1, 0), 40, 4, mat_water,
                                    "River Trail"));
  }

  for (int i = 0; i < 30; i++) {
    world.add(make_shared<sphere>(
        point3(WX + random_double(-50, 50), 10, WZ + random_double(-50, 50)),
        random_double(8, 20), mat_water, "Splash"));
  }

  for (int i = 0; i < 65; i++) {
    double angle = -pi / 2 + (pi * i) / 65.0 * 2.6;
    if (angle > pi * 1.3 || angle < -pi * 0.3)
      continue;

    double r = random_double(500, 750);
    double x = CX + r * cos(angle);
    double z = CZ + r * sin(angle);

    shared_ptr<material> mat_wall;
    double rnd = random_double(0, 1);
    if (rnd > 0.4)
      mat_wall = mat_moss;
    else if (rnd > 0.2)
      mat_wall = mat_dark;
    else
      mat_wall = mat_wall_stone;

    double h = random_double(300, 750);
    double size = random_double(150, 300);
    point3 rock_pos(x, h * 0.5, z);

    double dist_to_wf = vec3(x - WX, 0, z - WZ).length();
    if (dist_to_wf < 500.0)
      mat_wall = mat_wall_stone;

    world.add(make_shared<sphere>(rock_pos, size, mat_wall, "Cliff Rock"));

    if (random_double(0, 1) > 0.4) {
      world.add(make_shared<sphere>(point3(x, h * 0.9, z), size * 0.5, mat_moss,
                                    "Cliff Vegetation"));
    }
  }

  for (int i = 0; i < 50; i++) {
    double angle = -pi / 2 + (pi * i) / 50.0 * 2.6;
    if (angle > pi * 1.3 || angle < -pi * 0.3)
      continue;

    double r = random_double(500, 750);
    double x = CX + r * cos(angle);
    double z = CZ + r * sin(angle);

    shared_ptr<material> mat_wall;
    double rnd = random_double(0, 1);
    if (rnd > 0.4)
      mat_wall = mat_moss;
    else if (rnd > 0.2)
      mat_wall = mat_dark;
    else
      mat_wall = mat_wall_stone;

    double h = random_double(700, 1100);
    double size = random_double(180, 350);
    point3 rock_pos(x, h * 0.5, z);

    double dist_to_wf = vec3(x - WX, 0, z - WZ).length();
    if (dist_to_wf < 550.0)
      mat_wall = mat_wall_stone;

    world.add(
        make_shared<sphere>(rock_pos, size, mat_wall, "Upper Cliff Rock"));

    if (random_double(0, 1) > 0.6) {
      world.add(make_shared<sphere>(point3(x, h * 0.9, z), size * 0.5, mat_moss,
                                    "Upper Vegetation"));
    }
  }

  int mushroom_count = 0;
  for (int i = 0; i < 80; i++) {
    double r = random_double(200, 500);
    double angle = random_double(0, 2 * pi);
    double x = CX + r * cos(angle);
    double z = CZ + r * sin(angle);

    if (vec3(x - WX, 0, z - WZ).length() < 100)
      continue;

    double type = random_double(0, 1);
    if (type < 0.3) {

      auto mushroom_parts = make_shared<hittable_list>();
      double stem_height = random_double(5, 15);
      mushroom_parts->add(make_shared<cylinder>(
          point3(0, 0, 0), vec3(0, 1, 0), random_double(1, 3), stem_height,
          mat_stem, "Mushroom Stem"));
      mushroom_parts->add(make_shared<sphere>(point3(0, stem_height, 0),
                                              random_double(4, 8), mat_cap,
                                              "Mushroom Cap"));
      string mush_name = "Mushroom " + to_string(++mushroom_count);
      register_transformable(mushroom_parts, mush_name, vec3(x, 0, z));
    } else if (type < 0.7) {
      world.add(make_shared<sphere>(point3(x, 0, z), random_double(10, 25),
                                    mat_moss, "Bush"));
    } else {
      world.add(make_shared<cylinder>(
          point3(x, 0, z),
          vec3(random_double(-0.2, 0.2), 1, random_double(-0.2, 0.2)),
          random_double(2, 5), random_double(10, 20), mat_moss, "Grass Tufo"));
    }
  }

  int colony_count = 0;
  for (int i = 0; i < 15; i++) {
    double r_colony = random_double(200, 480);
    double ang_colony = random_double(0, 2 * pi);
    double colony_x = CX + r_colony * cos(ang_colony);
    double colony_z = CZ + r_colony * sin(ang_colony);

    if (vec3(colony_x - WX, 0, colony_z - WZ).length() < 80)
      continue;

    auto colony_parts = make_shared<hittable_list>();
    int num_mush = (int)random_double(3, 8);
    for (int k = 0; k < num_mush; k++) {
      double mr = random_double(5, 15);
      double mang = random_double(0, 2 * pi);
      double mx = mr * cos(mang);
      double mz = mr * sin(mang);
      double stem_h = random_double(3, 8);

      colony_parts->add(make_shared<cylinder>(point3(mx, 0, mz), vec3(0, 1, 0),
                                              random_double(0.5, 1.5), stem_h,
                                              mat_stem, "Colony Stem"));
      colony_parts->add(make_shared<sphere>(
          point3(mx, stem_h, mz), random_double(2, 5), mat_cap, "Colony Cap"));
    }
    string colony_name = "Colony " + to_string(++colony_count);
    register_transformable(colony_parts, colony_name,
                           vec3(colony_x, 0, colony_z));
  }

  point3 tree_positions[] = {point3(CX - 280, 0, CZ - 120),
                             point3(CX + 300, 0, CZ + 80)};
  string tree_names[] = {"Arvore 1", "Arvore 2"};

  for (int tree_idx = 0; tree_idx < 2; tree_idx++) {
    point3 base_pos = tree_positions[tree_idx];
    auto tree_parts = make_shared<hittable_list>();

    tree_parts->add(make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 30,
                                          400, mat_wood, "Tree Trunk"));

    for (int k = 0; k < 8; k++) {
      double h = random_double(150, 350);
      double ang = random_double(0, 2 * pi);
      vec3 branch_dir = vec3(cos(ang), 0.5, sin(ang));
      point3 branch_start = point3(0, h, 0);

      tree_parts->add(make_shared<cylinder>(branch_start, branch_dir, 8, 100,
                                            mat_wood, "Tree Branch"));
      point3 leaf_center = branch_start + branch_dir * 100.0;
      tree_parts->add(make_shared<sphere>(leaf_center, random_double(40, 70),
                                          mat_leaves, "Tree Leaves"));
    }

    register_transformable(tree_parts, tree_names[tree_idx],
                           vec3(base_pos.x(), base_pos.y(), base_pos.z()));
  }

  const double MOUNTAIN_HEIGHT = 45.0;

  auto rock_parts = make_shared<hittable_list>();

  auto mountain_core =
      make_shared<sphere>(point3(0, 0, 0), 60, mat_stone, "Nucleo Montanha");
  mat4 core_S = mat4::scale(2.2, 0.7, 2.2);
  mat4 core_Sinv = mat4::scale_inverse(2.2, 0.7, 2.2);
  mat4 core_T = mat4::translate(0, 30 - MOUNTAIN_HEIGHT, 0);
  mat4 core_Tinv = mat4::translate_inverse(0, 30 - MOUNTAIN_HEIGHT, 0);
  rock_parts->add(make_shared<class transform>(mountain_core, core_T * core_S,
                                               core_Sinv * core_Tinv));

  auto stone_base_mesh = make_shared<box_mesh>(
      point3(-50, 0, -40), point3(50, 60, 40), mat_stone, "Pedra Base");
  mat4 pedra_shear = mat4::shear(0.08, 0, 0, 0, 0.05, 0);
  mat4 pedra_shear_inv = mat4::shear_inverse(0.08, 0, 0, 0, 0.05, 0);
  rock_parts->add(make_shared<class transform>(stone_base_mesh, pedra_shear,
                                               pedra_shear_inv));

  auto stone_top = make_shared<box_mesh>(
      point3(-35, 60, -25), point3(35, 85, 25), mat_stone, "Pedra Topo");
  rock_parts->add(stone_top);

  register_transformable(rock_parts, "Rocha Principal",
                         vec3(CX, MOUNTAIN_HEIGHT, CZ));

  auto sword_parts = make_shared<hittable_list>();
  const double BLADE_LENGTH = 130.0;

  auto blade_mesh_obj =
      make_shared<blade_mesh>(point3(0, 0, 0), point3(0, BLADE_LENGTH, 0), 10,
                              3, mat_metal, "Lamina", 0.40);

  mat4 bl_R = mat4::rotate_y(degrees_to_radians(90)) *
              mat4::rotate_z(degrees_to_radians(180));
  mat4 bl_Rinv = mat4::rotate_z_inverse(degrees_to_radians(180)) *
                 mat4::rotate_y_inverse(degrees_to_radians(90));
  sword_parts->add(make_shared<class transform>(blade_mesh_obj, bl_R, bl_Rinv));

  auto guard_main = make_shared<cylinder>(point3(0, 0, 0), vec3(1, 0, 0), 3.5,
                                          55, mat_gold, "Guarda Principal");
  sword_parts->add(translate_object(guard_main, -27.5, 0, 0));

  sword_parts->add(
      make_shared<sphere>(point3(-30, 0, 0), 5, mat_gold, "Guarda Esq"));
  sword_parts->add(
      make_shared<sphere>(point3(30, 0, 0), 5, mat_gold, "Guarda Dir"));

  auto guard_center = make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 6,
                                            8, mat_gold, "Guarda Centro");
  sword_parts->add(translate_object(guard_center, 0, -4, 0));

  auto handle = make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 3, 25,
                                      mat_leather, "Cabo");
  sword_parts->add(translate_object(handle, 0, 4, 0));

  auto pomo_sphere =
      make_shared<sphere>(point3(0, 0, 0), 4.5, mat_ruby, "Pomo");
  auto pomo_transform = translate_object(pomo_sphere, 0, 31, 0);
  sword_parts->add(pomo_transform);

  auto tip_cone = cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 2.5, 8,
                                  mat_gold, "Ponta");
  sword_parts->add(translate_object(make_shared<cone>(tip_cone), 0, 35, 0));

  auto mat_sapphire = make_shared<material>(color(0.1, 0.2, 0.8), 0.2, 0.95,
                                            256.0, "Sapphire Gem");
  auto quaternion_sapphire = make_shared<sphere>(
      point3(0, 0, 0), 6, mat_sapphire, "Quaternion Sapphire Gem");
  auto rotated_sapphire = rotate_axis_object(quaternion_sapphire, vec3(1, 1, 1),
                                             degrees_to_radians(30));
  auto sapphire_transform = translate_object(rotated_sapphire, -30, 0, 0);
  sword_parts->add(sapphire_transform);

  auto mat_emerald = make_shared<material>(color(0.1, 0.7, 0.2), 0.2, 0.9,
                                           200.0, "Emerald Gem");
  auto quaternion_emerald = make_shared<sphere>(point3(0, 0, 0), 6, mat_emerald,
                                                "Quaternion Emerald");
  auto rotated_emerald = rotate_axis_object(quaternion_emerald, vec3(0, 1, 1),
                                            degrees_to_radians(60));
  auto emerald_transform = translate_object(rotated_emerald, 30, 0, 0);
  sword_parts->add(emerald_transform);

  const double GUARD_Y = 195.0;
  string sword_name = "Espada Completa";
  TransformState sword_state;
  sword_state.translation = vec3(CX, GUARD_Y, CZ);
  sword_state.rotation = vec3(0, 0, 0);
  sword_state.scale = vec3(1, 1, 1);

  mat4 sword_T =
      mat4::translate(sword_state.translation.x(), sword_state.translation.y(),
                      sword_state.translation.z());
  mat4 sword_Tinv = mat4::translate_inverse(sword_state.translation.x(),
                                            sword_state.translation.y(),
                                            sword_state.translation.z());

  auto sword_transform =
      make_shared<class transform>(sword_parts, sword_T, sword_Tinv);
  sword_transform->name = sword_name;

  world.add(sword_transform);
  object_states[sword_name] = sword_state;
  object_transforms[sword_name] = sword_transform;

  // Registrar os quaternions como transformáveis individuais
  // Eles fazem parte da espada mas podem ser transformados separadamente
  if (auto trans = dynamic_pointer_cast<class transform>(sapphire_transform)) {
    TransformState sapphire_state;
    sapphire_state.translation = vec3(-30, 0, 0);
    object_states["Sapphire Gem"] = sapphire_state;
    object_transforms["Sapphire Gem"] = trans;
  }

  if (auto trans = dynamic_pointer_cast<class transform>(emerald_transform)) {
    TransformState emerald_state;
    emerald_state.translation = vec3(30, 0, 0);
    object_states["Emerald Gem"] = emerald_state;
    object_transforms["Emerald Gem"] = trans;
  }

  if (auto trans = dynamic_pointer_cast<class transform>(pomo_transform)) {
    TransformState ruby_state;
    ruby_state.translation = vec3(0, 31, 0);
    object_states["Ruby Gem"] = ruby_state;
    object_transforms["Ruby Gem"] = trans;
  }

  auto mat_ancient_stone = make_shared<material>(color(0.35, 0.32, 0.28), 0.2,
                                                 0.08, 4.0, "Ancient Stone");

  auto pillar1_parts = make_shared<hittable_list>();
  auto pillar1_cyl =
      make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 15, 120,
                            mat_ancient_stone, "Ruined Pillar 1");
  mat4 p1_shear = mat4::shear(0.35, 0, 0, 0, 0, 0);
  mat4 p1_shear_inv = mat4::shear_inverse(0.35, 0, 0, 0, 0, 0);
  pillar1_parts->add(
      make_shared<class transform>(pillar1_cyl, p1_shear, p1_shear_inv));

  auto cap1 = make_shared<sphere>(point3(0, 0, 0), 20, mat_ancient_stone,
                                  "Pillar 1 Capital");
  mat4 cap1_S = mat4::scale(1.3, 0.4, 1.3);
  mat4 cap1_Sinv = mat4::scale_inverse(1.3, 0.4, 1.3);
  mat4 cap1_T = mat4::translate(42, 120, 0);
  mat4 cap1_Tinv = mat4::translate_inverse(42, 120, 0);
  pillar1_parts->add(make_shared<class transform>(cap1, cap1_T * cap1_S,
                                                  cap1_Sinv * cap1_Tinv));

  string pillar1_name = "Pilar Ruina 1";
  TransformState pillar1_state;
  pillar1_state.translation = vec3(CX - 220, 0, CZ + 150);
  pillar1_state.rotation = vec3(0, 0, 0);
  pillar1_state.scale = vec3(1, 1, 1);
  mat4 pillar1_T = mat4::translate(pillar1_state.translation.x(),
                                   pillar1_state.translation.y(),
                                   pillar1_state.translation.z());
  mat4 pillar1_Tinv = mat4::translate_inverse(pillar1_state.translation.x(),
                                              pillar1_state.translation.y(),
                                              pillar1_state.translation.z());
  auto pillar1_transform =
      make_shared<class transform>(pillar1_parts, pillar1_T, pillar1_Tinv);
  pillar1_transform->name = pillar1_name;
  world.add(pillar1_transform);
  object_states[pillar1_name] = pillar1_state;
  object_transforms[pillar1_name] = pillar1_transform;

  auto pillar2_parts = make_shared<hittable_list>();
  auto pillar2_cyl =
      make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 15, 130,
                            mat_ancient_stone, "Ruined Pillar 2");
  mat4 p2_shear = mat4::shear(-0.30, 0.10, 0, 0, 0, 0);
  mat4 p2_shear_inv = mat4::shear_inverse(-0.30, 0.10, 0, 0, 0, 0);
  pillar2_parts->add(
      make_shared<class transform>(pillar2_cyl, p2_shear, p2_shear_inv));

  auto cap2 = make_shared<sphere>(point3(0, 0, 0), 20, mat_ancient_stone,
                                  "Pillar 2 Capital");
  mat4 cap2_S = mat4::scale(1.3, 0.4, 1.3);
  mat4 cap2_Sinv = mat4::scale_inverse(1.3, 0.4, 1.3);
  mat4 cap2_local_T = mat4::translate(-39, 130, 13);
  mat4 cap2_local_Tinv = mat4::translate_inverse(-39, 130, 13);
  pillar2_parts->add(make_shared<class transform>(cap2, cap2_local_T * cap2_S,
                                                  cap2_Sinv * cap2_local_Tinv));

  string pillar2_name = "Pilar Ruina 2";
  TransformState pillar2_state;
  pillar2_state.translation = vec3(CX + 220, 0, CZ + 150);
  pillar2_state.rotation = vec3(0, 0, 0);
  pillar2_state.scale = vec3(1, 1, 1);
  mat4 pillar2_T = mat4::translate(pillar2_state.translation.x(),
                                   pillar2_state.translation.y(),
                                   pillar2_state.translation.z());
  mat4 pillar2_Tinv = mat4::translate_inverse(pillar2_state.translation.x(),
                                              pillar2_state.translation.y(),
                                              pillar2_state.translation.z());
  auto pillar2_transform =
      make_shared<class transform>(pillar2_parts, pillar2_T, pillar2_Tinv);
  pillar2_transform->name = pillar2_name;
  world.add(pillar2_transform);
  object_states[pillar2_name] = pillar2_state;
  object_transforms[pillar2_name] = pillar2_transform;

  auto pillar3 = make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 12, 80,
                                       mat_ancient_stone, "Ruined Pillar 3");
  mat4 pillar3_shear = mat4::shear(0.25, 0.30, 0, 0, 0, 0);
  mat4 pillar3_shear_inv = mat4::shear_inverse(0.25, 0.30, 0, 0, 0, 0);
  mat4 pillar3_T = mat4::translate(CX - 180, 0, CZ + 280);
  mat4 pillar3_Tinv = mat4::translate_inverse(CX - 180, 0, CZ + 280);
  world.add(make_shared<class transform>(pillar3, pillar3_T * pillar3_shear,
                                         pillar3_shear_inv * pillar3_Tinv));

  auto pillar4 = make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 14, 100,
                                       mat_ancient_stone, "Ruined Pillar 4");
  mat4 pillar4_shear = mat4::shear(-0.45, 0.15, 0, 0, 0, 0);
  mat4 pillar4_shear_inv = mat4::shear_inverse(-0.45, 0.15, 0, 0, 0, 0);
  mat4 pillar4_T = mat4::translate(CX + 200, 0, CZ + 300);
  mat4 pillar4_Tinv = mat4::translate_inverse(CX + 200, 0, CZ + 300);
  world.add(make_shared<class transform>(pillar4, pillar4_T * pillar4_shear,
                                         pillar4_shear_inv * pillar4_Tinv));

  auto mat_torch_flame = make_shared<material>(color(1.0, 0.55, 0.1), 0.95,
                                               0.25, 8.0, "Torch Flame");
  auto mat_torch_core = make_shared<material>(color(1.0, 0.85, 0.2), 0.98, 0.15,
                                              4.0, "Torch Core");
  auto mat_torch_pole = make_shared<material>(color(0.25, 0.15, 0.08), 0.12,
                                              0.04, 4.0, "Torch Pole");

  vec4 torch_base_pos = vec4(CX - 100, 0, CZ - 80, 1.0);
  const double POLE_HEIGHT = 120.0;

  auto torch_parts = make_shared<hittable_list>();
  torch_parts->add(make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 4,
                                         POLE_HEIGHT, mat_torch_pole,
                                         "Torch Pole"));

  auto flame_outer = cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 12, 35,
                                     mat_torch_flame, "Torch Flame Outer");
  torch_parts->add(
      translate_object(make_shared<cone>(flame_outer), 0, POLE_HEIGHT, 0));

  auto flame_inner = cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 6, 25,
                                     mat_torch_core, "Torch Flame Core");
  torch_parts->add(
      translate_object(make_shared<cone>(flame_inner), 0, POLE_HEIGHT + 5, 0));

  string torch_name = "Tocha Medieval";
  TransformState torch_state;
  torch_state.translation =
      vec3(torch_base_pos.x(), torch_base_pos.y(), torch_base_pos.z());
  torch_state.rotation = vec3(0, 0, 0);
  torch_state.scale = vec3(1, 1, 1);
  mat4 torch_T =
      mat4::translate(torch_state.translation.x(), torch_state.translation.y(),
                      torch_state.translation.z());
  mat4 torch_Tinv = mat4::translate_inverse(torch_state.translation.x(),
                                            torch_state.translation.y(),
                                            torch_state.translation.z());
  auto torch_transform =
      make_shared<class transform>(torch_parts, torch_T, torch_Tinv);
  torch_transform->name = torch_name;
  world.add(torch_transform);
  object_states[torch_name] = torch_state;
  object_transforms[torch_name] = torch_transform;

  auto reflected_gem = make_shared<sphere>(point3(CX, GUARD_Y + 31, CZ), 4.5,
                                           mat_ruby, "Pomo Gem Original");
  auto gem_mirrored =
      reflect_object(reflected_gem, point3(CX, 2.0, CZ), vec3(0, 1, 0));
  world.add(gem_mirrored);

  auto reflected_tip = make_shared<cone>(
      cone::from_base(point3(CX, GUARD_Y + 35, CZ), vec3(0, 1, 0), 2.5, 8,
                      mat_gold, "Tip Cone Original"));
  auto tip_mirrored =
      reflect_object(reflected_tip, point3(CX, 2.0, CZ), vec3(0, 1, 0));
  world.add(tip_mirrored);

  auto reflected_guard = make_shared<sphere>(point3(CX + 30, GUARD_Y, CZ), 5,
                                             mat_gold, "Guard Sphere Original");
  auto guard_mirrored =
      reflect_object(reflected_guard, point3(CX, 2.0, CZ), vec3(0, 1, 0));
  world.add(guard_mirrored);

  auto waterfall_splash = make_shared<sphere>(point3(WX - 30, 25, WZ + 20), 12,
                                              mat_water, "Waterfall Splash");
  world.add(waterfall_splash);
  auto splash_mirrored =
      reflect_object(waterfall_splash, point3(WX, 2.0, WZ), vec3(0, 1, 0));
  world.add(splash_mirrored);

  setup_camera();
}
