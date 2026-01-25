/*
 * TRABALHO DE COMPUTAÇÃO GRÁFICA - RAY CASTER
 * Tema: Espada na Pedra (Estilo Rei Arthur)
 *
 * Requisitos Atendidos:
 * - 1.3.1: Esfera, Cilindro, Cone, Malha
 * - 1.3.2: 4+ materiais distintos
 * - 1.3.3: Texturas (pedra, checker)
 * - 1.4: Translação, Rotação (eixo e arbitrária), Escala, Cisalhamento, Espelho
 * - 1.5: Luz Ambiente, Pontual, Spot, Direcional
 * - 2: Câmera com Eye, At, Up, d, janela
 * - 3: Projeção Perspectiva (zoom in/out), Ortográfica, Oblíqua
 * - 4: Sombras (shadow rays)
 * - 5: Pick + Interface Gráfica (GLUT)
 * - 6: Imagem 800x800 pixels
 */

#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/camera/camera.h"
#include "../include/cenario/hittable.h"
#include "../include/cenario/hittable_list.h"
#include "../include/cenario/light.h"
#include "../include/colors/color.h"
#include "../include/malha/mesh.h"
#include "../include/malha/triangle.h"
#include "../include/material/material.h"
#include "../include/object/cone.h"
#include "../include/object/cylinder.h"
#include "../include/object/plane.h"
#include "../include/object/sphere.h"
#include "../include/ray/ray.h"
#include "../include/textures/texture.h"
#include "../include/textures/utils.h"
#include "../include/transform/quaternion.h"
#include "../include/transform/transform.h"
#include "../include/vectors/mat4.h"
#include "../include/vectors/vec3.h"
#include "../include/vectors/vec4.h"
#include "../include/gui/gui_manager.h"

#include <GL/freeglut.h>

// ==================== CONFIGURAÇÕES GLOBAIS ====================
const int IMAGE_WIDTH = 400;
const int IMAGE_HEIGHT = 400;
unsigned char *PixelBuffer = nullptr;

// Cena
hittable_list world;
camera cam;
ambient_light ambient;
std::vector<std::shared_ptr<light>> lights;

// Estado da interface
int current_projection = 0; // 0=perspectiva, 1=ortográfica, 2=oblíqua
int vanishing_points = 3;   // 1, 2 ou 3 pontos de fuga
bool need_redraw = true;
std::string picked_object = "";

// Câmera interativa
// NOTA: Coordenadas ajustadas para primeiro octante (x, y, z >= 0)
point3 cam_eye(1050, 200, 750); // Posição inicial (400+650, 200, 100+650)
point3 cam_at(900, 100,
              900); // Olhando para o centro da cena (250+650, 100, 250+650)
double cam_speed = 30.0; // Velocidade de movimento

// ==================== MODELO DE ILUMINAÇÃO PHONG ====================
color calculate_lighting(const hit_record &rec, const ray &r,
                         const hittable_list &world) {
  color result(0, 0, 0);

  // Componente ambiente
  color diffuse_color = rec.mat->get_diffuse(rec.u, rec.v, rec.p);
  result = result + rec.mat->ka * diffuse_color;

  // Para cada fonte de luz
  for (const auto &light_ptr : lights) {
    vec3 L = light_ptr->get_direction(rec.p);
    double light_dist = light_ptr->get_distance(rec.p);

    // SOMBRAS (Requisito 4)
    ray shadow_ray(rec.p + 0.001 * rec.normal, L);
    hit_record shadow_rec;
    if (world.hit(shadow_ray, 0.001, light_dist - 0.001, shadow_rec)) {
      continue; // Ponto está na sombra desta luz
    }

    color light_intensity = light_ptr->get_intensity(rec.p);

    // Componente difusa
    double diff = std::max(0.0, dot(rec.normal, L));
    result = result + diffuse_color * light_intensity * diff;

    // Componente especular (Blinn-Phong)
    vec3 V = unit_vector(-r.direction());
    vec3 H = unit_vector(L + V);
    double spec =
        std::pow(std::max(0.0, dot(rec.normal, H)), rec.mat->shininess);
    result = result + rec.mat->ks * light_intensity * spec;
  }

  return result.clamp();
}

// ==================== RAY CASTING ====================
color ray_color(const ray &r, const hittable_list &world) {
  hit_record rec;

  if (world.hit(r, 0.001, infinity, rec)) {
    return calculate_lighting(rec, r, world);
  }

  // Cor de fundo (gradiente de céu místico)
  vec3 unit_direction = unit_vector(r.direction());
  double t = 0.5 * (unit_direction.y() + 1.0);
  color sky_top(0.15, 0.2, 0.4);   // Azul escuro
  color sky_bottom(0.5, 0.4, 0.6); // Roxo suave
  return sky_bottom * (1.0 - t) + sky_top * t;
}

// ==================== FUNÇÃO DE PICK (Requisito 5.1) ====================
void perform_pick(int mouse_x, int mouse_y) {
  // Obter tamanho atual da janela
  int window_width = glutGet(GLUT_WINDOW_WIDTH);
  int window_height = glutGet(GLUT_WINDOW_HEIGHT);
  
  // A imagem é desenhada com glRasterPos2i(-1, -1), ou seja, no canto INFERIOR ESQUERDO
  // Coordenadas do mouse GLUT: (0,0) = canto SUPERIOR esquerdo
  // Então a imagem ocupa: x=[0, IMAGE_WIDTH], y=[window_height-IMAGE_HEIGHT, window_height]
  
  // Calcular a posição Y da imagem na janela (começa de baixo)
  int image_y_start = window_height - IMAGE_HEIGHT;  // Onde a imagem começa (de cima para baixo)
  
  // Verificar se o clique está dentro da área da imagem
  if (mouse_x < 0 || mouse_x >= IMAGE_WIDTH ||
      mouse_y < image_y_start || mouse_y >= window_height) {
    // Clique fora da área da imagem (nas bordas pretas)
    std::cout << "\n[Pick] Clique fora da area da imagem\n";
    return;
  }
  
  // Converter coordenadas do mouse para coordenadas relativas à imagem
  // mouse_y relativo à imagem = mouse_y - image_y_start
  int image_mouse_y = mouse_y - image_y_start;
  
  // Converter para coordenadas normalizadas [0,1]
  double u = double(mouse_x) / IMAGE_WIDTH;
  double v = double(IMAGE_HEIGHT - image_mouse_y) / IMAGE_HEIGHT;

  ray r = cam.get_ray(u, v);
  hit_record rec;

  std::cout << "\n========== PICK ==========\n";
  std::cout << "Mouse: (" << mouse_x << ", " << mouse_y << ")\n";
  std::cout << "Image Mouse Y: " << image_mouse_y << "\n";
  std::cout << "UV: (" << u << ", " << v << ")\n";

  if (world.hit(r, 0.001, infinity, rec)) {
    picked_object = rec.object_name;
    std::cout << "OBJETO: " << rec.object_name << "\n";
    std::cout << "Material: " << rec.mat->name << "\n";
    std::cout << "Posicao: (" << rec.p.x() << ", " << rec.p.y() << ", "
              << rec.p.z() << ")\n";
    std::cout << "Normal: (" << rec.normal.x() << ", " << rec.normal.y() << ", "
              << rec.normal.z() << ")\n";
    std::cout << "Distancia (t): " << rec.t << "\n";
    
    // Mostrar GUI com propriedades do objeto
    GUIManager::show(rec.object_name, rec.mat->name,
                     rec.p.x(), rec.p.y(), rec.p.z(),
                     rec.normal.x(), rec.normal.y(), rec.normal.z(),
                     rec.t);
  } else {
    picked_object = "Fundo (Ceu)";
    std::cout << "OBJETO: Fundo (Ceu)\n";
    GUIManager::hide();
  }
  std::cout << "==========================\n";
}

