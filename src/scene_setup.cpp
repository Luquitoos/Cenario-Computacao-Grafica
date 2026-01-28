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

  auto t_object =
      make_shared<class transform>(obj, T * R * S, Sinv * Rinv * Tinv);
  t_object->name = name;

  world.add(t_object);

  object_transforms[name] = t_object;
  object_states[name] = state;

  if (initial_object_states.find(name) == initial_object_states.end()) {
    initial_object_states[name] = state;
  }

  return t_object;
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

    lights.push_back(make_shared<directional_light>(
        vec3(0, -1, 0), color(0.85, 0.80, 0.70), "Directional Light - Moon"));

    auto spot_dir = unit_vector(vec3(0.05, -1, 0));
    lights.push_back(make_shared<spot_light>(
        point3(CX, 400, CZ), spot_dir, color(0.3, 0.25, 0.4),
        degrees_to_radians(20), degrees_to_radians(50), 1.0, 0.000008,
        0.0000005, "Spot Light - Center"));

    lights.push_back(make_shared<point_light>(torch_light_pos,
                                              color(1.0, 0.6, 0.2), 2.5, 0.001,
                                              0.00004, "Point Light - Torch"));

    lights.push_back(
        make_shared<point_light>(point3(WX, 50, WZ), color(0.4, 0.7, 1.0), 0.6,
                                 0.003, 0.00005, "Point Light - Waterfall"));

    lights.push_back(make_shared<point_light>(point3(CX - 80, 100, CZ + 80),
                                              color(0.8, 0.5, 0.2), 0.5, 0.001,
                                              0.0001, "Point Light - River 1"));

    lights.push_back(make_shared<point_light>(point3(CX + 80, 120, CZ - 80),
                                              color(0.2, 0.4, 0.8), 0.4, 0.001,
                                              0.0001, "Point Light - River 2"));

    sky_color_top = color(0.4, 0.6, 0.9);
    sky_color_bottom = color(0.7, 0.8, 0.95);

  } else {

    ambient.intensity = color(0.3, 0.3, 0.3);

    vec3 sun_dir = unit_vector(vec3(1.0, -1.0, -0.5));
    lights.push_back(make_shared<directional_light>(
        sun_dir, color(1.0, 0.95, 0.9) * 0.8, "Directional Light - Sun"));

    lights.push_back(
        make_shared<point_light>(torch_light_pos, color(1.0, 0.6, 0.2), 0.8,
                                 0.001, 0.00004, "Point Light - Torch (Day)"));

    sky_color_top = color(0.15, 0.2, 0.4);
    sky_color_bottom = color(0.5, 0.4, 0.6);
  }
}

void remove_animals() {

  world.remove_by_name_prefix("Animal_");

  for (auto &fl : firefly_lights) {
    auto it = std::find(lights.begin(), lights.end(), fl);
    if (it != lights.end()) {
      lights.erase(it);
    }
  }
  firefly_lights.clear();
}

