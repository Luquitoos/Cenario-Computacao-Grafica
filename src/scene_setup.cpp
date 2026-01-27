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

// ============== ANIMAIS ==============

void remove_animals() {
  // Remove objetos com nomes de animais
  world.remove_by_name_prefix("Animal_");
  // Limpa luzes dos vagalumes
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
  const double WX = 710.0;
  const double WZ = 1090.0;

  // Materiais para animais
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

  // === BORBOLETAS GEOMÉTRICAS (3) - Voando ao redor da Espada ===
  // Espada esta em (CX, 195, CZ). Vamos colocar em volta.
  double sword_y = 195.0;

  // Cores das asas: Vermelho, Rosa, Roxo (Sem Verde)
  auto mat_wing_red =
      make_shared<material>(color(0.9, 0.1, 0.1), 0.2, 0.8, 80.0, "Wing Red");
  auto mat_wing_pink =
      make_shared<material>(color(1.0, 0.4, 0.7), 0.2, 0.8, 80.0, "Wing Pink");
  auto mat_wing_purple = make_shared<material>(color(0.6, 0.2, 0.9), 0.2, 0.8,
                                               80.0, "Wing Purple");

  shared_ptr<material> wing_mats[] = {mat_wing_red, mat_wing_pink,
                                      mat_wing_purple};

  double butterfly_offsets[][3] = {
      {30, 40, 20},   // Perto do punho/lamina
      {-40, 60, -30}, // Mais alto
      {20, 50, -50}   // Outro lado
  };

  for (int i = 0; i < 3; i++) {
    // Posicao relativa a Espada
    double bx = CX + butterfly_offsets[i][0];
    double by = sword_y + butterfly_offsets[i][1];
    double bz = CZ + butterfly_offsets[i][2];

    // Angulo base para olhar o centro (ou aleatorio)
    double rot_angle_y = i * 120.0;

    // Inclinacao (Banking) para parecer voo natural
    // P.ex: inclinar Z (roll) e X (pitch)
    double bank_z = (i % 2 == 0) ? 25.0 : -25.0;
    double pitch_x = 15.0;

    auto wing_mat = wing_mats[i % 3]; // Variar cor

    auto butterfly_parts = make_shared<hittable_list>();

    // 1. Corpo (Cilindro Fino e Antenas)
    auto body_mesh =
        make_shared<cylinder>(point3(0, -2, 0), vec3(0, 1, 0), 0.4, 5,
                              mat_butterfly_body, "Butterfly Body");
    mat4 body_base_T = mat4::rotate_x(degrees_to_radians(30));
    mat4 body_base_Tinv = mat4::rotate_x_inverse(degrees_to_radians(30));
    butterfly_parts->add(
        make_shared<class transform>(body_mesh, body_base_T, body_base_Tinv));

    // Antenas
    auto ant_mesh =
        make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0.5), 0.1, 1.5,
                              mat_butterfly_body, "Butterfly Antenna");
    butterfly_parts->add(
        translate_object(make_shared<cylinder>(*ant_mesh), -0.3, 2.5, 1));
    butterfly_parts->add(
        translate_object(make_shared<cylinder>(*ant_mesh), 0.3, 2.5, 1));

    // 2. Asas (Geometricas)
    // Asa superior base
    auto wing_up_mesh = make_shared<box_mesh>(
        point3(0, 0, -0.1), point3(5, 4, 0.1), wing_mat, "Butterfly Wing Up");
    mat4 shear_U = mat4::shear(0.5, 0, 0, 0, 0, 0);
    mat4 shear_Uinv = mat4::shear_inverse(0.5, 0, 0, 0, 0, 0);

    // Asa inferior base
    auto wing_low_mesh = make_shared<box_mesh>(
        point3(0, -3, -0.1), point3(3, 0, 0.1), wing_mat, "Butterfly Wing Low");
    mat4 shear_L = mat4::shear(-0.2, 0, 0, 0, 0, 0);
    mat4 shear_Linv = mat4::shear_inverse(-0.2, 0, 0, 0, 0, 0);

    // Lado DIREITO (X+)
    auto wing_R_Group = make_shared<hittable_list>();
    wing_R_Group->add(
        make_shared<class transform>(wing_up_mesh, shear_U, shear_Uinv));
    wing_R_Group->add(
        make_shared<class transform>(wing_low_mesh, shear_L, shear_Linv));
    mat4 wing_R_Final = mat4::rotate_z(degrees_to_radians(-30));
    mat4 wing_R_FinalInv = mat4::rotate_z_inverse(degrees_to_radians(-30));
    butterfly_parts->add(make_shared<class transform>(
        wing_R_Group, wing_R_Final, wing_R_FinalInv));

    // Lado ESQUERDO (X-)
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

    // === Transformação FINAL para o Mundo ===
    mat4 w_T = mat4::translate(bx, by, bz);
    mat4 w_Tinv = mat4::translate_inverse(bx, by, bz);

    // Rotacao Combinada (Y + Pitch/Roll para voo)
    mat4 w_R_Y = mat4::rotate_y(degrees_to_radians(rot_angle_y));
    mat4 w_R_Pitch =
        mat4::rotate_x(degrees_to_radians(pitch_x)); // Inclina pra frente/tras
    mat4 w_R_Roll =
        mat4::rotate_z(degrees_to_radians(bank_z)); // Inclina lateralmente

    // Ordem: Roll -> Pitch -> Y (Yaw)
    mat4 w_R = w_R_Y * w_R_Pitch * w_R_Roll;
    mat4 w_Rinv = mat4::rotate_z_inverse(degrees_to_radians(bank_z)) *
                  mat4::rotate_x_inverse(degrees_to_radians(pitch_x)) *
                  mat4::rotate_y_inverse(degrees_to_radians(rot_angle_y));

    double sc = 1.0 + random_double(-0.1, 0.1);
    mat4 w_S = mat4::scale(sc, sc, sc);
    mat4 w_Sinv = mat4::scale_inverse(sc, sc, sc);

    world.add(make_shared<class transform>(butterfly_parts, w_T * w_R * w_S,
                                           w_Sinv * w_Rinv * w_Tinv));
  }

  // === COELHOS GEOMÉTRICOS (5) ===
  // Movemos para raio ~230-240 para evitar colisao com Lake Rocks (que vao ate
  // ~210) [X, Z] apenas, direcao calculada dinamicamente
  double rabbit_pos[][2] = {
      {928.844, 682.594},   // (Ex-Leste) Solicitado pelo usuario
      {CX - 230, CZ},       // Oeste
      {CX, CZ + 230},       // Sul
      {CX + 165, CZ - 165}, // Nordeste
      {834.44, 687.431}     // (Ex-Cachoeira) Solicitado pelo usuario
  };

  for (int i = 0; i < 5; i++) {
    double rx = rabbit_pos[i][0];
    double rz = rabbit_pos[i][1];

    // Calcular rotação para olhar para o centro do lago (CX, CZ)
    // Vetor Rabbit -> Lake
    double dx = CX - rx;
    double dz = CZ - rz;
    // Normalizar nao é estritamente necessario para atan2, mas ajuda na clareza
    // atan2(x, z) pois Z+ é "Forward" no modelo base?
    // Modelo base olha Z+, entao queremos rotacionar para alinhar Z+ com (dx,
    // dz)
    double angle_rad = atan2(dx, dz);

    auto rabbit_parts = make_shared<hittable_list>();

    // 1. Corpo (Principal) - Inclinado (Sentado)
    // Usar uma esfera alongada ou cilindro
    auto body_mesh =
        make_shared<sphere>(point3(0, 0, 0), 9, mat_rabbit, "Rabbit Body Main");
    // Escalar para ser oval
    mat4 body_S = mat4::scale(0.8, 1.0, 1.4);
    mat4 body_Sinv = mat4::scale_inverse(0.8, 1.0, 1.4);
    // Ergue a traseira um pouco (Sentado/Alerta)
    mat4 body_R = mat4::rotate_x(degrees_to_radians(-20));
    mat4 body_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-20));
    mat4 body_T = mat4::translate(0, 10, 0);
    mat4 body_Tinv = mat4::translate_inverse(0, 10, 0);
    rabbit_parts->add(
        make_shared<class transform>(body_mesh, body_T * body_R * body_S,
                                     body_Sinv * body_Rinv * body_Tinv));

    // 2. Coxas Traseiras (Grandes, laterais)
    // Esferas achatadas nas laterais traseiras
    auto thigh_mesh =
        make_shared<sphere>(point3(0, 0, 0), 7, mat_rabbit, "Rabbit Thigh");
    mat4 thigh_S = mat4::scale(0.6, 1.2, 1.2);
    mat4 thigh_Sinv = mat4::scale_inverse(0.6, 1.2, 1.2);
    // Coxa Esq
    mat4 thigh_L_T = mat4::translate(-6, 8, -5);
    mat4 thigh_L_Tinv = mat4::translate_inverse(-6, 8, -5);
    rabbit_parts->add(make_shared<class transform>(
        thigh_mesh, thigh_L_T * thigh_S, thigh_Sinv * thigh_L_Tinv));
    // Coxa Dir
    mat4 thigh_R_T = mat4::translate(6, 8, -5);
    mat4 thigh_R_Tinv = mat4::translate_inverse(6, 8, -5);
    rabbit_parts->add(make_shared<class transform>(
        thigh_mesh, thigh_R_T * thigh_S, thigh_Sinv * thigh_R_Tinv));

    // 3. Peito/Pescoço
    auto chest =
        make_shared<sphere>(point3(0, 14, 8), 6, mat_rabbit, "Rabbit Chest");
    rabbit_parts->add(chest);

    // 4. Cabeça (Geométrica)
    // Usar esfera menor como base, mais alta
    auto head_mesh =
        make_shared<sphere>(point3(0, 0, 0), 6.5, mat_rabbit, "Rabbit Head");
    mat4 head_T = mat4::translate(0, 24, 12); // Mais alto e para frente
    mat4 head_Tinv = mat4::translate_inverse(0, 24, 12);
    rabbit_parts->add(
        make_shared<class transform>(head_mesh, head_T, head_Tinv));

    // Focinho (Cone apontando para frente/baixo)
    auto snout = cone::from_base(point3(0, 0, 0), vec3(0, -0.2, 1), 3.5, 7,
                                 mat_rabbit, "Rabbit Snout");
    rabbit_parts->add(translate_object(make_shared<cone>(snout), 0, 23, 16));

    // 5. Orelhas (No TOPO da cabeça - Parallelogramas)
    // Centro da cabeça em (0, 24, 12), Raio 6.5. Topo ~ (0, 30.5, 12)
    // Usar Box Mesh + Shear para fazer paralelogramo
    // Box: base (2.0 x 12 x 1.2) - Mais grossas
    auto ear_mesh_base =
        make_shared<box_mesh>(point3(-1.0, 0, -0.6), point3(1.0, 12, 0.6),
                              mat_rabbit, "Rabbit Ear Mesh");

    // Shear para "Paralelogramo" - Inclinar X com Y
    mat4 ear_shear = mat4::shear(0.3, 0, 0, 0, 0, 0); // X aumenta com Y
    mat4 ear_shearinv = mat4::shear_inverse(0.3, 0, 0, 0, 0, 0);

    // Orelha Esq
    // Rotacionar para trás (-X) e inclinar (Z-)
    mat4 ear_L_R =
        mat4::rotate_z(degrees_to_radians(25)) *
        mat4::rotate_x(degrees_to_radians(-20)); // Abre pra esq e pra tras
    mat4 ear_L_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-20)) *
                      mat4::rotate_z_inverse(degrees_to_radians(25));
    // Translate para o topo da cabeça
    mat4 ear_L_T = mat4::translate(-2.5, 30, 11);
    mat4 ear_L_Tinv = mat4::translate_inverse(-2.5, 30, 11);

    rabbit_parts->add(make_shared<class transform>(
        ear_mesh_base, ear_L_T * ear_L_R * ear_shear,
        ear_shearinv * ear_L_Rinv * ear_L_Tinv));

    // Orelha Dir
    mat4 ear_R_R =
        mat4::rotate_z(degrees_to_radians(-25)) *
        mat4::rotate_x(degrees_to_radians(-20)); // Abre pra dir e pra tras
    mat4 ear_R_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-20)) *
                      mat4::rotate_z_inverse(degrees_to_radians(-25));
    mat4 ear_R_T = mat4::translate(2.5, 30, 11);
    mat4 ear_R_Tinv = mat4::translate_inverse(2.5, 30, 11);

    rabbit_parts->add(make_shared<class transform>(
        ear_mesh_base, ear_R_T * ear_R_R * ear_shear,
        ear_shearinv * ear_R_Rinv * ear_R_Tinv));

    // 6. Patas Dianteiras (Inclinadas como no origami)
    auto leg_front =
        make_shared<cylinder>(point3(0, 0, 0), vec3(0, -1, 0.3), 1.5, 14,
                              mat_rabbit, "Rabbit Front Leg");
    rabbit_parts->add(translate_object(make_shared<cylinder>(*leg_front), -3.0,
                                       15, 11)); // Esq
    rabbit_parts->add(translate_object(make_shared<cylinder>(*leg_front), 3.0,
                                       15, 11)); // Dir

    // 7. Cauda (Pompom geometrico)
    rabbit_parts->add(
        make_shared<sphere>(point3(0, 8, -9), 3.5, mat_rabbit, "Rabbit Tail"));

    // === Transformação Final para o Mundo ===
    // Posiciona em (rx, 0, rz) e Rotaciona pelo angulo

    mat4 world_T = mat4::translate(rx, 0, rz);
    mat4 world_Tinv = mat4::translate_inverse(rx, 0, rz);
    mat4 world_R = mat4::rotate_y(angle_rad);
    mat4 world_Rinv = mat4::rotate_y_inverse(angle_rad);

    // Scale global se necessário (Coelho pequeno)
    // O modelo foi feito em unidades ~20-30 de altura. Está ok.

    // Ordem: T * R * (LocalModel)
    auto final_rabbit = make_shared<class transform>(
        rabbit_parts, world_T * world_R, world_Rinv * world_Tinv);
    final_rabbit->name = "Animal_Rabbit_Geo_" + to_string(i + 1);

    world.add(final_rabbit);
  }

  // === PÁSSAROS GEOMÉTRICOS (Origami Style) (4) - Area Especifica e Cores ===

  // Cores: Marrom, Branco (Pontas), Preto (Bico/Olhos)
  auto mat_bird_brown = make_shared<material>(color(0.5, 0.35, 0.2), 0.3, 0.5,
                                              40.0, "Bird Brown");
  auto mat_bird_white =
      make_shared<material>(color(0.9, 0.9, 0.9), 0.3, 0.5, 40.0, "Bird White");
  auto mat_bird_black =
      make_shared<material>(color(0.1, 0.1, 0.1), 0.1, 0.1, 10.0, "Bird Black");

  // Pontos limitantes fornecidos (aprox):
  // P1(1098, 0, 820), P2(735, 0, 686), P3(804, 53, 846), P4(1120, 0, 948)
  // Vamos usar esses pontos como ancoras para os 4 passaros, com altura
  // ajustada para voo. User deu altura 0 ou 53. Se estao voando, devem estar
  // mais altos? O prompt anterior dizia "voando pela cena". Vou colocar alturas
  // de voo (150-250) sobre esses pontos X,Z. Ou o user quer que fiquem
  // POUSEADOS nessas coordenadas exatas? "Posicione os passaros na area
  // delimitada de..." sugere voar NA AREA. Vou manter voo alto sobre essa area
  // XZ.

  double bird_positions[][3] = {
      {1098.42, 220, 819.919},
      {734.744, 250, 686.032},
      {804.178, 200, 845.856}, // Esse ponto tinha Y=53, talvez um voo rasante?
                               // Vou por 200 padrao p/ garantir visibilidade
      {1120.82, 230, 947.773}};

  for (int i = 0; i < 4; i++) {
    double px = bird_positions[i][0];
    double py = bird_positions[i][1];
    double pz = bird_positions[i][2];
    double rot_y = random_double(0, 360);

    auto bird_parts = make_shared<hittable_list>();

    // 1. Corpo (Marrom)
    auto body_mesh = make_shared<box_mesh>(point3(-3, -2, -6), point3(3, 2, 6),
                                           mat_bird_brown, "Bird Body");
    mat4 body_R = mat4::rotate_x(degrees_to_radians(10));
    mat4 body_Rinv = mat4::rotate_x_inverse(degrees_to_radians(10));
    bird_parts->add(make_shared<class transform>(body_mesh, body_R, body_Rinv));

    // Detalhe Branco (Barra/Barriga)
    // Box achatado embaixo
    auto belly_mesh =
        make_shared<box_mesh>(point3(-2.5, -2.2, -5), point3(2.5, -1.8, 5),
                              mat_bird_white, "Bird Belly");
    bird_parts->add(make_shared<class transform>(
        belly_mesh, body_R, body_Rinv)); // Segue rotacao corpo

    // 2. Asas (Marrom base + Ponta Branca)
    // Geometria Base da Asa
    auto wing_base_geo = make_shared<box_mesh>(
        point3(0, 0, -3), point3(8, 1, 4), mat_bird_brown, "Wing Base");
    auto wing_tip_geo =
        make_shared<box_mesh>(point3(8, 0, -3), point3(12, 1, 4),
                              mat_bird_white, "Wing Tip"); // Ponta branca

    mat4 wing_shear = mat4::shear(0, 0, 0.5, 0, 0, 0);
    mat4 wing_shearinv = mat4::shear_inverse(0, 0, 0.5, 0, 0, 0);

    // Grupo Asa (Base + Ponta)
    auto wing_comp = make_shared<hittable_list>();
    wing_comp->add(wing_base_geo);
    wing_comp->add(wing_tip_geo);

    // Asa Esq
    mat4 wing_L_T = mat4::translate(-2, 1, 0);
    mat4 wing_L_Tinv = mat4::translate_inverse(-2, 1, 0);
    mat4 wing_L_R = mat4::rotate_z(degrees_to_radians(20)) *
                    mat4::rotate_y(degrees_to_radians(180));
    mat4 wing_L_Rinv = mat4::rotate_y_inverse(degrees_to_radians(180)) *
                       mat4::rotate_z_inverse(degrees_to_radians(20));
    bird_parts->add(make_shared<class transform>(
        wing_comp, wing_L_T * wing_L_R * wing_shear,
        wing_shearinv * wing_L_Rinv * wing_L_Tinv));

    // Asa Dir
    mat4 wing_R_T = mat4::translate(2, 1, 0);
    mat4 wing_R_Tinv = mat4::translate_inverse(2, 1, 0);
    mat4 wing_R_R = mat4::rotate_z(degrees_to_radians(-20));
    mat4 wing_R_Rinv = mat4::rotate_z_inverse(degrees_to_radians(-20));
    bird_parts->add(make_shared<class transform>(
        wing_comp, wing_R_T * wing_R_R * wing_shear,
        wing_shearinv * wing_R_Rinv * wing_R_Tinv));

    // 3. Cauda (Marrom com ponta branca?)
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

    // 4. Cabeça (Marrom), Bico (Preto), Olhos (Preto)
    // Cabeca
    auto head_mesh = make_shared<box_mesh>(
        point3(-2, -2, -2.5), point3(2, 2, 2.5), mat_bird_brown, "Bird Head");
    mat4 head_T = mat4::translate(0, 2, 7);
    mat4 head_Tinv = mat4::translate_inverse(0, 2, 7);
    bird_parts->add(make_shared<class transform>(head_mesh, head_T, head_Tinv));

    // Bico (Preto)
    auto beak_mesh = make_shared<cone>(point3(0, 0, 0), vec3(0, 0, 1), 1.0, 3.0,
                                       mat_bird_black, "Bird Beak");
    mat4 beak_R = mat4::rotate_x(degrees_to_radians(90));
    mat4 beak_Rinv = mat4::rotate_x_inverse(degrees_to_radians(90));
    mat4 beak_T = mat4::translate(0, 2, 9.5);
    mat4 beak_Tinv = mat4::translate_inverse(0, 2, 9.5);
    bird_parts->add(make_shared<class transform>(beak_mesh, beak_T * beak_R,
                                                 beak_Rinv * beak_Tinv));

    // Olhos (Esferas Pretas)
    auto eye_mesh =
        make_shared<sphere>(point3(0, 0, 0), 0.8, mat_bird_black, "Bird Eye");
    // Olho Esq
    bird_parts->add(
        translate_object(make_shared<sphere>(*eye_mesh), -2.1, 3, 8));
    // Olho Dir
    bird_parts->add(
        translate_object(make_shared<sphere>(*eye_mesh), 2.1, 3, 8));

    // === Transformação Final ===
    mat4 w_T = mat4::translate(px, py, pz);
    mat4 w_Tinv = mat4::translate_inverse(px, py, pz);
    mat4 w_R = mat4::rotate_y(degrees_to_radians(rot_y));
    mat4 w_Rinv = mat4::rotate_y_inverse(degrees_to_radians(rot_y));

    double bank = random_double(-15, 15);
    mat4 w_R_Bank = mat4::rotate_z(degrees_to_radians(bank));
    mat4 w_R_BankInv = mat4::rotate_z_inverse(degrees_to_radians(bank));

    world.add(make_shared<class transform>(bird_parts, w_T * w_R * w_R_Bank,
                                           w_R_BankInv * w_Rinv * w_Tinv));
  }

  // === CAVALO - perto da cachoeira ===
  double hx = WX - 60, hy = 5, hz = WZ + 50;
  // Corpo
  world.add(make_shared<sphere>(point3(hx, hy + 35, hz), 30, mat_horse_body,
                                "Animal_Horse_Body"));
  // Pescoço (cilindro inclinado simulado)
  world.add(make_shared<cylinder>(point3(hx + 25, hy + 45, hz), vec3(0.5, 1, 0),
                                  8, 35, mat_horse_body, "Animal_Horse_Neck"));
  // Cabeça
  world.add(make_shared<sphere>(point3(hx + 40, hy + 75, hz), 12,
                                mat_horse_body, "Animal_Horse_Head"));
  // Focinho
  world.add(make_shared<sphere>(point3(hx + 52, hy + 70, hz), 6, mat_horse_body,
                                "Animal_Horse_Snout"));
  // Pernas (4)
  world.add(make_shared<cylinder>(point3(hx - 15, hy, hz - 12), vec3(0, 1, 0),
                                  4, 30, mat_horse_body, "Animal_Horse_Leg"));
  world.add(make_shared<cylinder>(point3(hx - 15, hy, hz + 12), vec3(0, 1, 0),
                                  4, 30, mat_horse_body, "Animal_Horse_Leg"));
  world.add(make_shared<cylinder>(point3(hx + 15, hy, hz - 12), vec3(0, 1, 0),
                                  4, 30, mat_horse_body, "Animal_Horse_Leg"));
  world.add(make_shared<cylinder>(point3(hx + 15, hy, hz + 12), vec3(0, 1, 0),
                                  4, 30, mat_horse_body, "Animal_Horse_Leg"));
  // Cauda
  world.add(make_shared<cylinder>(point3(hx - 30, hy + 35, hz),
                                  vec3(-1, -0.5, 0), 3, 25, mat_horse_mane,
                                  "Animal_Horse_Tail"));

  cout << "[Animais] Animais DIURNOS adicionados\n";
}