// Forward declaration
void setup_camera();

// ==================== CRIAÇÃO DA CENA ====================
void create_scene() {
  world.clear();
  lights.clear();

  // ===== MATERIAIS =====
  auto mat_metal = materials::sword_metal();
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

  // Centro da cena - AJUSTADO para garantir primeiro octante (x, y, z >= 0)
  // Offset de +650 aplicado para evitar coordenadas negativas nos cliffs
  const double CX = 900.0;
  const double CZ = 900.0;

  // ===== AMBIENTE ÉPICO (GRUTA NATURAL) =====

  // Chão coberto de musgo/vegetação
  world.add(std::make_shared<plane>(point3(0, 0, 0), vec3(0, 1, 0), mat_moss,
                                    "Chao Musgo"));

  // ===== STREAM MOAT (Riacho cercando a ilha central) =====
  // Um único cilindro largo para água suave (sem ondinhas)
  world.add(std::make_shared<cylinder>(point3(CX, 1.0, CZ), vec3(0, 1, 0), 180,
                                       2, mat_water, "Stream Lake"));

  // Rochas ao redor do lago (Textura: rochas_lago.jpg) - CONTORNO COMPLETO
  double current_ang = 0;
  while (current_ang < 2 * pi) {
    double r = random_double(175, 185); // Bem na borda
    double x = CX + r * cos(current_ang);
    double z = CZ + r * sin(current_ang);

    double sz = random_double(12, 22); // Rochas robustas
    world.add(std::make_shared<sphere>(point3(x, 0, z), sz, mat_lake_rock,
                                       "Lake Rock"));

    // Incremento baseado no tamanho para garantir overlap (sem buracos)
    // Angulo = arco / raio
    double ang_step = (sz * 0.7) / r; // 30% de overlap
    current_ang += ang_step;
  }

  // ===== WATERFALL SYSTEM (Cachoeira Grande + Trilha + Lago) =====
  // Posição ajustada: offset +650 aplicado
  double WX = 710.0;  // 60 + 650 = 710 (primeiro octante)
  double WZ = 1090.0; // 440 + 650 = 1090

  // 1. A Queda D'água (COORDENADAS POSITIVAS)
  // Definida com coordenadas positivas na origem e depois transformada
  auto wf_sheet = std::make_shared<box_mesh>(
      point3(0, 0, 0), point3(200, 500, 16), mat_water, "Waterfall Sheet");

  // Transformação: Translação para centralizar + Rotação 45 graus + Translação
  // final
  double angle_deg = -45.0;
  mat4 Tcenter = mat4::translate(-100, 0, -8); // Mover centro para origem
  mat4 Tcenter_inv = mat4::translate_inverse(-100, 0, -8);
  mat4 R = mat4::rotate_y(degrees_to_radians(angle_deg));
  mat4 Rinv = mat4::rotate_y_inverse(degrees_to_radians(angle_deg));
  mat4 T = mat4::translate(WX, 20, WZ);
  mat4 Tinv = mat4::translate_inverse(WX, 20, WZ);
  world.add(std::make_shared<transform>(wf_sheet, T * R * Tcenter,
                                        Tcenter_inv * Rinv * Tinv));

  // 2. Lago da Cachoeira (Maior)
  world.add(std::make_shared<cylinder>(point3(WX, 2, WZ), vec3(0, 1, 0), 120, 5,
                                       mat_water, "Waterfall Pool"));

  // 3. Trilha de Conexão (Rio conectando Cachoeira -> Riacho Central)
  // Um retângulo de água conectando os dois lagos
  // Vetor do centro (CX,CZ) até Cachoeira (WX,WZ)
  vec3 river_dir = unit_vector(point3(WX, 0, WZ) - point3(CX, 0, CZ));
  point3 river_mid =
      point3(CX, 2, CZ) + river_dir * 150.0; // Ponto médio aproximado
  // Simplificação: Vários cilindros ou um box rotacionado. Usando cilindros
  // para curva suave
  for (int k = 0; k < 5; k++) {
    double t = k / 4.0;
    // Interpolacao linear manual para evitar erro de sintaxe
    double start_x = CX + 130 * river_dir.x();
    double start_z = CZ + 130 * river_dir.z();

    point3 p1(start_x, 2, start_z);
    point3 p2(WX, 2, WZ);

    point3 pos = (1.0 - t) * p1 + t * p2;
    world.add(std::make_shared<cylinder>(pos, vec3(0, 1, 0), 40, 4, mat_water,
                                         "River Trail"));
  }

  // Espuma/Nevoa na base e no caminho
  for (int i = 0; i < 30; i++) {
    world.add(std::make_shared<sphere>(
        point3(WX + random_double(-50, 50), 10, WZ + random_double(-50, 50)),
        random_double(8, 20), mat_water, "Splash"));
  }

  // ===== OPEN CLIFFS (Paredes Irregulares, Altas e Imponentes) =====
  for (int i = 0; i < 65; i++) { // Aumentado para 65 (mais denso)
    double angle = -pi / 2 + (pi * i) / 65.0 * 2.6; // Ajustado para 65
    if (angle > pi * 1.3 || angle < -pi * 0.3)
      continue;

    double r = random_double(500, 750); // Mais distante para dar espaço
    double x = CX + r * cos(angle);
    double z = CZ + r * sin(angle);

    // Paredes rochosas (Mistura de pedra, musgo e rocha especifica da parede)
    // 60% Musgo (Cliff vegetation), 20% Rocha Escura, 20% Rocha Parede (Textura
    // Nova)
    std::shared_ptr<material> mat_wall;
    double rnd = random_double(0, 1);
    if (rnd > 0.4)
      mat_wall = mat_moss;
    else if (rnd > 0.2)
      mat_wall = mat_dark;
    else
      mat_wall = mat_wall_stone;

    double h = random_double(300, 750);    // Mais altas (Aura aumentada)
    double size = random_double(150, 300); // Rochas maiores

    point3 rock_pos(x, h * 0.5, z);

    // Check de Proximidade com a Cachoeira
    // As paredes perto da cachoeira devem ter a textura "rocha das paredes"
    // Cachoeira está em (WX, 0, WZ).
    // Cliffs estão raio 500+. WZ=440.
    // Usamos uma distancia de corte para selecionar o setor.
    double dist_to_wf = vec3(x - WX, 0, z - WZ).length();
    if (dist_to_wf <
        500.0) { // Ajuste o raio conforme necessario para pegar a area desejada
      mat_wall = mat_wall_stone;
    }

    world.add(std::make_shared<sphere>(rock_pos, size, mat_wall, "Cliff Rock"));

    // Vegetação nas paredes
    if (random_double(0, 1) > 0.4) {
      world.add(std::make_shared<sphere>(point3(x, h * 0.9, z), size * 0.5,
                                         mat_moss, "Cliff Vegetation"));
    }
  }

  // ===== UPPER CLIFFS (Segundo nivel de altura - "Aura" ainda maior) =====
  for (int i = 0; i < 50; i++) {
    double angle = -pi / 2 + (pi * i) / 50.0 * 2.6;
    if (angle > pi * 1.3 || angle < -pi * 0.3)
      continue;

    double r = random_double(500, 750);
    double x = CX + r * cos(angle);
    double z = CZ + r * sin(angle);

    std::shared_ptr<material> mat_wall;
    double rnd = random_double(0, 1);
    if (rnd > 0.4)
      mat_wall = mat_moss;
    else if (rnd > 0.2)
      mat_wall = mat_dark;
    else
      mat_wall = mat_wall_stone;

    double h = random_double(700, 1100);   // NIVEL SUPERIOR (700 a 1100)
    double size = random_double(180, 350); // Rochas enormes no topo

    point3 rock_pos(x, h * 0.5, z);

    // Mesma logica de textura da cachoeira
    double dist_to_wf = vec3(x - WX, 0, z - WZ).length();
    if (dist_to_wf < 550.0) {
      mat_wall = mat_wall_stone;
    }

    world.add(
        std::make_shared<sphere>(rock_pos, size, mat_wall, "Upper Cliff Rock"));

    // Vegetação (menos densa no topo)
    if (random_double(0, 1) > 0.6) {
      world.add(std::make_shared<sphere>(point3(x, h * 0.9, z), size * 0.5,
                                         mat_moss, "Upper Vegetation"));
    }
  }

  // ===== VEGETATION (Chão e Árvores Detalhadas) =====
  // 1. Plantas Pequenas e Médias no Chão
  for (int i = 0; i < 80; i++) {
    double r = random_double(200, 500);
    double angle = random_double(0, 2 * pi);
    double x = CX + r * cos(angle);
    double z = CZ + r * sin(angle);

    // Evitar area da cachoeira aprox
    if (vec3(x - WX, 0, z - WZ).length() < 100)
      continue;

    double type = random_double(0, 1);
    if (type < 0.3) {
      // Cogumelo Grande Solitario
      world.add(std::make_shared<cylinder>(
          point3(x, 0, z), vec3(0, 1, 0), random_double(1, 3),
          random_double(5, 15), mat_stem, "Mushroom"));
      world.add(std::make_shared<sphere>(point3(x, random_double(5, 15), z),
                                         random_double(4, 8), mat_cap,
                                         "Mushroom"));
    } else if (type < 0.7) {
      // Planta Média (Arbusto)
      world.add(std::make_shared<sphere>(point3(x, 0, z), random_double(10, 25),
                                         mat_moss, "Bush"));
    } else {
      // Planta Pequena (Grama alta/Tufo)
      world.add(std::make_shared<cylinder>(
          point3(x, 0, z),
          vec3(random_double(-0.2, 0.2), 1, random_double(-0.2, 0.2)),
          random_double(2, 5), random_double(10, 20), mat_moss, "Grass Tufo"));
    }
  }

  // 1.5 Colonias de Cogumelos (Novos Grupos Densos)
  for (int i = 0; i < 15; i++) {
    double r_colony = random_double(200, 480);
    double ang_colony = random_double(0, 2 * pi);
    double cx = CX + r_colony * cos(ang_colony);
    double cz = CZ + r_colony * sin(ang_colony);

    // Evitar riacho/cachoeira
    if (vec3(cx - WX, 0, cz - WZ).length() < 80)
      continue;

    // Criar 3-7 cogumelos pequenos no grupo
    int num_mush = (int)random_double(3, 8);
    for (int k = 0; k < num_mush; k++) {
      double mr = random_double(5, 15); // Raio do grupo
      double mang = random_double(0, 2 * pi);
      double mx = cx + mr * cos(mang);
      double mz = cz + mr * sin(mang);

      world.add(std::make_shared<cylinder>(
          point3(mx, 0, mz), vec3(0, 1, 0), random_double(0.5, 1.5),
          random_double(3, 8), mat_stem, "Colony Stem"));
      world.add(std::make_shared<sphere>(point3(mx, random_double(3, 8), mz),
                                         random_double(2, 5), mat_cap,
                                         "Colony Cap"));
    }
  }

  // 2. Arvores Frondosas (Tronco + Galhos + Folhas)
  point3 tree_pos[] = {point3(CX - 280, 0, CZ - 120),
                       point3(CX + 300, 0, CZ + 80)};
  for (auto pos : tree_pos) {
    // Tronco
    world.add(std::make_shared<cylinder>(pos, vec3(0, 1, 0), 30, 400, mat_wood,
                                         "Tree Trunk"));

    // Galhos e Folhas
    for (int k = 0; k < 8; k++) {
      double h = random_double(150, 350);
      double ang = random_double(0, 2 * pi);
      vec3 branch_dir = vec3(cos(ang), 0.5, sin(ang));
      point3 branch_start = pos + vec3(0, h, 0);

      // Galho
      world.add(std::make_shared<cylinder>(branch_start, branch_dir, 8, 100,
                                           mat_wood, "Tree Branch"));

      // Copa de Folhas (Agrupamento de esferas na ponta) (AGORA COM TEX FOLHAS)
      point3 leaf_center = branch_start + branch_dir * 100.0;
      world.add(std::make_shared<sphere>(leaf_center, random_double(40, 70),
                                         mat_leaves, "Tree Leaves"));
    }
  }

  // ===== BASE ROCHOSA / MONTANHOSA
  // (REALISTA) =====
  const double MOUNTAIN_HEIGHT = 45.0; // Reduzido (60 -> 45) para enterrar mais

  // Núcleo da montanha (grande rocha
  // achatada central)
  auto mountain_core = std::make_shared<sphere>(point3(0, 0, 0), 60, mat_stone,
                                                "Nucleo Montanha");
  mat4 core_S = mat4::scale(2.2, 0.7, 2.2); // Um pouco mais alta
                                            // (0.6 -> 0.7)
  mat4 core_Sinv = mat4::scale_inverse(2.2, 0.7, 2.2);
  mat4 core_T = mat4::translate(CX, 30, CZ); // Subiu de 20 para 30
  mat4 core_Tinv = mat4::translate_inverse(CX, 30, CZ);
  world.add(std::make_shared<transform>(mountain_core, core_T * core_S,
                                        core_Sinv * core_Tinv));

  // ===== PEDESTAL ELEVADO =====
  // Pedra Base COM CISALHAMENTO (Requisito 1.4.4 - Bônus +0.5)
  // O cisalhamento faz a pedra parecer naturalmente inclinada/irregular
  auto stone_base_mesh = std::make_shared<box_mesh>(
      point3(-50, 0, -40), point3(50, 60, 40), mat_stone, "Pedra Base");
  // Aplicar CISALHAMENTO: ligeira inclinação da pedra
  mat4 pedra_shear =
      mat4::shear(0.08, 0, 0, 0, 0.05, 0); // Cisalhamento sutil XY e ZY
  mat4 pedra_shear_inv = mat4::shear_inverse(0.08, 0, 0, 0, 0.05, 0);
  mat4 pedra_T = mat4::translate(CX, MOUNTAIN_HEIGHT, CZ);
  mat4 pedra_Tinv = mat4::translate_inverse(CX, MOUNTAIN_HEIGHT, CZ);
  world.add(std::make_shared<transform>(stone_base_mesh, pedra_T * pedra_shear,
                                        pedra_shear_inv * pedra_Tinv));

  // Pedra Topo
  auto stone_top = std::make_shared<box_mesh>(
      point3(CX - 35, MOUNTAIN_HEIGHT + 60, CZ - 25),
      point3(CX + 35, MOUNTAIN_HEIGHT + 85, CZ + 25), mat_stone, "Pedra Topo");
  world.add(stone_top);

  // ===== ESPADA CRAVADA NA PEDRA =====
  // A lâmina aponta para CIMA (Y+) na
  // definição local Depois translada para
  // que a PONTA fique dentro da pedra

  const double BLADE_LENGTH = 130.0;
  // A guarda deve ficar acima do topo da
  // pedra. Topo da pedra = MOUNTAIN_HEIGHT + 85 = 130
  // Nova altura = 195 (65 unidades acima do topo) (Subido MAIS AINDA x2
  // conforme pedido)
  const double GUARD_Y = 195.0;

  // 1. Lâmina (MALHA) - definida apontando
  // para cima
  auto blade = std::make_shared<blade_mesh>(point3(0, 0, 0), // Base
                                            point3(0, BLADE_LENGTH,
                                                   0), // Ponta para CIMA
                                            10, 3,     // Largura MENOR,
                                                       // espessura FINA
                                            mat_metal, "Lamina da Espada",
                                            0.40); // Afina a partir de 40%
                                                   // (bem evidente)

  // Rotacionar: 180 graus em Z (inverte) +
  // 90 graus em Y (gira)
  mat4 blade_Rz = mat4::rotate_z(degrees_to_radians(180));
  mat4 blade_Ry = mat4::rotate_y(degrees_to_radians(90));
  mat4 blade_R = blade_Ry * blade_Rz; // Combina rotações
  mat4 blade_T = mat4::translate(CX, GUARD_Y, CZ);
  mat4 blade_M = blade_T * blade_R;

  mat4 blade_Rzinv = mat4::rotate_z_inverse(degrees_to_radians(180));
  mat4 blade_Ryinv = mat4::rotate_y_inverse(degrees_to_radians(90));
  mat4 blade_Rinv = blade_Rzinv * blade_Ryinv;
  mat4 blade_Tinv = mat4::translate_inverse(CX, GUARD_Y, CZ);
  mat4 blade_Minv = blade_Rinv * blade_Tinv;

  world.add(std::make_shared<transform>(blade, blade_M, blade_Minv));

  // 2. Guarda elaborada (CILINDROS +
  // ESFERAS) Cilindro principal horizontal
  auto guard_main = std::make_shared<cylinder>(point3(0, 0, 0),
                                               vec3(1, 0,
                                                    0), // Eixo horizontal (X)
                                               3.5,
                                               55, // Raio menor, comprimento
                                               mat_gold, "Guarda Principal");
  mat4 guard_T = mat4::translate(CX - 27.5, GUARD_Y, CZ);
  mat4 guard_Tinv = mat4::translate_inverse(CX - 27.5, GUARD_Y, CZ);
  world.add(std::make_shared<transform>(guard_main, guard_T, guard_Tinv));

  // Esferas decorativas nas pontas da
  // guarda
  world.add(std::make_shared<sphere>(point3(CX - 30, GUARD_Y, CZ), 5, mat_gold,
                                     "Guarda Esq"));
  world.add(std::make_shared<sphere>(point3(CX + 30, GUARD_Y, CZ), 5, mat_gold,
                                     "Guarda Dir"));

  // Cilindro central da guarda (mais
  // grosso)
  auto guard_center = std::make_shared<cylinder>(
      point3(0, 0, 0), vec3(0, 1, 0), // Vertical curto
      6, 8, mat_gold, "Guarda Centro");
  mat4 gc_T = mat4::translate(CX, GUARD_Y - 4, CZ);
  mat4 gc_Tinv = mat4::translate_inverse(CX, GUARD_Y - 4, CZ);
  world.add(std::make_shared<transform>(guard_center, gc_T, gc_Tinv));

  // 3. Cabo (CILINDRO) - mais curto
  auto handle = std::make_shared<cylinder>(point3(0, 0, 0),
                                           vec3(0, 1, 0), // Eixo vertical (Y)
                                           3, 25, // Raio 3, altura REDUZIDA
                                           mat_leather, "Cabo da Espada");
  mat4 handle_T = mat4::translate(CX, GUARD_Y + 4, CZ);
  mat4 handle_Tinv = mat4::translate_inverse(CX, GUARD_Y + 4, CZ);
  world.add(std::make_shared<transform>(handle, handle_T, handle_Tinv));

  // 4. Pomo (ESFERA) - conectado ao cabo
  // Cabo termina em GUARD_Y + 4 + 25 =
  // GUARD_Y + 29
  world.add(std::make_shared<sphere>(point3(CX, GUARD_Y + 31, CZ), 4.5,
                                     mat_ruby, "Gema do Pomo"));

  // 5. CONE decorativo no topo do pomo
  auto tip_cone = cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 2.5, 8,
                                  mat_gold, "Ponta Decorativa");
  mat4 tip_T = mat4::translate(CX, GUARD_Y + 35,
                               CZ); // Ajustado para conectar no
                                    // pomo
  mat4 tip_Tinv = mat4::translate_inverse(CX, GUARD_Y + 35, CZ);
  world.add(std::make_shared<transform>(std::make_shared<cone>(tip_cone), tip_T,
                                        tip_Tinv));

  // ===== DEMONSTRAÇÃO DE TRANSFORMAÇÕES (Requisitos 1.4.4 e 1.4.5) =====

  // ===== 1. PILARES DE RUÍNAS ANTIGAS COM CISALHAMENTO (Requisito 1.4.4 -
  // Bônus +0.5) ===== Pilares de pedra antigos ao redor do lago, inclinados
  // devido à erosão/tempo O cisalhamento simula o "assentamento" natural de
  // estruturas antigas

  // Material pedra antiga (mais escuro e desgastado)
  auto mat_ancient_stone = std::make_shared<material>(
      color(0.35, 0.32, 0.28), // Pedra cinza-amarronzada
      0.2,                     // ka
      0.08,                    // ks - pouco brilho (desgaste)
      4.0,                     // shininess baixo
      "Ancient Stone");

  // Posições dos pilares ao redor do lago (formando uma entrada em ruínas)
  // Pilar 1 - Entrada esquerda (inclinado para a direita)
  auto pillar1 =
      std::make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 15, 120,
                                 mat_ancient_stone, "Ruined Pillar 1");
  mat4 pillar1_shear = mat4::shear(0.35, 0, 0, 0, 0,
                                   0); // Cisalhamento XY - MUITO EVIDENTE
  mat4 pillar1_shear_inv = mat4::shear_inverse(0.35, 0, 0, 0, 0, 0);
  mat4 pillar1_T = mat4::translate(CX - 220, 0, CZ + 150);
  mat4 pillar1_Tinv = mat4::translate_inverse(CX - 220, 0, CZ + 150);
  world.add(std::make_shared<transform>(pillar1, pillar1_T * pillar1_shear,
                                        pillar1_shear_inv * pillar1_Tinv));

  // Pilar 2 - Entrada direita (inclinado para a esquerda)
  auto pillar2 =
      std::make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 15, 130,
                                 mat_ancient_stone, "Ruined Pillar 2");
  mat4 pillar2_shear = mat4::shear(-0.30, 0.10, 0, 0, 0,
                                   0); // Cisalhamento oposto - MUITO EVIDENTE
  mat4 pillar2_shear_inv = mat4::shear_inverse(-0.30, 0.10, 0, 0, 0, 0);
  mat4 pillar2_T = mat4::translate(CX + 220, 0, CZ + 150);
  mat4 pillar2_Tinv = mat4::translate_inverse(CX + 220, 0, CZ + 150);
  world.add(std::make_shared<transform>(pillar2, pillar2_T * pillar2_shear,
                                        pillar2_shear_inv * pillar2_Tinv));

  // Pilar 3 - Mais ao fundo, parcialmente destruído (menor e mais inclinado)
  auto pillar3 =
      std::make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 12, 80,
                                 mat_ancient_stone, "Ruined Pillar 3");
  mat4 pillar3_shear = mat4::shear(0.25, 0.30, 0, 0, 0,
                                   0); // Cisalhamento diagonal - MUITO EVIDENTE
  mat4 pillar3_shear_inv = mat4::shear_inverse(0.25, 0.30, 0, 0, 0, 0);
  mat4 pillar3_T = mat4::translate(CX - 180, 0, CZ + 280);
  mat4 pillar3_Tinv = mat4::translate_inverse(CX - 180, 0, CZ + 280);
  world.add(std::make_shared<transform>(pillar3, pillar3_T * pillar3_shear,
                                        pillar3_shear_inv * pillar3_Tinv));

  // Pilar 4 - Quase caído (muito inclinado)
  auto pillar4 =
      std::make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 14, 100,
                                 mat_ancient_stone, "Ruined Pillar 4");
  mat4 pillar4_shear =
      mat4::shear(-0.45, 0.15, 0, 0, 0, 0); // QUASE CAINDO - muito evidente
  mat4 pillar4_shear_inv = mat4::shear_inverse(-0.45, 0.15, 0, 0, 0, 0);
  mat4 pillar4_T = mat4::translate(CX + 200, 0, CZ + 300);
  mat4 pillar4_Tinv = mat4::translate_inverse(CX + 200, 0, CZ + 300);
  world.add(std::make_shared<transform>(pillar4, pillar4_T * pillar4_shear,
                                        pillar4_shear_inv * pillar4_Tinv));

  // Capitéis (topos decorativos dos pilares - esferas achatadas)
  // Capitel do Pilar 1
  auto cap1 = std::make_shared<sphere>(point3(0, 0, 0), 20, mat_ancient_stone,
                                       "Pillar 1 Capital");
  mat4 cap1_S = mat4::scale(1.3, 0.4, 1.3);
  mat4 cap1_Sinv = mat4::scale_inverse(1.3, 0.4, 1.3);
  mat4 cap1_T =
      mat4::translate(CX - 220 + 14.4, 120, CZ + 150); // Offset pelo shear
  mat4 cap1_Tinv = mat4::translate_inverse(CX - 220 + 14.4, 120, CZ + 150);
  world.add(std::make_shared<transform>(cap1, cap1_T * cap1_S,
                                        cap1_Sinv * cap1_Tinv));

  // Capitel do Pilar 2
  auto cap2 = std::make_shared<sphere>(point3(0, 0, 0), 20, mat_ancient_stone,
                                       "Pillar 2 Capital");
  mat4 cap2_T = mat4::translate(CX + 220 - 13, 130, CZ + 150 + 6.5);
  mat4 cap2_Tinv = mat4::translate_inverse(CX + 220 - 13, 130, CZ + 150 + 6.5);
  world.add(std::make_shared<transform>(cap2, cap2_T * cap1_S,
                                        cap1_Sinv * cap2_Tinv));

  // ===== TOCHA PRÓXIMA À CLIFF ROCK (Uso criativo de CONE + vec4) =====
  // Tocha estilo medieval: poste vertical fincado no chão com chama no topo
  // Posicionada próxima às rochas, mas NO CHÃO (como na imagem de referência)
  
  // Material para a chama externa (laranja brilhante - emissivo)
  auto mat_torch_flame = std::make_shared<material>(
      color(1.0, 0.55, 0.1),  // Laranja quente
      0.95,                   // ka muito alto - simula emissão
      0.25,                   // ks
      8.0,                    // shininess
      "Torch Flame");

  // Material para o núcleo da chama (amarelo intenso)
  auto mat_torch_core = std::make_shared<material>(
      color(1.0, 0.85, 0.2),  // Amarelo dourado
      0.98,                   // ka máximo
      0.15, 4.0,
      "Torch Core");

  // Material para o poste de madeira (marrom escuro)
  auto mat_torch_pole = std::make_shared<material>(
      color(0.25, 0.15, 0.08), // Marrom escuro
      0.12,                    // ka baixo
      0.04,                    // ks - fosco
      4.0,
      "Torch Pole");

  // POSIÇÃO DA TOCHA usando vec4
  // Câmera em (1050, 200, 750), olhando para (900, 100, 900)
  // Tocha posicionada ENTRE a câmera e o centro, à esquerda da pedra
  // Esta posição está no campo de visão da câmera
  vec4 torch_base_pos = vec4(CX - 100, 0, CZ - 80, 1.0);  // (800, 0, 820)
  point3 base_pos = torch_base_pos.to_point3();

  // Altura do poste
  const double POLE_HEIGHT = 120.0;

  // 1. POSTE DE MADEIRA (cilindro vertical fincado no chão)
  world.add(std::make_shared<cylinder>(
      base_pos,
      vec3(0, 1, 0),        // Vertical
      4,                    // Raio do poste
      POLE_HEIGHT,          // Altura do poste
      mat_torch_pole,
      "Torch Pole"));

  // Posição do topo do poste usando vec4
  vec4 pole_top_vec4 = torch_base_pos + vec4(0, POLE_HEIGHT, 0, 0);
  point3 pole_top = pole_top_vec4.to_point3();

  // 2. CHAMA EXTERNA (CONE laranja) - no topo do poste
  auto flame_outer = cone::from_base(
      point3(0, 0, 0),
      vec3(0, 1, 0),        // Aponta para cima
      12,                   // Raio base
      35,                   // Altura
      mat_torch_flame,
      "Torch Flame Outer");

  mat4 fo_T = mat4::translate(pole_top.x(), pole_top.y(), pole_top.z());
  mat4 fo_Tinv = mat4::translate_inverse(pole_top.x(), pole_top.y(), pole_top.z());
  world.add(std::make_shared<transform>(
      std::make_shared<cone>(flame_outer), fo_T, fo_Tinv));

  // 3. NÚCLEO DA CHAMA (CONE amarelo menor) - ligeiramente acima
  auto flame_inner = cone::from_base(
      point3(0, 0, 0),
      vec3(0, 1, 0),
      6,                    // Raio menor
      25,                   // Altura menor
      mat_torch_core,
      "Torch Flame Core");

  mat4 fi_T = mat4::translate(pole_top.x(), pole_top.y() + 5, pole_top.z());
  mat4 fi_Tinv = mat4::translate_inverse(pole_top.x(), pole_top.y() + 5, pole_top.z());
  world.add(std::make_shared<transform>(
      std::make_shared<cone>(flame_inner), fi_T, fi_Tinv));

  // 4. FONTE DE LUZ PONTUAL (ilumina a área)
  vec4 light_pos_vec4 = pole_top_vec4 + vec4(0, 25, 0, 0);
  point3 torch_light_pos = light_pos_vec4.to_point3();
  lights.push_back(std::make_shared<point_light>(
      torch_light_pos,
      color(1.0, 0.6, 0.2),  // Luz laranja quente
      0.8,                   // Intensidade
      0.001,                 // Atenuação linear
      0.00004));             // Atenuação quadrática

  // ===== 2. REFLEXOS NA ÁGUA COM ESPELHO (Requisito 1.4.5 - Bônus +0.5) =====
  // Reflexos no Stream Lake (lago ao redor da pedra central)
  // Altura da superfície da água: y = 2.0 (topo do cilindro de água)

  // Reflexo da gema do pomo da espada no lago
  auto reflected_gem = std::make_shared<sphere>(
      point3(CX, GUARD_Y + 31, CZ), 4.5, mat_ruby, "Pomo Gem Original");
  auto gem_mirrored =
      reflect_object(reflected_gem, point3(CX, 2.0, CZ), vec3(0, 1, 0));
  world.add(gem_mirrored);

  // Reflexo do cone decorativo da espada
  auto reflected_tip = std::make_shared<cone>(
      cone::from_base(point3(CX, GUARD_Y + 35, CZ), vec3(0, 1, 0), 2.5, 8,
                      mat_gold, "Tip Cone Original"));
  auto tip_mirrored =
      reflect_object(reflected_tip, point3(CX, 2.0, CZ), vec3(0, 1, 0));
  world.add(tip_mirrored);

  // Reflexo de uma esfera da guarda no lago central
  auto reflected_guard = std::make_shared<sphere>(
      point3(CX + 30, GUARD_Y, CZ), 5, mat_gold, "Guard Sphere Original");
  auto guard_mirrored =
      reflect_object(reflected_guard, point3(CX, 2.0, CZ), vec3(0, 1, 0));
  world.add(guard_mirrored);

  // Reflexo na cachoeira (Waterfall Pool) - reflexo de splash spheres
  // Usando uma esfera de névoa/splash e seu reflexo
  auto waterfall_splash = std::make_shared<sphere>(
      point3(WX - 30, 25, WZ + 20), 12, mat_water, "Waterfall Splash");
  world.add(waterfall_splash);
  auto splash_mirrored =
      reflect_object(waterfall_splash, point3(WX, 2.0, WZ), vec3(0, 1, 0));
  world.add(splash_mirrored);

  // ===== 3. GEMA COM ROTAÇÃO QUATERNION NA GUARDA DA ESPADA (Requisito 1.4.2)
  // ===== Material safira azul para a gema
  auto mat_sapphire =
      std::make_shared<material>(color(0.1, 0.2, 0.8), // Azul safira
                                 0.2,                  // ka
                                 0.95,                 // ks - muito brilhante
                                 256.0,                // shininess muito alto
                                 "Sapphire Gem");

  // Gema azul com rotação em eixo arbitrário usando quatérnios
  // Posicionada na guarda da espada principal (lado oposto à esfera dourada)
  auto quaternion_sapphire = std::make_shared<sphere>(
      point3(0, 0, 0), 6, mat_sapphire, "Quaternion Sapphire Gem");
  // Rotação de 30 graus em torno do eixo diagonal (1, 1, 1) - eixo arbitrário
  auto rotated_sapphire = rotate_axis_object(quaternion_sapphire, vec3(1, 1, 1),
                                             degrees_to_radians(30));
  // Posicionar na guarda da espada principal (lado esquerdo)
  auto positioned_sapphire =
      translate_object(rotated_sapphire, CX - 30, GUARD_Y, CZ);
  world.add(positioned_sapphire);

  // Segunda gema com quaternion no centro da guarda (mais visível)
  auto quaternion_emerald = std::make_shared<sphere>(
      point3(0, 0, 0), 4,
      std::make_shared<material>(color(0.1, 0.7, 0.2), 0.2, 0.9, 200.0,
                                 "Emerald Gem"),
      "Quaternion Emerald");
  // Rotação de 60 graus em torno do eixo (0, 1, 1) - outro eixo arbitrário
  auto rotated_emerald = rotate_axis_object(quaternion_emerald, vec3(0, 1, 1),
                                            degrees_to_radians(60));
  auto positioned_emerald =
      translate_object(rotated_emerald, CX, GUARD_Y + 5, CZ);
  world.add(positioned_emerald);

  // ===== ILUMINAÇÃO DRAMÁTICA (NOITE/PALCO) =====

  // Luz ambiente muito escura (noite) - destaca a luz direcional na espada
  ambient = ambient_light(0.03, 0.035, 0.06); // Muito escura, levemente azulada

  // LUZ DIRECIONAL (Requisito 1.5.3 - Bônus +0.5)
  // Vem de DIRETAMENTE ACIMA como luz de palco/hotspot na espada
  lights.push_back(std::make_shared<directional_light>(
      vec3(0, -1, 0),            // Direção: DIRETAMENTE de cima (vertical)
      color(0.85, 0.80, 0.70))); // Luz suave e quente (não estoura)

  // Spot light secundário para efeito místico suave
  auto spot_dir = unit_vector(vec3(0.05, -1, 0));
  lights.push_back(std::make_shared<spot_light>(
      point3(CX, 400, CZ), spot_dir,
      color(0.3, 0.25, 0.4),  // Luz roxa mística bem suave
      degrees_to_radians(20), // Cone interno
      degrees_to_radians(50), // Cone externo
      1.0, 0.000008, 0.0000005));

  // Luz 4 (Point Light - Cachoeira Glow)
  lights.push_back(
      std::make_shared<point_light>(point3(WX, 50,
                                           WZ), // Base da cachoeira
                                    color(0.4, 0.7,
                                          1.0), // Azul claro
                                    0.6, 0.003, 0.00005));

  // Luz de preenchimento
  // (Tocha/Vagalumes?) Luz pontual laranja
  // fraca na base para iluminar as pedras
  lights.push_back(std::make_shared<point_light>(
      point3(CX - 80, 100, CZ + 80), color(0.8, 0.5, 0.2), 0.5, 0.001, 0.0001));

  // Luz de preenchimento azulada (mágica)
  // do outro lado
  lights.push_back(std::make_shared<point_light>(
      point3(CX + 80, 120, CZ - 80), color(0.2, 0.4, 0.8), 0.4, 0.001, 0.0001));

  // ===== CÂMERA =====
  setup_camera();
}