void add_day_animals() {
  const double CX = 900.0;
  const double CZ = 900.0;

  auto mat_butterfly_body = make_shared<material>(color(0.1, 0.1, 0.1), 0.3,
                                                  0.7, 50.0, "Butterfly Body");
  auto mat_butterfly_wing1 = make_shared<material>(
      color(0.8, 0.2, 0.6), 0.2, 0.8, 80.0, "Butterfly Wing Pink");
  auto mat_butterfly_wing2 = make_shared<material>(
      color(0.2, 0.5, 0.9), 0.2, 0.8, 80.0, "Butterfly Wing Blue");
  auto mat_rabbit = make_shared<material>(color(0.85, 0.8, 0.75), 0.4, 0.5,
                                          30.0, "Rabbit Fur");
  auto mat_bird = make_shared<material>(color(0.3, 0.5, 0.8), 0.3, 0.6, 50.0,
                                        "Bird Feathers");
  auto mat_horse_body = make_shared<material>(color(0.4, 0.25, 0.15), 0.4, 0.5,
                                              40.0, "Horse Body");
  auto mat_horse_mane = make_shared<material>(color(0.1, 0.05, 0.02), 0.3, 0.4,
                                              30.0, "Horse Mane");
  double sword_y = 195.0;

  auto mat_wing_red =
      make_shared<material>(color(0.9, 0.1, 0.1), 0.2, 0.8, 80.0, "Wing Red");
  auto mat_wing_pink =
      make_shared<material>(color(1.0, 0.4, 0.7), 0.2, 0.8, 80.0, "Wing Pink");
  auto mat_wing_purple = make_shared<material>(color(0.6, 0.2, 0.9), 0.2, 0.8,
                                               80.0, "Wing Purple");

  shared_ptr<material> wing_mats[] = {mat_wing_red, mat_wing_pink,
                                      mat_wing_purple};

  double butterfly_offsets[][3] = {{30, 40, 20}, {-40, 60, -30}, {20, 50, -50}};

  for (int i = 0; i < 3; i++) {

    double bx = CX + butterfly_offsets[i][0];
    double by = sword_y + butterfly_offsets[i][1];
    double bz = CZ + butterfly_offsets[i][2];

    double rot_angle_y = i * 120.0;

    double bank_z = (i % 2 == 0) ? 25.0 : -25.0;
    double pitch_x = 15.0;

    auto wing_mat = wing_mats[i % 3];

    auto butterfly_parts = make_shared<hittable_list>();

    auto body_mesh =
        make_shared<cylinder>(point3(0, -2, 0), vec3(0, 1, 0), 0.4, 5,
                              mat_butterfly_body, "Butterfly Body");
    mat4 body_base_T = mat4::rotate_x(degrees_to_radians(30));
    mat4 body_base_Tinv = mat4::rotate_x_inverse(degrees_to_radians(30));
    butterfly_parts->add(
        make_shared<class transform>(body_mesh, body_base_T, body_base_Tinv));

    auto ant_mesh =
        make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0.5), 0.1, 1.5,
                              mat_butterfly_body, "Butterfly Antenna");
    butterfly_parts->add(
        translate_object(make_shared<cylinder>(*ant_mesh), -0.3, 2.5, 1));
    butterfly_parts->add(
        translate_object(make_shared<cylinder>(*ant_mesh), 0.3, 2.5, 1));

    auto wing_up_mesh = make_shared<box_mesh>(
        point3(0, 0, -0.1), point3(5, 4, 0.1), wing_mat, "Butterfly Wing Up");
    mat4 shear_U = mat4::shear(0.5, 0, 0, 0, 0, 0);
    mat4 shear_Uinv = mat4::shear_inverse(0.5, 0, 0, 0, 0, 0);

    auto wing_low_mesh = make_shared<box_mesh>(
        point3(0, -3, -0.1), point3(3, 0, 0.1), wing_mat, "Butterfly Wing Low");
    mat4 shear_L = mat4::shear(-0.2, 0, 0, 0, 0, 0);
    mat4 shear_Linv = mat4::shear_inverse(-0.2, 0, 0, 0, 0, 0);

    auto wing_R_Group = make_shared<hittable_list>();
    wing_R_Group->add(
        make_shared<class transform>(wing_up_mesh, shear_U, shear_Uinv));
    wing_R_Group->add(
        make_shared<class transform>(wing_low_mesh, shear_L, shear_Linv));
    mat4 wing_R_Final = mat4::rotate_z(degrees_to_radians(-30));
    mat4 wing_R_FinalInv = mat4::rotate_z_inverse(degrees_to_radians(-30));
    butterfly_parts->add(make_shared<class transform>(
        wing_R_Group, wing_R_Final, wing_R_FinalInv));

    auto wing_up_mesh_L =
        make_shared<box_mesh>(point3(-5, 0, -0.1), point3(0, 4, 0.1), wing_mat,
                              "Butterfly Wing Up L");
    mat4 shear_UL = mat4::shear(-0.5, 0, 0, 0, 0, 0);
    mat4 shear_ULinv = mat4::shear_inverse(-0.5, 0, 0, 0, 0, 0);

    auto wing_low_mesh_L =
        make_shared<box_mesh>(point3(-3, -3, -0.1), point3(0, 0, 0.1), wing_mat,
                              "Butterfly Wing Low L");
    mat4 shear_LL = mat4::shear(0.2, 0, 0, 0, 0, 0);
    mat4 shear_LLinv = mat4::shear_inverse(0.2, 0, 0, 0, 0, 0);

    auto wing_L_Group = make_shared<hittable_list>();
    wing_L_Group->add(
        make_shared<class transform>(wing_up_mesh_L, shear_UL, shear_ULinv));
    wing_L_Group->add(
        make_shared<class transform>(wing_low_mesh_L, shear_LL, shear_LLinv));
    mat4 wing_L_Final = mat4::rotate_z(degrees_to_radians(30));
    mat4 wing_L_FinalInv = mat4::rotate_z_inverse(degrees_to_radians(30));
    butterfly_parts->add(make_shared<class transform>(
        wing_L_Group, wing_L_Final, wing_L_FinalInv));

    double sc = 1.0 + random_double(-0.1, 0.1);
    register_transformable(
        butterfly_parts, "Butterfly_Wings_" + to_string(i + 1),
        vec3(bx, by, bz), vec3(pitch_x, rot_angle_y, bank_z), vec3(sc, sc, sc));
  }

  double rabbit_pos[][2] = {{928.844, 682.594},
                            {CX - 230, CZ},
                            {CX, CZ + 230},
                            {CX + 165, CZ - 165},
                            {834.44, 687.431}};

  for (int i = 0; i < 5; i++) {
    double rx = rabbit_pos[i][0];
    double rz = rabbit_pos[i][1];

    double dx = CX - rx;
    double dz = CZ - rz;

    double angle_rad = atan2(dx, dz);

    auto rabbit_parts = make_shared<hittable_list>();

    auto body_mesh =
        make_shared<sphere>(point3(0, 0, 0), 9, mat_rabbit, "Rabbit Body Main");

    mat4 body_S = mat4::scale(0.8, 1.0, 1.4);
    mat4 body_Sinv = mat4::scale_inverse(0.8, 1.0, 1.4);

    mat4 body_R = mat4::rotate_x(degrees_to_radians(-20));
    mat4 body_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-20));
    mat4 body_T = mat4::translate(0, 10, 0);
    mat4 body_Tinv = mat4::translate_inverse(0, 10, 0);
    rabbit_parts->add(
        make_shared<class transform>(body_mesh, body_T * body_R * body_S,
                                     body_Sinv * body_Rinv * body_Tinv));

    auto thigh_mesh =
        make_shared<sphere>(point3(0, 0, 0), 7, mat_rabbit, "Rabbit Thigh");
    mat4 thigh_S = mat4::scale(0.6, 1.2, 1.2);
    mat4 thigh_Sinv = mat4::scale_inverse(0.6, 1.2, 1.2);

    mat4 thigh_L_T = mat4::translate(-6, 8, -5);
    mat4 thigh_L_Tinv = mat4::translate_inverse(-6, 8, -5);
    rabbit_parts->add(make_shared<class transform>(
        thigh_mesh, thigh_L_T * thigh_S, thigh_Sinv * thigh_L_Tinv));

    mat4 thigh_R_T = mat4::translate(6, 8, -5);
    mat4 thigh_R_Tinv = mat4::translate_inverse(6, 8, -5);
    rabbit_parts->add(make_shared<class transform>(
        thigh_mesh, thigh_R_T * thigh_S, thigh_Sinv * thigh_R_Tinv));

    auto chest =
        make_shared<sphere>(point3(0, 14, 8), 6, mat_rabbit, "Rabbit Chest");
    rabbit_parts->add(chest);

    auto head_mesh =
        make_shared<sphere>(point3(0, 0, 0), 6.5, mat_rabbit, "Rabbit Head");
    mat4 head_T = mat4::translate(0, 24, 12);
    mat4 head_Tinv = mat4::translate_inverse(0, 24, 12);
    rabbit_parts->add(
        make_shared<class transform>(head_mesh, head_T, head_Tinv));

    auto snout = cone::from_base(point3(0, 0, 0), vec3(0, -0.2, 1), 3.5, 7,
                                 mat_rabbit, "Rabbit Snout");
    rabbit_parts->add(translate_object(make_shared<cone>(snout), 0, 23, 16));

    auto ear_mesh_base =
        make_shared<box_mesh>(point3(-1.0, 0, -0.6), point3(1.0, 12, 0.6),
                              mat_rabbit, "Rabbit Ear Mesh");

    mat4 ear_shear = mat4::shear(0.3, 0, 0, 0, 0, 0);
    mat4 ear_shearinv = mat4::shear_inverse(0.3, 0, 0, 0, 0, 0);

    mat4 ear_L_R = mat4::rotate_z(degrees_to_radians(25)) *
                   mat4::rotate_x(degrees_to_radians(-20));
    mat4 ear_L_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-20)) *
                      mat4::rotate_z_inverse(degrees_to_radians(25));

    mat4 ear_L_T = mat4::translate(-2.5, 30, 11);
    mat4 ear_L_Tinv = mat4::translate_inverse(-2.5, 30, 11);

    rabbit_parts->add(make_shared<class transform>(
        ear_mesh_base, ear_L_T * ear_L_R * ear_shear,
        ear_shearinv * ear_L_Rinv * ear_L_Tinv));

    mat4 ear_R_R = mat4::rotate_z(degrees_to_radians(-25)) *
                   mat4::rotate_x(degrees_to_radians(-20));
    mat4 ear_R_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-20)) *
                      mat4::rotate_z_inverse(degrees_to_radians(-25));
    mat4 ear_R_T = mat4::translate(2.5, 30, 11);
    mat4 ear_R_Tinv = mat4::translate_inverse(2.5, 30, 11);

    rabbit_parts->add(make_shared<class transform>(
        ear_mesh_base, ear_R_T * ear_R_R * ear_shear,
        ear_shearinv * ear_R_Rinv * ear_R_Tinv));

    auto leg_front =
        make_shared<cylinder>(point3(0, 0, 0), vec3(0, -1, 0.3), 1.5, 14,
                              mat_rabbit, "Rabbit Front Leg");
    rabbit_parts->add(
        translate_object(make_shared<cylinder>(*leg_front), -3.0, 15, 11));
    rabbit_parts->add(
        translate_object(make_shared<cylinder>(*leg_front), 3.0, 15, 11));

    rabbit_parts->add(
        make_shared<sphere>(point3(0, 8, -9), 3.5, mat_rabbit, "Rabbit Tail"));

    double angle_deg = radians_to_degrees(angle_rad);
    register_transformable(
        rabbit_parts, "Animal_Rabbit_Geometric_" + to_string(i + 1),
        vec3(rx, 0, rz), vec3(0, angle_deg, 0), vec3(1, 1, 1));
  }

  auto mat_bird_brown = make_shared<material>(color(0.5, 0.35, 0.2), 0.3, 0.5,
                                              40.0, "Bird Brown");
  auto mat_bird_white =
      make_shared<material>(color(0.9, 0.9, 0.9), 0.3, 0.5, 40.0, "Bird White");
  auto mat_bird_black =
      make_shared<material>(color(0.1, 0.1, 0.1), 0.1, 0.1, 10.0, "Bird Black");

  double bird_positions[][3] = {{1098.42, 220, 819.919},
                                {734.744, 250, 686.032},
                                {804.178, 200, 845.856},
                                {1120.82, 230, 947.773}};

  for (int i = 0; i < 4; i++) {
    double px = bird_positions[i][0];
    double py = bird_positions[i][1];
    double pz = bird_positions[i][2];
    double rot_y = random_double(0, 360);

    auto bird_parts = make_shared<hittable_list>();

    auto body_mesh = make_shared<box_mesh>(point3(-3, -2, -6), point3(3, 2, 6),
                                           mat_bird_brown, "Bird Body");
    mat4 body_R = mat4::rotate_x(degrees_to_radians(10));
    mat4 body_Rinv = mat4::rotate_x_inverse(degrees_to_radians(10));
    bird_parts->add(make_shared<class transform>(body_mesh, body_R, body_Rinv));

    auto belly_mesh =
        make_shared<box_mesh>(point3(-2.5, -2.2, -5), point3(2.5, -1.8, 5),
                              mat_bird_white, "Bird Belly");
    bird_parts->add(
        make_shared<class transform>(belly_mesh, body_R, body_Rinv));

    auto wing_base_geo = make_shared<box_mesh>(
        point3(0, 0, -3), point3(8, 1, 4), mat_bird_brown, "Wing Base");
    auto wing_tip_geo = make_shared<box_mesh>(
        point3(8, 0, -3), point3(12, 1, 4), mat_bird_white, "Wing Tip");

    mat4 wing_shear = mat4::shear(0, 0, 0.5, 0, 0, 0);
    mat4 wing_shearinv = mat4::shear_inverse(0, 0, 0.5, 0, 0, 0);

    auto wing_comp = make_shared<hittable_list>();
    wing_comp->add(wing_base_geo);
    wing_comp->add(wing_tip_geo);

    mat4 wing_L_T = mat4::translate(-2, 1, 0);
    mat4 wing_L_Tinv = mat4::translate_inverse(-2, 1, 0);
    mat4 wing_L_R = mat4::rotate_z(degrees_to_radians(20)) *
                    mat4::rotate_y(degrees_to_radians(180));
    mat4 wing_L_Rinv = mat4::rotate_y_inverse(degrees_to_radians(180)) *
                       mat4::rotate_z_inverse(degrees_to_radians(20));
    bird_parts->add(make_shared<class transform>(
        wing_comp, wing_L_T * wing_L_R * wing_shear,
        wing_shearinv * wing_L_Rinv * wing_L_Tinv));

    mat4 wing_R_T = mat4::translate(2, 1, 0);
    mat4 wing_R_Tinv = mat4::translate_inverse(2, 1, 0);
    mat4 wing_R_R = mat4::rotate_z(degrees_to_radians(-20));
    mat4 wing_R_Rinv = mat4::rotate_z_inverse(degrees_to_radians(-20));
    bird_parts->add(make_shared<class transform>(
        wing_comp, wing_R_T * wing_R_R * wing_shear,
        wing_shearinv * wing_R_Rinv * wing_R_Tinv));

    auto tail_mesh = make_shared<box_mesh>(
        point3(-2.5, 0, 0), point3(2.5, 1, 6), mat_bird_brown, "Bird Tail");
    mat4 tail_T = mat4::translate(0, 1, -5);
    mat4 tail_Tinv = mat4::translate_inverse(0, 1, -5);
    mat4 tail_R = mat4::rotate_x(degrees_to_radians(15)) *
                  mat4::rotate_y(degrees_to_radians(180));
    mat4 tail_Rinv = mat4::rotate_y_inverse(degrees_to_radians(180)) *
                     mat4::rotate_x_inverse(degrees_to_radians(15));
    bird_parts->add(make_shared<class transform>(tail_mesh, tail_T * tail_R,
                                                 tail_Rinv * tail_Tinv));

    auto head_mesh = make_shared<box_mesh>(
        point3(-2, -2, -2.5), point3(2, 2, 2.5), mat_bird_brown, "Bird Head");
    mat4 head_T = mat4::translate(0, 2, 7);
    mat4 head_Tinv = mat4::translate_inverse(0, 2, 7);
    bird_parts->add(make_shared<class transform>(head_mesh, head_T, head_Tinv));

    auto beak_mesh = make_shared<cone>(point3(0, 0, 0), vec3(0, 0, 1), 1.0, 3.0,
                                       mat_bird_black, "Bird Beak");
    mat4 beak_R = mat4::rotate_x(degrees_to_radians(90));
    mat4 beak_Rinv = mat4::rotate_x_inverse(degrees_to_radians(90));
    mat4 beak_T = mat4::translate(0, 2, 9.5);
    mat4 beak_Tinv = mat4::translate_inverse(0, 2, 9.5);
    bird_parts->add(make_shared<class transform>(beak_mesh, beak_T * beak_R,
                                                 beak_Rinv * beak_Tinv));

    auto eye_mesh =
        make_shared<sphere>(point3(0, 0, 0), 0.8, mat_bird_black, "Bird Eye");
    bird_parts->add(
        translate_object(make_shared<sphere>(*eye_mesh), -2.1, 3, 8));
    bird_parts->add(
        translate_object(make_shared<sphere>(*eye_mesh), 2.1, 3, 8));

    double bank = random_double(-15, 15);
    register_transformable(bird_parts, "Bird_Brown_" + to_string(i + 1),
                           vec3(px, py, pz), vec3(0, rot_y, bank),
                           vec3(1, 1, 1));
  }

  {
    double horse_geo_x = 1034.48;
    double horse_geo_z = 1084.61;
    double horse_geo_y = 12.0;

    double dx = 900.0 - horse_geo_x;
    double dz = 900.0 - horse_geo_z;
    double horse_angle = atan2(dx, dz);

    auto mat_horse_geo_body = make_shared<material>(
        color(0.95, 0.95, 0.95), 0.2, 0.3, 10.0, "Horse Origami Body");
    auto mat_horse_geo_mane = make_shared<material>(
        color(0.3, 0.2, 0.15), 0.2, 0.3, 10.0, "Horse Origami Dark");

    auto horse_parts = make_shared<hittable_list>();

    auto chest_mesh =
        make_shared<box_mesh>(point3(-8, -8, -10), point3(8, 12, 10),
                              mat_horse_geo_body, "Horse Chest");
    mat4 chest_R = mat4::rotate_x(degrees_to_radians(-15));
    mat4 chest_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-15));
    mat4 chest_T = mat4::translate(0, 15, 10);
    mat4 chest_Tinv = mat4::translate_inverse(0, 15, 10);
    horse_parts->add(make_shared<class transform>(chest_mesh, chest_T * chest_R,
                                                  chest_Rinv * chest_Tinv));

    auto flank_mesh =
        make_shared<box_mesh>(point3(-7, -7, -10), point3(7, 10, 10),
                              mat_horse_geo_body, "Horse Flank");
    mat4 flank_T = mat4::translate(0, 14, -8);
    mat4 flank_Tinv = mat4::translate_inverse(0, 14, -8);
    horse_parts->add(
        make_shared<class transform>(flank_mesh, flank_T, flank_Tinv));

    auto neck_mesh = make_shared<box_mesh>(point3(-4, 0, -5), point3(4, 25, 5),
                                           mat_horse_geo_body, "Horse Neck");
    mat4 neck_T = mat4::translate(0, 22, 15);
    mat4 neck_Tinv = mat4::translate_inverse(0, 22, 15);
    mat4 neck_R = mat4::rotate_x(degrees_to_radians(-30));
    mat4 neck_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-30));
    horse_parts->add(make_shared<class transform>(neck_mesh, neck_T * neck_R,
                                                  neck_Rinv * neck_Tinv));

    auto head_mesh =
        make_shared<box_mesh>(point3(-3.5, 0, -6), point3(3.5, 8, 14),
                              mat_horse_geo_body, "Horse Head");

    mat4 head_T = mat4::translate(0, 43, 3);
    mat4 head_Tinv = mat4::translate_inverse(0, 43, 3);

    mat4 head_R = mat4::rotate_x(degrees_to_radians(20));
    mat4 head_Rinv = mat4::rotate_x_inverse(degrees_to_radians(20));
    horse_parts->add(make_shared<class transform>(head_mesh, head_T * head_R,
                                                  head_Rinv * head_Tinv));

    auto ear_mesh = make_shared<box_mesh>(point3(-1, 0, -1), point3(1, 4, 1),
                                          mat_horse_geo_body, "Horse Ear");

    // mat4 ear_offset = mat4::translate(-2.5, 8, -4); // Unused
    // mat4 ear_offset = mat4::translate(-2.5, 8, -4); // Unused
    {
      mat4 ear_L_T = mat4::translate(-2.5, 8, -4);
      mat4 ear_R_T = mat4::translate(2.5, 8, -4);
      mat4 ear_Rot = mat4::rotate_x(degrees_to_radians(-20));

      auto ears_group = make_shared<hittable_list>();
      ears_group->add(make_shared<class transform>(
          ear_mesh, ear_L_T * ear_Rot,
          mat4::rotate_x_inverse(degrees_to_radians(-20)) *
              mat4::translate_inverse(-2.5, 8, -4)));
      ears_group->add(make_shared<class transform>(
          ear_mesh, ear_R_T * ear_Rot,
          mat4::rotate_x_inverse(degrees_to_radians(-20)) *
              mat4::translate_inverse(2.5, 8, -4)));

      horse_parts->add(make_shared<class transform>(ears_group, head_T * head_R,
                                                    head_Rinv * head_Tinv));
    }

    auto thigh_mesh =
        make_shared<box_mesh>(point3(-3, -12, -3), point3(3, 0, 3),
                              mat_horse_geo_body, "Horse Thigh");
    auto shin_mesh =
        make_shared<box_mesh>(point3(-2.5, -12, -2.5), point3(2.5, 0, 2.5),
                              mat_horse_geo_body, "Horse Shin");

    mat4 to_knee = mat4::translate(0, -12, 0);
    mat4 from_knee = mat4::translate_inverse(0, -12, 0);

    {
      mat4 hip_T = mat4::translate(-8, 15, 18);
      mat4 thigh_R = mat4::rotate_x(degrees_to_radians(-60));
      mat4 shin_R = mat4::rotate_x(degrees_to_radians(110));

      auto leg_group = make_shared<hittable_list>();
      leg_group->add(make_shared<class transform>(
          thigh_mesh, thigh_R,
          mat4::rotate_x_inverse(degrees_to_radians(-60))));

      mat4 s_M = thigh_R * to_knee * shin_R;
      mat4 s_Minv = mat4::rotate_x_inverse(degrees_to_radians(110)) *
                    from_knee * mat4::rotate_x_inverse(degrees_to_radians(-60));
      leg_group->add(make_shared<class transform>(shin_mesh, s_M, s_Minv));

      horse_parts->add(make_shared<class transform>(
          leg_group, hip_T, mat4::translate_inverse(-8, 15, 18)));
    }

    {
      mat4 hip_T = mat4::translate(8, 15, 18);
      mat4 thigh_R = mat4::rotate_x(degrees_to_radians(0));
      mat4 shin_R = mat4::rotate_x(degrees_to_radians(0));

      auto leg_group = make_shared<hittable_list>();
      leg_group->add(make_shared<class transform>(
          thigh_mesh, thigh_R, mat4::rotate_x_inverse(degrees_to_radians(0))));

      mat4 s_M = thigh_R * to_knee * shin_R;
      mat4 s_Minv = mat4::rotate_x_inverse(degrees_to_radians(0)) * from_knee *
                    mat4::rotate_x_inverse(degrees_to_radians(0));
      leg_group->add(make_shared<class transform>(shin_mesh, s_M, s_Minv));

      horse_parts->add(make_shared<class transform>(
          leg_group, hip_T, mat4::translate_inverse(8, 15, 18)));
    }

    {
      mat4 hip_T = mat4::translate(-8, 14, -15);
      mat4 thigh_R = mat4::rotate_x(degrees_to_radians(15));
      mat4 shin_R = mat4::rotate_x(degrees_to_radians(-25));

      auto leg_group = make_shared<hittable_list>();
      leg_group->add(make_shared<class transform>(
          thigh_mesh, thigh_R, mat4::rotate_x_inverse(degrees_to_radians(15))));

      mat4 s_M = thigh_R * to_knee * shin_R;
      mat4 s_Minv = mat4::rotate_x_inverse(degrees_to_radians(-25)) *
                    from_knee * mat4::rotate_x_inverse(degrees_to_radians(15));
      leg_group->add(make_shared<class transform>(shin_mesh, s_M, s_Minv));

      horse_parts->add(make_shared<class transform>(
          leg_group, hip_T, mat4::translate_inverse(-8, 14, -15)));
    }

    {
      mat4 hip_T = mat4::translate(8, 14, -15);
      mat4 thigh_R = mat4::rotate_x(degrees_to_radians(5));
      mat4 shin_R = mat4::rotate_x(degrees_to_radians(-15));

      auto leg_group = make_shared<hittable_list>();
      leg_group->add(make_shared<class transform>(
          thigh_mesh, thigh_R, mat4::rotate_x_inverse(degrees_to_radians(5))));

      mat4 s_M = thigh_R * to_knee * shin_R;
      mat4 s_Minv = mat4::rotate_x_inverse(degrees_to_radians(-15)) *
                    from_knee * mat4::rotate_x_inverse(degrees_to_radians(5));
      leg_group->add(make_shared<class transform>(shin_mesh, s_M, s_Minv));

      horse_parts->add(make_shared<class transform>(
          leg_group, hip_T, mat4::translate_inverse(8, 14, -15)));
    }

    auto tail_geo = make_shared<box_mesh>(point3(-2, 0, -2), point3(2, 12, 2),
                                          mat_horse_geo_mane, "Horse Tail");
    mat4 tail_T = mat4::translate(0, 18, -12);
    mat4 tail_R = mat4::rotate_x(degrees_to_radians(-45));
    horse_parts->add(make_shared<class transform>(
        tail_geo, tail_T * tail_R,
        mat4::rotate_x_inverse(degrees_to_radians(-45)) *
            mat4::translate_inverse(0, 18, -12)));

    double horse_angle_deg = radians_to_degrees(horse_angle);

    register_transformable(horse_parts, "Animal_Horse_Geometric",
                           vec3(horse_geo_x, horse_geo_y, horse_geo_z),
                           vec3(0, horse_angle_deg, 0), vec3(1.8, 1.8, 1.8));
  }

  cout << "[Animais] Animais DIURNOS adicionados\n";
}