void add_night_animals() {
  const double CX = 900.0;
  const double CZ = 900.0;
  // WX e WZ removidos (unused)

  // Materiais para animais noturnos
  // Vagalume (luz)
  auto mat_firefly_light = make_shared<material>(color(0.8, 0.9, 0.3), 0.1, 0.9,
                                                 100.0, "Firefly Light");
  mat_firefly_light->emission =
      color(0.5, 0.6, 0.2); // Emissão para brilho próprio

  // Vagalume (corpo preto)
  auto mat_firefly_body = make_shared<material>(color(0.05, 0.05, 0.05), 0.1,
                                                0.1, 10.0, "Firefly Body");

  auto mat_wolf =
      make_shared<material>(color(0.35, 0.35, 0.4), 0.4, 0.5, 40.0, "Wolf Fur");
  auto mat_bear =
      make_shared<material>(color(0.3, 0.2, 0.15), 0.5, 0.4, 30.0, "Bear Fur");

  // === VAGALUMES (20) com luzes e corpo preto ===
  for (int i = 0; i < 20; i++) {
    double fx = CX + random_double(-250, 250);
    double fy = random_double(30, 150);
    double fz = CZ + random_double(-250, 250);

    // Luz do vagalume (bunda) - Esfera luminosa
    world.add(make_shared<sphere>(point3(fx, fy, fz), 2, mat_firefly_light,
                                  "Animal_Firefly_Light"));

    // Corpo do vagalume (cabeça preta) - Esfera pequena ao lado da luz
    world.add(make_shared<sphere>(point3(fx + 1.5, fy + 0.5, fz + 1.5), 1.5,
                                  mat_firefly_body, "Animal_Firefly_Body"));

    // Luz do vagalume (amarelo/verde)
    double r = random_double(0.3, 0.5);
    double g = random_double(0.5, 0.7);
    double b = random_double(0.1, 0.2);
    auto firefly_light =
        make_shared<point_light>(point3(fx, fy, fz), color(r, g, b),
                                 0.3,  // intensidade base
                                 0.05, // atenuação linear alta
                                 0.005 // atenuação quadrática
        );
    lights.push_back(firefly_light);
    firefly_lights.push_back(firefly_light);
  }

  // === ALCATEIA DE LOBOS (3) - Olhando para o Stream Lake ===
  // Posição do líder
  double leader_x = 833.7;
  double leader_z = 685.8;

  // Vetor direção para o lago (900, 900)
  // V = (900 - 833.7, 900 - 685.8) = (66.3, 214.2)
  // Normalizado approx: (0.3, 0.95)
  double wdx = 0.3;
  double wdz = 0.95;
  // wrx e wrz removidos (unused)

  // Offsets para os 3 lobos: Líder, Esq, Dir
  struct WolfOffset {
    double off_x, off_z;
  };
  WolfOffset wolves[] = {
      {0, 0},     // Líder
      {-25, -20}, // Seguidor esquerda
      {25, -20}   // Seguidor direita
  };

  for (int i = 0; i < 3; i++) {
    // === LOBO GEOMÉTRICO (Em pé e Alerta) ===
    // Posição base do lobo atual
    double base_x = leader_x + wolves[i].off_x;
    double base_z = leader_z + wolves[i].off_z;
    double ly = 12.0; // Altura do chão até o peito

    auto wolf_parts = make_shared<hittable_list>();

    // 1. Torso (Dividido em Peito e Flanco para dar forma)
    // Peito (Mais largo na frente)
    auto chest_mesh = make_shared<box_mesh>(point3(-5, -5, -8), point3(5, 7, 8),
                                            mat_wolf, "Wolf Chest");
    mat4 chest_T = mat4::translate(0, 0, 5); // Levemente a frente
    mat4 chest_Tinv = mat4::translate_inverse(0, 0, 5);
    // Inclinar peito para cima um pouco? Nao, reto para estar "Alerta/Em pe"
    wolf_parts->add(
        make_shared<class transform>(chest_mesh, chest_T, chest_Tinv));

    // Flanco (Traseira, mais fina)
    auto flank_mesh = make_shared<box_mesh>(point3(-4, -4, -7), point3(4, 6, 7),
                                            mat_wolf, "Wolf Flank");
    mat4 flank_T = mat4::translate(0, 1, -8); // Atras e levemente acima
    mat4 flank_Tinv = mat4::translate_inverse(0, 1, -8);
    wolf_parts->add(
        make_shared<class transform>(flank_mesh, flank_T, flank_Tinv));

    // 2. Pescoço (Longo e inclinado para frente/cima)
    auto neck_mesh = make_shared<box_mesh>(point3(-3, 0, -3), point3(3, 10, 3),
                                           mat_wolf, "Wolf Neck");
    mat4 neck_T = mat4::translate(0, 5, 10);
    mat4 neck_Tinv = mat4::translate_inverse(0, 5, 10);
    mat4 neck_R = mat4::rotate_x(degrees_to_radians(-40)); // Inclina 40 graus
    mat4 neck_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-40));
    wolf_parts->add(make_shared<class transform>(neck_mesh, neck_T * neck_R,
                                                 neck_Rinv * neck_Tinv));

    // 3. Cabeça (Geométrica)
    // Cranio
    auto head_mesh = make_shared<box_mesh>(point3(-4, -3, -4), point3(4, 5, 4),
                                           mat_wolf, "Wolf Head");
    mat4 head_T = mat4::translate(0, 14, 16);
    mat4 head_Tinv = mat4::translate_inverse(0, 14, 16);
    wolf_parts->add(make_shared<class transform>(head_mesh, head_T, head_Tinv));

    // Focinho (Prisma Tapered - simulado com box menor)
    auto snout_mesh = make_shared<box_mesh>(
        point3(-2, -1.5, 0), point3(2, 2.5, 6), mat_wolf, "Wolf Snout");
    mat4 snout_T = mat4::translate(0, 14, 19);
    mat4 snout_Tinv = mat4::translate_inverse(0, 14, 19);
    wolf_parts->add(
        make_shared<class transform>(snout_mesh, snout_T, snout_Tinv));

    // Orelhas (Triangulares/Sheared)
    auto ear_mesh = make_shared<box_mesh>(
        point3(-1.5, 0, -1), point3(1.5, 5, 1), mat_wolf, "Wolf Ear");
    mat4 ear_shear =
        mat4::shear(0, 0, 0, 0, 0.4, 0); // Z aumenta com Y (Inclina pra tras)
    mat4 ear_shearinv = mat4::shear_inverse(0, 0, 0, 0, 0.4, 0);
    // Esq
    mat4 ear_L_T = mat4::translate(-2.5, 19, 17);
    mat4 ear_L_Tinv = mat4::translate_inverse(-2.5, 19, 17);
    wolf_parts->add(make_shared<class transform>(ear_mesh, ear_L_T * ear_shear,
                                                 ear_shearinv * ear_L_Tinv));
    // Dir
    mat4 ear_R_T = mat4::translate(2.5, 19, 17);
    mat4 ear_R_Tinv = mat4::translate_inverse(2.5, 19, 17);
    wolf_parts->add(make_shared<class transform>(ear_mesh, ear_R_T * ear_shear,
                                                 ear_shearinv * ear_R_Tinv));

    // 4. Pernas (Articuladas)
    auto leg_upper = make_shared<box_mesh>(point3(-2, -6, -2), point3(2, 2, 2),
                                           mat_wolf, "Wolf Leg Upper");
    auto leg_lower =
        make_shared<box_mesh>(point3(-1.5, -8, -1.5), point3(1.5, 0, 1.5),
                              mat_wolf, "Wolf Leg Lower");

    // Dianteira Esq
    mat4 leg_FL_T = mat4::translate(-4, 0, 10);
    mat4 leg_FL_Tinv = mat4::translate_inverse(-4, 0, 10);
    wolf_parts->add(make_shared<class transform>(leg_upper, leg_FL_T,
                                                 leg_FL_Tinv)); // Coxa reta
    mat4 leg_FL_Low_T = mat4::translate(-4, -6, 10);
    mat4 leg_FL_Low_Tinv = mat4::translate_inverse(-4, -6, 10);
    wolf_parts->add(make_shared<class transform>(
        leg_lower, leg_FL_Low_T, leg_FL_Low_Tinv)); // Canela reta

    // Dianteira Dir
    mat4 leg_FR_T = mat4::translate(4, 0, 10);
    mat4 leg_FR_Tinv = mat4::translate_inverse(4, 0, 10);
    wolf_parts->add(
        make_shared<class transform>(leg_upper, leg_FR_T, leg_FR_Tinv));
    mat4 leg_FR_Low_T = mat4::translate(4, -6, 10);
    mat4 leg_FR_Low_Tinv = mat4::translate_inverse(4, -6, 10);
    wolf_parts->add(
        make_shared<class transform>(leg_lower, leg_FR_Low_T, leg_FR_Low_Tinv));

    // Traseira Esq (Joelho para tras)
    mat4 leg_BL_T = mat4::translate(-3.5, 0, -12);
    mat4 leg_BL_Tinv = mat4::translate_inverse(-3.5, 0, -12);
    mat4 leg_BL_R = mat4::rotate_x(degrees_to_radians(15)); // Coxa pra tras
    mat4 leg_BL_Rinv = mat4::rotate_x_inverse(degrees_to_radians(15));
    wolf_parts->add(make_shared<class transform>(leg_upper, leg_BL_T * leg_BL_R,
                                                 leg_BL_Rinv * leg_BL_Tinv));

    // Canela Traseira (Angulada para frente - Hock joint)
    mat4 leg_BL_Low_T = mat4::translate(-3.5, -5.5, -13);
    mat4 leg_BL_Low_Tinv = mat4::translate_inverse(-3.5, -5.5, -13);
    mat4 leg_BL_Low_R = mat4::rotate_x(degrees_to_radians(-20));
    mat4 leg_BL_Low_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-20));
    wolf_parts->add(
        make_shared<class transform>(leg_lower, leg_BL_Low_T * leg_BL_Low_R,
                                     leg_BL_Low_Rinv * leg_BL_Low_Tinv));

    // Traseira Dir
    mat4 leg_BR_T = mat4::translate(3.5, 0, -12);
    mat4 leg_BR_Tinv = mat4::translate_inverse(3.5, 0, -12);
    wolf_parts->add(make_shared<class transform>(
        leg_upper, leg_BR_T * leg_BL_R,
        leg_BL_Rinv * leg_BR_Tinv)); // Recicla rotacao

    mat4 leg_BR_Low_T = mat4::translate(3.5, -5.5, -13);
    mat4 leg_BR_Low_Tinv = mat4::translate_inverse(3.5, -5.5, -13);
    wolf_parts->add(
        make_shared<class transform>(leg_lower, leg_BR_Low_T * leg_BL_Low_R,
                                     leg_BL_Low_Rinv * leg_BR_Low_Tinv));

    // 5. Cauda (Caida)
    auto tail_mesh = make_shared<box_mesh>(
        point3(-1.5, -8, -1.5), point3(1.5, 0, 1.5), mat_wolf, "Wolf Tail");
    mat4 tail_T = mat4::translate(0, 3, -14);
    mat4 tail_Tinv = mat4::translate_inverse(0, 3, -14);
    mat4 tail_R = mat4::rotate_x(degrees_to_radians(30)); // Pra tras/baixo
    mat4 tail_Rinv = mat4::rotate_x_inverse(degrees_to_radians(30));
    wolf_parts->add(make_shared<class transform>(tail_mesh, tail_T * tail_R,
                                                 tail_Rinv * tail_Tinv));

    // === Transformação FINAL para o Mundo ===
    mat4 w_T = mat4::translate(base_x, ly, base_z);
    mat4 w_Tinv = mat4::translate_inverse(base_x, ly, base_z);

    // Calcular angulo para o lago
    // O modelo base foi construido +Z (Frente). Mas a posicao diz wdx, wdz como
    // vetor direcao
    double wolf_angle = atan2(wdx, wdz);
    mat4 w_R = mat4::rotate_y(wolf_angle);
    mat4 w_Rinv = mat4::rotate_y_inverse(wolf_angle);

    mat4 w_S = mat4::scale(1.2, 1.2, 1.2);
    mat4 w_Sinv = mat4::scale_inverse(1.2, 1.2, 1.2);

    auto final_wolf = make_shared<class transform>(wolf_parts, w_T * w_R * w_S,
                                                   w_Sinv * w_Rinv * w_Tinv);
    final_wolf->name = "Animal_Wolf_Geometric";
    world.add(final_wolf);
  }

  // === URSO GEOMÉTRICO (Sentado e Low Poly) ===
  // Posição solicitada: (973.894, 8.09958, 1106.82)
  double ux = 973.894;
  double uy = 8.09958 + 2; // Ajuste leve para o chao
  double uz = 1106.82;

  // Vetor direção para o lago (900, 900)
  // V = (900 - 973.8, 900 - 1106.8) = (-73.8, -206.8)
  // Vetor direção para o lago (900, 900)
  // V = (900 - 973.8, 900 - 1106.8) = (-73.8, -206.8)
  double udx = -0.33;
  double udz = -0.94;
  // Vetor lateral (perpendicular à direita) necessário para as tochas
  double urx = udz;
  double urz = -udx;

  double bear_angle = atan2(udx, udz); // Rotacao Y

  auto bear_parts = make_shared<hittable_list>();

  // 1. Corpo Central (Tronco - Box Grande)
  // Formato meio trapezoidal (usando shear ou scale)
  auto body_mesh = make_shared<box_mesh>(
      point3(-25, 0, -20), point3(25, 60, 20), mat_bear, "Bear Body Main");
  // Inclinar levemente para frente
  mat4 body_R = mat4::rotate_x(degrees_to_radians(10));
  mat4 body_Rinv = mat4::rotate_x_inverse(degrees_to_radians(10));
  bear_parts->add(make_shared<class transform>(body_mesh, body_R, body_Rinv));

  // 2. Corcunda (Ombros - Box Superior)
  auto hump_mesh = make_shared<box_mesh>(
      point3(-18, 50, -15), point3(18, 70, 15), mat_bear, "Bear Hump");
  // Posicionar nas costas
  mat4 hump_T = mat4::translate(0, 0, -5);
  mat4 hump_Tinv = mat4::translate_inverse(0, 0, -5);
  bear_parts->add(make_shared<class transform>(hump_mesh, hump_T, hump_Tinv));

  // 3. Pernas Traseiras (Sentadas - Coxas grandes laterais)
  // Box achatado e rotacionado
  auto thigh_mesh = make_shared<box_mesh>(
      point3(-10, 0, -20), point3(10, 45, 10), mat_bear, "Bear Thigh");
  // Coxa Esq
  mat4 thigh_L_T = mat4::translate(-30, 0, -5);
  mat4 thigh_L_Tinv = mat4::translate_inverse(-30, 0, -5);
  mat4 thigh_L_R = mat4::rotate_z(degrees_to_radians(15)) *
                   mat4::rotate_x(degrees_to_radians(-30)); // Abre e inclina
  mat4 thigh_L_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-30)) *
                      mat4::rotate_z_inverse(degrees_to_radians(15));
  bear_parts->add(make_shared<class transform>(
      thigh_mesh, thigh_L_T * thigh_L_R, thigh_L_Rinv * thigh_L_Tinv));

  // Coxa Dir
  mat4 thigh_R_T = mat4::translate(30, 0, -5);
  mat4 thigh_R_Tinv = mat4::translate_inverse(30, 0, -5);
  mat4 thigh_R_R = mat4::rotate_z(degrees_to_radians(-15)) *
                   mat4::rotate_x(degrees_to_radians(-30));
  mat4 thigh_R_Rinv = mat4::rotate_x_inverse(degrees_to_radians(-30)) *
                      mat4::rotate_z_inverse(degrees_to_radians(-15));
  bear_parts->add(make_shared<class transform>(
      thigh_mesh, thigh_R_T * thigh_R_R, thigh_R_Rinv * thigh_R_Tinv));

  // Pés Traseiros
  auto foot_mesh = make_shared<box_mesh>(point3(-8, 0, -12), point3(8, 6, 12),
                                         mat_bear, "Bear Foot");
  // Esq
  mat4 foot_L_T = mat4::translate(-35, 0, 20);
  mat4 foot_L_Tinv = mat4::translate_inverse(-35, 0, 20);
  mat4 foot_L_R = mat4::rotate_y(degrees_to_radians(-20)); // Aponta pra fora
  mat4 foot_L_Rinv = mat4::rotate_y_inverse(degrees_to_radians(-20));
  bear_parts->add(make_shared<class transform>(foot_mesh, foot_L_T * foot_L_R,
                                               foot_L_Rinv * foot_L_Tinv));
  // Dir
  mat4 foot_R_T = mat4::translate(35, 0, 20);
  mat4 foot_R_Tinv = mat4::translate_inverse(35, 0, 20);
  mat4 foot_R_R = mat4::rotate_y(degrees_to_radians(20));
  mat4 foot_R_Rinv = mat4::rotate_y_inverse(degrees_to_radians(20));
  bear_parts->add(make_shared<class transform>(foot_mesh, foot_R_T * foot_R_R,
                                               foot_R_Rinv * foot_R_Tinv));

  // 4. Pernas Dianteiras (Colunas retas de apoio)
  auto arm_mesh = make_shared<box_mesh>(point3(-7, 0, -7), point3(7, 45, 7),
                                        mat_bear, "Bear Arm");
  // Esq
  mat4 arm_L_T = mat4::translate(-18, 0, 25);
  mat4 arm_L_Tinv = mat4::translate_inverse(-18, 0, 25);
  bear_parts->add(make_shared<class transform>(arm_mesh, arm_L_T, arm_L_Tinv));
  // Dir
  mat4 arm_R_T = mat4::translate(18, 0, 25);
  mat4 arm_R_Tinv = mat4::translate_inverse(18, 0, 25);
  bear_parts->add(make_shared<class transform>(arm_mesh, arm_R_T, arm_R_Tinv));

  // 5. Cabeça (Box Geométrico)
  auto head_base = make_shared<box_mesh>(
      point3(-14, -12, -14), point3(14, 12, 14), mat_bear, "Bear Head Base");
  mat4 head_T = mat4::translate(0, 65, 25);
  mat4 head_Tinv = mat4::translate_inverse(0, 65, 25);
  bear_parts->add(make_shared<class transform>(head_base, head_T, head_Tinv));

  // Focinho (Box menor na frente)
  auto snout_box = make_shared<box_mesh>(point3(-8, -6, 0), point3(8, 6, 12),
                                         mat_bear, "Bear Snout");
  mat4 snout_T = mat4::translate(0, 60, 39);
  mat4 snout_Tinv = mat4::translate_inverse(0, 60, 39);
  bear_parts->add(make_shared<class transform>(snout_box, snout_T, snout_Tinv));

  // Orelhas (Pequenas piramides/boxes)
  auto bear_ear = make_shared<box_mesh>(point3(-4, 0, -2), point3(4, 6, 2),
                                        mat_bear, "Bear Ear");
  // Esq
  mat4 bear_ear_L_T = mat4::translate(-12, 77, 20);
  mat4 bear_ear_L_Tinv = mat4::translate_inverse(-12, 77, 20);
  bear_parts->add(
      make_shared<class transform>(bear_ear, bear_ear_L_T, bear_ear_L_Tinv));
  // Dir
  mat4 bear_ear_R_T = mat4::translate(12, 77, 20);
  mat4 bear_ear_R_Tinv = mat4::translate_inverse(12, 77, 20);
  bear_parts->add(
      make_shared<class transform>(bear_ear, bear_ear_R_T, bear_ear_R_Tinv));

  // === Transformação FINAL para o Mundo ===
  mat4 w_T = mat4::translate(ux, uy, uz);
  mat4 w_Tinv = mat4::translate_inverse(ux, uy, uz);
  mat4 w_R = mat4::rotate_y(bear_angle);
  mat4 w_Rinv = mat4::rotate_y_inverse(bear_angle);

  // Escala se necessário (Urso deve ser GRANDE)
  mat4 w_S = mat4::scale(1.5, 1.5, 1.5);
  mat4 w_Sinv = mat4::scale_inverse(1.5, 1.5, 1.5);

  auto final_bear = make_shared<class transform>(bear_parts, w_T * w_R * w_S,
                                                 w_Sinv * w_Rinv * w_Tinv);
  final_bear->name = "Animal_Bear_Geometric";
  world.add(final_bear);

  // === TOCHAS MEDIEVAIS TIPO LANTERNA (Ao lado do Urso) ===
  // Materiais das Tochas
  auto mat_torch_flame = make_shared<material>(color(1.0, 0.55, 0.1), 0.95,
                                               0.25, 8.0, "Torch Flame");
  auto mat_torch_core = make_shared<material>(color(1.0, 0.85, 0.2), 0.98, 0.15,
                                              4.0, "Torch Core");
  auto mat_torch_pole = make_shared<material>(color(0.25, 0.15, 0.08), 0.12,
                                              0.04, 4.0, "Torch Pole");
  // Material de ferro escuro para a grade
  auto mat_iron = make_shared<material>(color(0.15, 0.15, 0.18), 0.2, 0.3, 20.0,
                                        "Lantern Iron");

  double torch_dist = 90.0; // Distância lateral
  // Posições P1 = P + Right*dist, P2 = P - Right*dist
  double t_pos[][2] = {{ux + urx * torch_dist, uz + urz * torch_dist},
                       {ux - urx * torch_dist, uz - urz * torch_dist}};
  double POLE_HEIGHT = 50.0; // Altura BEM menor

  for (int i = 0; i < 2; i++) {
    double tx = t_pos[i][0];
    double tz = t_pos[i][1];

    auto torch_parts = make_shared<hittable_list>();

    // 1. Poste (Mais curto)
    torch_parts->add(make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 4,
                                           POLE_HEIGHT, mat_torch_pole,
                                           "Animal_Lantern_Pole"));

    // 2. Base da Lanterna (Copo)
    // CylinderBaseStart = POLE_HEIGHT
    // CylinderBaseHeight = 4
    torch_parts->add(make_shared<cylinder>(point3(0, POLE_HEIGHT, 0),
                                           vec3(0, 1, 0), 12, 4, mat_iron,
                                           "Animal_Lantern_Base"));

    // 3. Grades (4 Barras verticais)
    // Starts ON TOP of Base (POLE_HEIGHT + 4)
    double lantern_base_h = 4.0;
    double cage_start_y = POLE_HEIGHT + lantern_base_h;
    double cage_h = 25.0;
    double cage_r = 10.0;

    for (int k = 0; k < 4; k++) {
      double ang = k * (pi / 2.0); // 0, 90, 180, 270
      double bx = cage_r * cos(ang);
      double bz = cage_r * sin(ang);
      torch_parts->add(make_shared<cylinder>(point3(bx, cage_start_y, bz),
                                             vec3(0, 1, 0), 1.2, cage_h,
                                             mat_iron, "Animal_Lantern_Bar"));
    }

    // 4. Topo da Lanterna (Tampa)
    // Starts ON TOP of Cage (cage_start_y + cage_h)
    double cap_start_y = cage_start_y + cage_h;
    torch_parts->add(make_shared<cylinder>(point3(0, cap_start_y, 0),
                                           vec3(0, 1, 0), 13, 3, mat_iron,
                                           "Animal_Lantern_Cap_Base"));

    // Telhado cônico pequeno
    // Starts ON TOP of Cap Base (cap_start_y + 3)
    auto cap_cone = cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 14, 8,
                                    mat_iron, "Animal_Lantern_Cap_Roof");
    torch_parts->add(
        translate_object(make_shared<cone>(cap_cone), 0, cap_start_y + 3, 0));

    // 5. Chama (Dentro da grade)
    // Starts ON TOP of Lantern Base (cage_start_y)
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

    // Transform position
    mat4 t_T = mat4::translate(tx, 0, tz);
    mat4 t_Tinv = mat4::translate_inverse(tx, 0, tz);
    auto t_trans = make_shared<class transform>(torch_parts, t_T, t_Tinv);
    t_trans->name = "Animal_Lantern_" + to_string(i + 1);
    world.add(t_trans);

    // Light Source (Laranja Vivido)
    // Posição: Centro da grade -> cage_start_y + (cage_h / 2)
    auto torch_light = make_shared<point_light>(
        point3(tx, cage_start_y + (cage_h * 0.5), tz), color(1.0, 0.6, 0.2),
        2.5,    // Intensidade
        0.001,  // Atenuação linear
        0.00004 // Atenuação quadrática
    );
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
    cam_eye = point3(SWORD_X, SWORD_Y + 40, SWORD_Z - 130);
    cam_at = sword_center;
    cam_up = vec3(0, 1, 0);
    cout << "[Perspectiva] 1 PONTO DE FUGA - Vista frontal\n";
    break;

  case 2:
    cam_eye = point3(SWORD_X + 60, SWORD_Y + 220, SWORD_Z - 80);
    cam_at = sword_center;
    cam_up = vec3(0, 1, 0);
    cout << "[Perspectiva] 2 PONTOS DE FUGA - Vista de cima\n";
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

  // Adiciona animais iniciais (invertido para corresponder à lógica dia/noite)
  if (!is_night_mode) {
    add_night_animals();
  } else {
    add_day_animals();
  }
}