void setup_camera() {
  double window_size = 200.0;

  switch (current_projection) {
  case 0: // Perspectiva
    cam.setup(cam_eye, cam_at, vec3(0, 1, 0), 100.0, -window_size, window_size,
              -window_size, window_size, ProjectionType::PERSPECTIVE);
    break;

  case 1: // Ortográfica
    cam.setup(cam_eye, cam_at, vec3(0, 1, 0), 100.0, -window_size, window_size,
              -window_size, window_size, ProjectionType::ORTHOGRAPHIC);
    break;

  case 2: // Oblíqua
    cam.setup(cam_eye, cam_at, vec3(0, 1, 0), 100.0, -window_size, window_size,
              -window_size, window_size, ProjectionType::OBLIQUE);
    cam.oblique_angle = 0.5;
    cam.oblique_strength = 0.35;
    break;
  }

  std::cout << "Camera: Eye(" << cam_eye.x() << ", " << cam_eye.y() << ", "
            << cam_eye.z() << ")\n";
}

// ==================== RENDERIZAÇÃO ====================
void render() {
  std::cout << "Renderizando " << IMAGE_WIDTH << "x" << IMAGE_HEIGHT
            << " pixels...\n";

  for (int j = 0; j < IMAGE_HEIGHT; j++) {
    if (j % 100 == 0) {
      std::cout << "Linha " << j << "/" << IMAGE_HEIGHT << "\r" << std::flush;
    }

    for (int i = 0; i < IMAGE_WIDTH; i++) {
      double u = double(i) / (IMAGE_WIDTH - 1);
      double v = double(j) / (IMAGE_HEIGHT - 1);

      ray r = cam.get_ray(u, v);
      color pixel_color = ray_color(r, world);

      // Armazenar no buffer (sem inversão - OpenGL faz isso)
      int idx = (j * IMAGE_WIDTH + i) * 3;

      PixelBuffer[idx] = pixel_color.r_byte();
      PixelBuffer[idx + 1] = pixel_color.g_byte();
      PixelBuffer[idx + 2] = pixel_color.b_byte();
    }
  }

  std::cout << "Renderizacao concluida!                    \n";
  need_redraw = false;
}