void add_night_animals() {
  const double CX = 900.0;
  const double CZ = 900.0;

  auto mat_firefly_light = make_shared<material>(color(0.8, 0.9, 0.3), 0.1, 0.9,
                                                 100.0, "Firefly Light");
  mat_firefly_light->emission = color(0.5, 0.6, 0.2);

  auto mat_firefly_body = make_shared<material>(color(0.05, 0.05, 0.05), 0.1,
                                                0.1, 10.0, "Firefly Body");

  auto mat_wolf =
      make_shared<material>(color(0.35, 0.35, 0.4), 0.4, 0.5, 40.0, "Wolf Fur");
  auto mat_bear =
      make_shared<material>(color(0.3, 0.2, 0.15), 0.5, 0.4, 30.0, "Bear Fur");

  for (int i = 0; i < 20; i++) {
    double firefly_x = CX + random_double(-250, 250);
    double firefly_y = random_double(30, 150);
    double firefly_z = CZ + random_double(-250, 250);

    auto firefly_visuals = make_shared<hittable_list>();
    firefly_visuals->add(make_shared<sphere>(
        point3(0, 0, 0), 2, mat_firefly_light, "Animal_Firefly_Light"));
    firefly_visuals->add(make_shared<sphere>(
        point3(1.5, 0.5, 1.5), 1.5, mat_firefly_body, "Animal_Firefly_Body"));

    register_transformable(
        firefly_visuals, "Animal_Firefly_Geometric_" + to_string(i + 1),
        vec3(firefly_x, firefly_y, firefly_z), vec3(0, 0, 0), vec3(1, 1, 1));

    double r = random_double(0.3, 0.5);
    double g = random_double(0.5, 0.7);
    double b = random_double(0.1, 0.2);
    auto firefly_light = make_shared<point_light>(
        point3(firefly_x, firefly_y, firefly_z), color(r, g, b), 0.3, 0.05,
        0.005, "Point Light - Firefly " + to_string(i + 1));
    lights.push_back(firefly_light);
    firefly_lights.push_back(firefly_light);
  }

  double leader_x = 833.7;
  double leader_z = 685.8;

  double wdx = 0.3;
  double wdz = 0.95;

  double wrx = wdz;
  double wrz = -wdx;
  (void)wrx;
  (void)wrz;

  struct WolfOffset {
    double off_x, off_z;
  };
  WolfOffset wolves[] = {{0, 0}, {-25, -20}, {25, -20}};

  for (int i = 0; i < 3; i++) {

    double base_x = leader_x + wolves[i].off_x;
    double base_z = leader_z + wolves[i].off_z;
    double ly = 12.0;

    auto wolf_parts = make_shared<hittable_list>();

    auto chest_mesh = make_shared<box_mesh>(point3(-5, -5, -8), point3(5, 7, 8),
                                            mat_wolf, "Wolf Chest");
    mat4 chest_T = mat4::translate(0, 0, 5);
    mat4 chest_Tinv = mat4::translate_inverse(0, 0, 5);
    wolf_parts->add(
        make_shared<class transform>(chest_mesh, chest_T, chest_Tinv));

    auto flank_mesh = make_shared<box_mesh>(point3(-4, -4, -7), point3(4, 6, 7),
                                            mat_wolf, "Wolf Flank");
    mat4 flank_T = mat4::translate(0, 1, -8);
    mat4 flank_Tinv = mat4::translate_inverse(0, 1, -8);
    wolf_parts->add(
        make_shared<class transform>(flank_mesh, flank_T, flank_Tinv));

    auto neck_mesh = make_shared<box_mesh>(point3(-3, 0, -3), point3(3, 10, 3),
                                           mat_wolf, "Wolf Neck");
    mat4 neck_T = mat4::translate(0, 5, 10);
    mat4 neck_Tinv = mat4::translate_inverse(0, 5, 10);
    mat4 neck_R = mat4::rotate_x(degrees_to_radians(-40));
    mat4 neck_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-40));
    wolf_parts->add(make_shared<class transform>(neck_mesh, neck_T * neck_R,
                                                 neck_Rinv * neck_Tinv));

    auto head_mesh = make_shared<box_mesh>(point3(-4, -3, -4), point3(4, 5, 4),
                                           mat_wolf, "Wolf Head");
    mat4 head_T = mat4::translate(0, 14, 16);
    mat4 head_Tinv = mat4::translate_inverse(0, 14, 16);
    wolf_parts->add(make_shared<class transform>(head_mesh, head_T, head_Tinv));

    auto snout_mesh = make_shared<box_mesh>(
        point3(-2, -1.5, 0), point3(2, 2.5, 6), mat_wolf, "Wolf Snout");
    mat4 snout_T = mat4::translate(0, 14, 19);
    mat4 snout_Tinv = mat4::translate_inverse(0, 14, 19);
    wolf_parts->add(
        make_shared<class transform>(snout_mesh, snout_T, snout_Tinv));

    auto ear_mesh = make_shared<box_mesh>(
        point3(-1.5, 0, -1), point3(1.5, 5, 1), mat_wolf, "Wolf Ear");
    mat4 ear_shear = mat4::shear(0, 0, 0, 0, 0.4, 0);
    mat4 ear_shearinv = mat4::shear_inverse(0, 0, 0, 0, 0.4, 0);

    mat4 ear_L_T = mat4::translate(-2.5, 19, 17);
    mat4 ear_L_Tinv = mat4::translate_inverse(-2.5, 19, 17);
    wolf_parts->add(make_shared<class transform>(ear_mesh, ear_L_T * ear_shear,
                                                 ear_shearinv * ear_L_Tinv));

    mat4 ear_R_T = mat4::translate(2.5, 19, 17);
    mat4 ear_R_Tinv = mat4::translate_inverse(2.5, 19, 17);
    wolf_parts->add(make_shared<class transform>(ear_mesh, ear_R_T * ear_shear,
                                                 ear_shearinv * ear_R_Tinv));

    auto leg_upper = make_shared<box_mesh>(point3(-2, -6, -2), point3(2, 2, 2),
                                           mat_wolf, "Wolf Leg Upper");
    auto leg_lower =
        make_shared<box_mesh>(point3(-1.5, -8, -1.5), point3(1.5, 0, 1.5),
                              mat_wolf, "Wolf Leg Lower");

    mat4 leg_FL_T = mat4::translate(-4, 0, 10);
    mat4 leg_FL_Tinv = mat4::translate_inverse(-4, 0, 10);
    wolf_parts->add(
        make_shared<class transform>(leg_upper, leg_FL_T, leg_FL_Tinv));
    mat4 leg_FL_Low_T = mat4::translate(-4, -6, 10);
    mat4 leg_FL_Low_Tinv = mat4::translate_inverse(-4, -6, 10);
    wolf_parts->add(
        make_shared<class transform>(leg_lower, leg_FL_Low_T, leg_FL_Low_Tinv));

    mat4 leg_FR_T = mat4::translate(4, 0, 10);
    mat4 leg_FR_Tinv = mat4::translate_inverse(4, 0, 10);
    wolf_parts->add(
        make_shared<class transform>(leg_upper, leg_FR_T, leg_FR_Tinv));
    mat4 leg_FR_Low_T = mat4::translate(4, -6, 10);
    mat4 leg_FR_Low_Tinv = mat4::translate_inverse(4, -6, 10);
    wolf_parts->add(
        make_shared<class transform>(leg_lower, leg_FR_Low_T, leg_FR_Low_Tinv));

    mat4 leg_BL_T = mat4::translate(-3.5, 0, -12);
    mat4 leg_BL_Tinv = mat4::translate_inverse(-3.5, 0, -12);
    mat4 leg_BL_R = mat4::rotate_x(degrees_to_radians(15));
    mat4 leg_BL_Rinv = mat4::rotate_x_inverse(degrees_to_radians(15));
    wolf_parts->add(make_shared<class transform>(leg_upper, leg_BL_T * leg_BL_R,
                                                 leg_BL_Rinv * leg_BL_Tinv));

    mat4 leg_BL_Low_T = mat4::translate(-3.5, -5.5, -13);
    mat4 leg_BL_Low_Tinv = mat4::translate_inverse(-3.5, -5.5, -13);
    mat4 leg_BL_Low_R = mat4::rotate_x(degrees_to_radians(-20));
    mat4 leg_BL_Low_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-20));
    wolf_parts->add(
        make_shared<class transform>(leg_lower, leg_BL_Low_T * leg_BL_Low_R,
                                     leg_BL_Low_Rinv * leg_BL_Low_Tinv));

    mat4 leg_BR_T = mat4::translate(3.5, 0, -12);
    mat4 leg_BR_Tinv = mat4::translate_inverse(3.5, 0, -12);
    wolf_parts->add(make_shared<class transform>(leg_upper, leg_BR_T * leg_BL_R,
                                                 leg_BL_Rinv * leg_BR_Tinv));

    mat4 leg_BR_Low_T = mat4::translate(3.5, -5.5, -13);
    mat4 leg_BR_Low_Tinv = mat4::translate_inverse(3.5, -5.5, -13);
    wolf_parts->add(
        make_shared<class transform>(leg_lower, leg_BR_Low_T * leg_BL_Low_R,
                                     leg_BL_Low_Rinv * leg_BR_Low_Tinv));

    auto tail_mesh = make_shared<box_mesh>(
        point3(-1.5, -8, -1.5), point3(1.5, 0, 1.5), mat_wolf, "Wolf Tail");
    mat4 tail_T = mat4::translate(0, 3, -14);
    mat4 tail_Tinv = mat4::translate_inverse(0, 3, -14);
    mat4 tail_R = mat4::rotate_x(degrees_to_radians(30));
    mat4 tail_Rinv = mat4::rotate_x_inverse(degrees_to_radians(30));
    wolf_parts->add(make_shared<class transform>(tail_mesh, tail_T * tail_R,
                                                 tail_Rinv * tail_Tinv));

    double wolf_angle = atan2(wdx, wdz);
    double wolf_angle_deg = radians_to_degrees(wolf_angle);

    register_transformable(wolf_parts,
                           "Animal_Wolf_Geometric_" + to_string(i + 1),
                           vec3(base_x, ly, base_z), vec3(0, wolf_angle_deg, 0),
                           vec3(1.2, 1.2, 1.2));
  }

  double ux = 973.894;
  double uy = 8.09958 + 2;
  double uz = 1106.82;

  double udx = -0.33;
  double udz = -0.94;

  double urx = udz;
  double urz = -udx;

  double bear_angle = atan2(udx, udz);

  auto bear_parts = make_shared<hittable_list>();

  auto body_mesh = make_shared<box_mesh>(
      point3(-25, 0, -20), point3(25, 60, 20), mat_bear, "Bear Body Main");

  mat4 body_R = mat4::rotate_x(degrees_to_radians(10));
  mat4 body_Rinv = mat4::rotate_x_inverse(degrees_to_radians(10));
  bear_parts->add(make_shared<class transform>(body_mesh, body_R, body_Rinv));

  auto hump_mesh = make_shared<box_mesh>(
      point3(-18, 50, -15), point3(18, 70, 15), mat_bear, "Bear Hump");

  mat4 hump_T = mat4::translate(0, 0, -5);
  mat4 hump_Tinv = mat4::translate_inverse(0, 0, -5);
  bear_parts->add(make_shared<class transform>(hump_mesh, hump_T, hump_Tinv));

  auto thigh_mesh = make_shared<box_mesh>(
      point3(-10, 0, -20), point3(10, 45, 10), mat_bear, "Bear Thigh");

  mat4 thigh_L_T = mat4::translate(-30, 0, -5);
  mat4 thigh_L_Tinv = mat4::translate_inverse(-30, 0, -5);
  mat4 thigh_L_R = mat4::rotate_z(degrees_to_radians(15)) *
                   mat4::rotate_x(degrees_to_radians(-30));
  mat4 thigh_L_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-30)) *
                      mat4::rotate_z_inverse(degrees_to_radians(15));
  bear_parts->add(make_shared<class transform>(
      thigh_mesh, thigh_L_T * thigh_L_R, thigh_L_Rinv * thigh_L_Tinv));

  mat4 thigh_R_T = mat4::translate(30, 0, -5);
  mat4 thigh_R_Tinv = mat4::translate_inverse(30, 0, -5);
  mat4 thigh_R_R = mat4::rotate_z(degrees_to_radians(-15)) *
                   mat4::rotate_x(degrees_to_radians(-30));
  mat4 thigh_R_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-30)) *
                      mat4::rotate_z_inverse(degrees_to_radians(-15));
  bear_parts->add(make_shared<class transform>(
      thigh_mesh, thigh_R_T * thigh_R_R, thigh_R_Rinv * thigh_R_Tinv));

  auto foot_mesh = make_shared<box_mesh>(point3(-8, 0, -12), point3(8, 6, 12),
                                         mat_bear, "Bear Foot");

  mat4 foot_L_T = mat4::translate(-35, 0, 20);
  mat4 foot_L_Tinv = mat4::translate_inverse(-35, 0, 20);
  mat4 foot_L_R = mat4::rotate_y(degrees_to_radians(-20));
  mat4 foot_L_Rinv = mat4::rotate_y_inverse(degrees_to_radians(-20));
  bear_parts->add(make_shared<class transform>(foot_mesh, foot_L_T * foot_L_R,
                                               foot_L_Rinv * foot_L_Tinv));

  mat4 foot_R_T = mat4::translate(35, 0, 20);
  mat4 foot_R_Tinv = mat4::translate_inverse(35, 0, 20);
  mat4 foot_R_R = mat4::rotate_y(degrees_to_radians(20));
  mat4 foot_R_Rinv = mat4::rotate_y_inverse(degrees_to_radians(20));
  bear_parts->add(make_shared<class transform>(foot_mesh, foot_R_T * foot_R_R,
                                               foot_R_Rinv * foot_R_Tinv));

  auto arm_mesh = make_shared<box_mesh>(point3(-7, 0, -7), point3(7, 45, 7),
                                        mat_bear, "Bear Arm");

  mat4 arm_L_T = mat4::translate(-18, 0, 25);
  mat4 arm_L_Tinv = mat4::translate_inverse(-18, 0, 25);
  bear_parts->add(make_shared<class transform>(arm_mesh, arm_L_T, arm_L_Tinv));

  mat4 arm_R_T = mat4::translate(18, 0, 25);
  mat4 arm_R_Tinv = mat4::translate_inverse(18, 0, 25);
  bear_parts->add(make_shared<class transform>(arm_mesh, arm_R_T, arm_R_Tinv));

  auto head_base = make_shared<box_mesh>(
      point3(-14, -12, -14), point3(14, 12, 14), mat_bear, "Bear Head Base");
  mat4 head_T = mat4::translate(0, 65, 25);
  mat4 head_Tinv = mat4::translate_inverse(0, 65, 25);
  bear_parts->add(make_shared<class transform>(head_base, head_T, head_Tinv));

  auto snout_box = make_shared<box_mesh>(point3(-8, -6, 0), point3(8, 6, 12),
                                         mat_bear, "Bear Snout");
  mat4 snout_T = mat4::translate(0, 60, 39);
  mat4 snout_Tinv = mat4::translate_inverse(0, 60, 39);
  bear_parts->add(make_shared<class transform>(snout_box, snout_T, snout_Tinv));

  auto bear_ear = make_shared<box_mesh>(point3(-4, 0, -2), point3(4, 6, 2),
                                        mat_bear, "Bear Ear");

  mat4 bear_ear_L_T = mat4::translate(-12, 77, 20);
  mat4 bear_ear_L_Tinv = mat4::translate_inverse(-12, 77, 20);
  bear_parts->add(
      make_shared<class transform>(bear_ear, bear_ear_L_T, bear_ear_L_Tinv));

  mat4 bear_ear_R_T = mat4::translate(12, 77, 20);
  mat4 bear_ear_R_Tinv = mat4::translate_inverse(12, 77, 20);
  bear_parts->add(
      make_shared<class transform>(bear_ear, bear_ear_R_T, bear_ear_R_Tinv));

  double bear_angle_deg = radians_to_degrees(bear_angle);

  register_transformable(bear_parts, "Animal_Bear_Geometric", vec3(ux, uy, uz),
                         vec3(0, bear_angle_deg, 0), vec3(1.5, 1.5, 1.5));

  auto mat_torch_flame = make_shared<material>(color(1.0, 0.55, 0.1), 0.95,
                                               0.25, 8.0, "Torch Flame");
  auto mat_torch_core = make_shared<material>(color(1.0, 0.85, 0.2), 0.98, 0.15,
                                              4.0, "Torch Core");
  auto mat_torch_pole = make_shared<material>(color(0.25, 0.15, 0.08), 0.12,
                                              0.04, 4.0, "Torch Pole");

  auto mat_iron = make_shared<material>(color(0.15, 0.15, 0.18), 0.2, 0.3, 20.0,
                                        "Lantern Iron");

  double torch_dist = 90.0;

  double t_pos[][2] = {{ux + urx * torch_dist, uz + urz * torch_dist},
                       {ux - urx * torch_dist, uz - urz * torch_dist}};
  double POLE_HEIGHT = 50.0;

  for (int i = 0; i < 2; i++) {
    double tx = t_pos[i][0];
    double tz = t_pos[i][1];

    auto torch_parts = make_shared<hittable_list>();

    torch_parts->add(make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 4,
                                           POLE_HEIGHT, mat_torch_pole,
                                           "Animal_Lantern_Pole"));

    torch_parts->add(make_shared<cylinder>(point3(0, POLE_HEIGHT, 0),
                                           vec3(0, 1, 0), 12, 4, mat_iron,
                                           "Animal_Lantern_Base"));

    double lantern_base_h = 4.0;
    double cage_start_y = POLE_HEIGHT + lantern_base_h;
    double cage_h = 25.0;
    double cage_r = 10.0;

    for (int k = 0; k < 4; k++) {
      double ang = k * (pi / 2.0);
      double bx = cage_r * cos(ang);
      double bz = cage_r * sin(ang);
      torch_parts->add(make_shared<cylinder>(point3(bx, cage_start_y, bz),
                                             vec3(0, 1, 0), 1.2, cage_h,
                                             mat_iron, "Animal_Lantern_Bar"));
    }

    double cap_start_y = cage_start_y + cage_h;
    torch_parts->add(make_shared<cylinder>(point3(0, cap_start_y, 0),
                                           vec3(0, 1, 0), 13, 3, mat_iron,
                                           "Animal_Lantern_Cap_Base"));

    auto cap_cone = cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 14, 8,
                                    mat_iron, "Animal_Lantern_Cap_Roof");
    torch_parts->add(
        translate_object(make_shared<cone>(cap_cone), 0, cap_start_y + 3, 0));

    auto flame_outer =
        cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 8, 20, mat_torch_flame,
                        "Animal_Lantern_Flame_Outer");
    torch_parts->add(
        translate_object(make_shared<cone>(flame_outer), 0, cage_start_y, 0));

    auto flame_inner =
        cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 4, 15, mat_torch_core,
                        "Animal_Lantern_Flame_Inner");
    torch_parts->add(translate_object(make_shared<cone>(flame_inner), 0,
                                      cage_start_y + 1, 0));

    auto t_trans = register_transformable(
        torch_parts, "Animal_Lantern_" + to_string(i + 1), vec3(tx, 0, tz));

    world.add(t_trans);

    auto torch_light =
        make_shared<point_light>(point3(tx, cage_start_y + (cage_h * 0.5), tz),
                                 color(1.0, 0.6, 0.2), 2.5, 0.001, 0.00004);
    lights.push_back(torch_light);
    firefly_lights.push_back(torch_light);
  }

  cout << "[Animais] Animais NOTURNOS adicionados (Vagalumes, Alcateia, Urso, "
          "Tochas)\n";
}

void toggle_day_night(bool set_to_night) {
  is_night_mode = set_to_night;
  remove_animals();
  setup_lighting();

  if (is_night_mode) {
    add_day_animals();
  } else {
    add_night_animals();
  }

  build_scene_bvh();

  cout << "[Ambiente] Modo alterado para: " << (is_night_mode ? "DIA" : "NOITE")
       << "\n";
  need_redraw = true;
}

void apply_vanishing_point_preset(int preset) {

  const double SWORD_X = 900.0;
  const double SWORD_Y = 150.0;
  const double SWORD_Z = 900.0;
  point3 sword_center(SWORD_X, SWORD_Y, SWORD_Z);

  vanishing_points_preset = preset;

  switch (preset) {
  case 0:
    cout << "[Perspectiva] Modo LIVRE\n";
    break;

  case 1:
    cam_eye = point3(SWORD_X, SWORD_Y, SWORD_Z - 130);
    cam_at = sword_center;
    cam_up = vec3(0, 1, 0);
    cout << "[Perspectiva] 1 PONTO DE FUGA - Vista frontal\n";
    break;

  case 2:
    cam_eye = point3(SWORD_X + 60, SWORD_Y, SWORD_Z - 80);
    cam_at = sword_center;
    cam_up = vec3(0, 1, 0);
    cout << "[Perspectiva] 2 PONTOS DE FUGA - Vista diagonal\n";
    break;

  case 3:
    cam_eye = point3(SWORD_X + 50, SWORD_Y - 60, SWORD_Z - 90);
    cam_at = point3(SWORD_X, SWORD_Y + 80, SWORD_Z);
    cam_up = vec3(0, 1, 0);
    cout << "[Perspectiva] 3 PONTOS DE FUGA - Vista de baixo\n";
    break;
  }

  if (preset > 0) {
    setup_camera();
  }
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

  if (!is_night_mode) {
    add_night_animals();
  } else {
    add_day_animals();
  }
}