// ==================== CALLBACKS GLUT ====================
void display() {
  if (need_redraw) {
    render();
  }

  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(-1, -1);
  glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
               PixelBuffer);

  // Mostrar informações na tela
  glColor3f(1.0f, 1.0f, 0.0f);
  glRasterPos2f(-0.98f, 0.92f);

  std::string info = "Projecao: ";
  switch (current_projection) {
  case 0:
    info += "Perspectiva (" + std::to_string(vanishing_points) + " PF)";
    break;
  case 1:
    info += "Ortografica";
    break;
  case 2:
    info += "Obliqua";
    break;
  }
  for (char c : info) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }

  if (!picked_object.empty()) {
    glRasterPos2f(-0.98f, 0.85f);
    std::string pick_info = "Pick: " + picked_object;
    for (char c : pick_info) {
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
  }

  // Desenhar GUI de propriedades (se visível)
  GUIManager::draw();

  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
  bool changed = false;
  vec3 forward = unit_vector(cam_at - cam_eye);
  vec3 right = unit_vector(cross(forward, vec3(0, 1, 0)));

  switch (key) {
  case 27: // ESC
  case 'q':
  case 'Q':
    std::cout << "Encerrando...\n";
    if (PixelBuffer)
      delete[] PixelBuffer;
    exit(0);
    break;

  // === MOVIMENTO DA CÂMERA ===
  case 'w':
  case 'W':
    cam_eye = cam_eye + forward * cam_speed;
    cam_at = cam_at + forward * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 's':
  case 'S':
    cam_eye = cam_eye - forward * cam_speed;
    cam_at = cam_at - forward * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 'a':
  case 'A':
    cam_eye = cam_eye - right * cam_speed;
    cam_at = cam_at - right * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 'd':
  case 'D':
    cam_eye = cam_eye + right * cam_speed;
    cam_at = cam_at + right * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 'r':
  case 'R':
    cam_eye[1] += cam_speed;
    cam_at[1] += cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 'f':
  case 'F':
    cam_eye[1] -= cam_speed;
    cam_at[1] -= cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  // === PROJEÇÕES ===
  case '1':
    current_projection = 0;
    vanishing_points = 1;
    setup_camera();
    need_redraw = true;
    changed = true;
    std::cout << "Perspectiva\n";
    break;

  case 'o':
  case 'O':
    current_projection = 1;
    setup_camera();
    need_redraw = true;
    changed = true;
    std::cout << "Projecao Ortografica\n";
    break;

  case 'p':
  case 'P':
    current_projection = 2;
    setup_camera();
    need_redraw = true;
    changed = true;
    std::cout << "Projecao Obliqua\n";
    break;

  case '+':
  case '=':
    cam.zoom_in(0.8);
    need_redraw = true;
    changed = true;
    std::cout << "Zoom In\n";
    break;

  case '-':
  case '_':
    cam.zoom_out(1.25);
    need_redraw = true;
    changed = true;
    std::cout << "Zoom Out\n";
    break;

  case 'h':
  case 'H':
    std::cout << "\n=== CONTROLES ===\n";
    std::cout << "WASD - Mover camera (frente/tras/esq/dir)\n";
    std::cout << "R/F - Subir/Descer camera\n";
    std::cout << "Setas - Rotacionar camera\n";
    std::cout << "1 - Perspectiva | O - Ortografica | P - Obliqua\n";
    std::cout << "+/- - Zoom In/Out\n";
    std::cout << "Click - Pick de objeto\n";
    std::cout << "Q/ESC - Sair\n";
    std::cout << "=================\n\n";
    break;

  case 'c':
  case 'C':
    // Reset câmera para posição padrão
    cam_eye = point3(400, 200, 100);
    cam_at = point3(250, 100, 250);
    setup_camera();
    need_redraw = true;
    changed = true;
    std::cout << "Camera resetada\n";
    break;
  }

  if (changed) {
    glutPostRedisplay();
  }
}

// Função para teclas especiais (setas)
void special_keys(int key, int x, int y) {
  bool changed = false;

  vec3 forward = unit_vector(cam_at - cam_eye);
  vec3 right = unit_vector(cross(forward, vec3(0, 1, 0)));
  vec3 up(0, 1, 0);

  switch (key) {
  case GLUT_KEY_UP:
    cam_at = cam_at + up * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case GLUT_KEY_DOWN:
    cam_at = cam_at - up * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case GLUT_KEY_LEFT:
    cam_at = cam_at - right * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case GLUT_KEY_RIGHT:
    cam_at = cam_at + right * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;
  }

  if (changed) {
    glutPostRedisplay();
  }
}

void mouse(int button, int state, int x, int y) {
  // Primeiro verificar se o clique foi na GUI
  if (GUIManager::handleMouseClick(x, y, button, state)) {
    glutPostRedisplay();
    return; // Clique consumido pela GUI
  }
  
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    perform_pick(x, y);
    glutPostRedisplay();
  }
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

// ==================== MAIN ====================
int main(int argc, char **argv) {
  std::cout << "============================================\n";
  std::cout << "  COMPUTACAO GRAFICA - ESPADA NA PEDRA\n";
  std::cout << "  Resolucao: " << IMAGE_WIDTH << "x" << IMAGE_HEIGHT << "\n";
  std::cout << "============================================\n\n";

  // Inicializar buffer
  PixelBuffer = new unsigned char[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
  memset(PixelBuffer, 0, IMAGE_WIDTH * IMAGE_HEIGHT * 3);

  // Criar cena
  std::cout << "Criando cena...\n";
  create_scene();

  // Inicializar GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(IMAGE_WIDTH, IMAGE_HEIGHT);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("CG - Espada na Pedra (Ray Caster)");

  // Configurar OpenGL
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // Registrar callbacks
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special_keys);
  glutMouseFunc(mouse);
  glutReshapeFunc(reshape);

  std::cout << "\n=== CONTROLES ===\n";
  std::cout << "WASD - Mover camera\n";
  std::cout << "R/F - Subir/Descer\n";
  std::cout << "Setas - Rotacionar visao\n";
  std::cout << "C - Reset camera\n";
  std::cout << "1/O/P - Perspectiva/Ortografica/Obliqua\n";
  std::cout << "+/- - Zoom\n";
  std::cout << "Click - Pick | H - Help\n";
  std::cout << "=================\n\n";

  // Loop principal
  glutMainLoop();

  return 0;
}
