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
#include <map>

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
const int IMAGE_WIDTH = 800;
const int IMAGE_HEIGHT = 800;
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

// (Variáveis de brilho movidas para junto de toggle_blade_shine)

// Estado Dia/Noite
bool is_night_mode = false;
color sky_color_top(0.15, 0.2, 0.4);      // Azul escuro (Dia)
color sky_color_bottom(0.5, 0.4, 0.6);    // Roxo suave (Dia)

// ==================== ESTADO DE TRANSFORMAÇÃO ====================
struct TransformState {
    vec3 scale;
    vec3 rotation;    // Euler angles em graus (X, Y, Z)
    vec3 translation;
    
    TransformState() : scale(1,1,1), rotation(0,0,0), translation(0,0,0) {}
};

// Mapas globais para controle de objetos transformáveis
std::map<std::string, TransformState> object_states;
std::map<std::string, std::shared_ptr<transform>> object_transforms;
std::string selected_transform_name = ""; // Nome do objeto transformável selecionado atualmente

// Função para atualizar a matriz de um objeto baseado no seu estado
void update_object_transform(const std::string& name) {
    if (object_states.find(name) == object_states.end() || 
        object_transforms.find(name) == object_transforms.end()) {
        return;
    }
    
    TransformState& state = object_states[name];
    auto trans_ptr = object_transforms[name];
    
    // Recriar a transformação composta: Scale -> Rotate -> Translate
    // Ordem inversa: Translate * Rotate * Scale * Ponto
    
    // Rotação (Euler XYZ)
    mat4 Rx = mat4::rotate_x(degrees_to_radians(state.rotation.x()));
    mat4 Ry = mat4::rotate_y(degrees_to_radians(state.rotation.y()));
    mat4 Rz = mat4::rotate_z(degrees_to_radians(state.rotation.z()));
    mat4 R = Rz * Ry * Rx; // Z * Y * X
    
    // Translação e Escala
    mat4 T = mat4::translate(state.translation.x(), state.translation.y(), state.translation.z());
    mat4 S = mat4::scale(state.scale.x(), state.scale.y(), state.scale.z());
    
    // Matriz Forward (Mundo <- Objeto)
    trans_ptr->forward = T * R * S;
    
    // Inversas
    mat4 Rinv = mat4::rotate_x_inverse(degrees_to_radians(state.rotation.x())) * 
                mat4::rotate_y_inverse(degrees_to_radians(state.rotation.y())) * 
                mat4::rotate_z_inverse(degrees_to_radians(state.rotation.z()));
                
    mat4 Tinv = mat4::translate_inverse(state.translation.x(), state.translation.y(), state.translation.z());
    mat4 Sinv = mat4::scale_inverse(state.scale.x(), state.scale.y(), state.scale.z());
    
    // Matriz Inverse (Objeto <- Mundo) = Sinv * Rinv * Tinv
    trans_ptr->inverse = Sinv * Rinv * Tinv;
    
    // Normal Mat = Transpose(Inverse)
    trans_ptr->normal_mat = trans_ptr->inverse.transpose();
    
    need_redraw = true;
}

// Câmera interativa
// NOTA: Coordenadas ajustadas para primeiro octante (x, y, z >= 0)
point3 cam_eye(1050, 200, 750); // Posição inicial (400+650, 200, 100+650)
point3 cam_at(900, 100,
              900); // Olhando para o centro da cena (250+650, 100, 250+650)
vec3 cam_up(0, 1, 0); // Vetor Up (padrão: Y+)
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

  // Cor de fundo (gradiente dinâmico dependendo do horário)
  vec3 unit_direction = unit_vector(r.direction());
  double t = 0.5 * (unit_direction.y() + 1.0);
  return sky_color_bottom * (1.0 - t) + sky_color_top * t;
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
                     
    // Verificar se pertence a um grupo transformável
    selected_transform_name = "";
    
    // Regra para ESPADA
    if (rec.object_name == "Lamina" || 
        rec.object_name == "Guarda Principal" || 
        rec.object_name == "Guarda Esq" || 
        rec.object_name == "Guarda Dir" || 
        rec.object_name == "Guarda Centro" || 
        rec.object_name == "Cabo" || 
        rec.object_name == "Pomo" || 
        rec.object_name == "Ponta" ||
        rec.object_name == "Quaternion Sapphire Gem" ||
        rec.object_name == "Quaternion Emerald") {
        selected_transform_name = "Espada Completa";
    }
    // Regra para TOCHA
    else if (rec.object_name == "Torch Pole" ||
             rec.object_name == "Torch Flame Outer" ||
             rec.object_name == "Torch Flame Core") {
        selected_transform_name = "Tocha Medieval";
    }
    // Regra para PILAR 1
    else if (rec.object_name == "Ruined Pillar 1" ||
             rec.object_name == "Pillar 1 Capital") {
        selected_transform_name = "Pilar Ruina 1";
    }
    // Regra para PILAR 2
    else if (rec.object_name == "Ruined Pillar 2" ||
             rec.object_name == "Pillar 2 Capital") {
        selected_transform_name = "Pilar Ruina 2";
    }
    // Regra genérica: se o objeto clicado tem um transform registrado com seu nome
    else if (object_transforms.find(rec.object_name) != object_transforms.end()) {
        selected_transform_name = rec.object_name;
    }
    
    if (!selected_transform_name.empty()) {
        std::cout << "Selecionado Transformavel: " << selected_transform_name << "\n";
    }
    
  } else {
    picked_object = "Fundo (Ceu)";
    selected_transform_name = ""; // Limpar seleção
    std::cout << "OBJETO: Fundo (Ceu)\n";
    GUIManager::hide();
  }
  std::cout << "==========================\n";
}

// Forward declaration
void setup_camera();

// Estado do brilho da lâmina (toggle)
bool blade_shine_enabled = false;  // Começa SEM brilho
std::shared_ptr<material> mat_metal_ptr = nullptr;  // Ponteiro para o material da lâmina
std::shared_ptr<light> sword_light_ptr = nullptr;   // Luz dinâmica que acompanha a espada

// ==================== ATUALIZAR LUZ DA ESPADA ====================
void update_sword_light() {
    // 1. Remover luz antiga se existir
    if (sword_light_ptr) {
        auto it = std::find(lights.begin(), lights.end(), sword_light_ptr);
        if (it != lights.end()) {
            lights.erase(it);
        }
        sword_light_ptr = nullptr;
    }

    // 2. Se brilho desligado, sai
    if (!blade_shine_enabled) return;

    // 3. Obter estado atual de transformação
    vec3 t_vec(900, 195, 900); // Default
    vec3 r_vec(0,0,0);
    vec3 s_vec(1,1,1);
    
    if (object_states.find("Espada Completa") != object_states.end()) {
        TransformState& state = object_states["Espada Completa"];
        t_vec = state.translation;
        r_vec = state.rotation;
        s_vec = state.scale;
    }

    // 4. Reconstruir Matriz Modelo (T * R * S)
    // Rotação (Euler XYZ)
    mat4 Rx = mat4::rotate_x(degrees_to_radians(r_vec.x()));
    mat4 Ry = mat4::rotate_y(degrees_to_radians(r_vec.y()));
    mat4 Rz = mat4::rotate_z(degrees_to_radians(r_vec.z()));
    mat4 R = Rz * Ry * Rx; 
    
    mat4 T = mat4::translate(t_vec.x(), t_vec.y(), t_vec.z());
    mat4 S = mat4::scale(s_vec.x(), s_vec.y(), s_vec.z());
    
    mat4 M = T * R * S;

    // 5. Ponto local na lâmina
    // Lâmina vai de 0 a -130 em Y (por causa da rotação interna do grupo).
    // Posicionar luz no meio da lâmina (-50) e levemente afastada em Z (+15) para iluminar a face
    vec4 local_pos(0, -50, 15, 1.0);
    
    // Transformar para Mundo
    vec4 world_pos = M * local_pos;
    
    // 6. Criar nova luz
    sword_light_ptr = std::make_shared<point_light>(
        point3(world_pos[0], world_pos[1], world_pos[2]),
        color(0.5, 0.7, 1.0), // Azulado intenso
        2.5,                  // Intensidade alta
        0.001, 0.0001
    );
    
    lights.push_back(sword_light_ptr);
}

// ==================== FUNÇÃO TOGGLE BRILHO DA LÂMINA ====================
void toggle_blade_shine(bool increase) {
  if (increase && !blade_shine_enabled) {
    // Ligar brilho
    blade_shine_enabled = true;
    if (mat_metal_ptr) {
      // Brilho MUITO alto e saturado para ser visível
      mat_metal_ptr->ks = color(4.0, 4.0, 5.0);  // Valores > 1 para saturar
      mat_metal_ptr->shininess = 64.0;           // Menor shininess = destaque maior/mais espalhado
    }
    std::cout << "[Brilho] Lamina: LIGADO\n";
  } else if (!increase && blade_shine_enabled) {
    // Desligar brilho
    blade_shine_enabled = false;
    if (mat_metal_ptr) {
      mat_metal_ptr->ks = color(0.1, 0.1, 0.1);  // Brilho baixo
      mat_metal_ptr->shininess = 8.0;
    }
    std::cout << "[Brilho] Lamina: DESLIGADO\n";
  }
  
  // Atualizar a luz dinâmica
  update_sword_light();
  need_redraw = true;
}

// ==================== CONFIGURAÇÃO DE ILUMINAÇÃO ====================
void setup_lighting() {
  lights.clear();
  const double CX = 900.0;
  const double CZ = 900.0;
  
  const double WX = 710.0;
  const double WZ = 1090.0;
  
  // Posição da luz da tocha (recalculada para garantir consistência)
  vec4 torch_base_pos = vec4(CX - 100, 0, CZ - 80, 1.0);
  double POLE_HEIGHT = 120.0;
  vec4 pole_top_vec4 = torch_base_pos + vec4(0, POLE_HEIGHT, 0, 0);
  point3 torch_light_pos = (pole_top_vec4 + vec4(0, 25, 0, 0)).to_point3();

  if (is_night_mode) {
    // --- NOITE (Dramatica/Palco) ---
    // Luz ambiente muito escura (noite)
    ambient.intensity = color(0.03, 0.035, 0.06); // Muito escura, levemente azulada
    
    // LUZ DIRECIONAL (Palco/Hotspot - Requisito 1.5.3)
    // Vem de DIRETAMENTE ACIMA como luz de palco/hotspot na espada
    lights.push_back(std::make_shared<directional_light>(
        vec3(0, -1, 0),            // Direção: DIRETAMENTE de cima (vertical)
        color(0.85, 0.80, 0.70))); // Luz suave e quente
    
    // Spot light secundário para efeito místico suave
    auto spot_dir = unit_vector(vec3(0.05, -1, 0));
    lights.push_back(std::make_shared<spot_light>(
        point3(CX, 400, CZ), spot_dir,
        color(0.3, 0.25, 0.4),  // Luz roxa mística bem suave
        degrees_to_radians(20), // Cone interno
        degrees_to_radians(50), // Cone externo
        1.0, 0.000008, 0.0000005));

    // Luz da Tocha: Muito mais visível e importante à noite
    lights.push_back(std::make_shared<point_light>(
        torch_light_pos,
        color(1.0, 0.6, 0.2),  // Laranja
        2.5,                   // Intensidade ALTA na noite
        0.001, 0.00004
    ));

    // Luz (Point Light - Cachoeira Glow)
    lights.push_back(std::make_shared<point_light>(
        point3(WX, 50, WZ),   // Base da cachoeira
        color(0.4, 0.7, 1.0), // Azul claro
        0.6, 0.003, 0.00005
    ));

    // Luz de preenchimento (Laranja) - Iluminar rochas
    lights.push_back(std::make_shared<point_light>(
        point3(CX - 80, 100, CZ + 80), 
        color(0.8, 0.5, 0.2), 
        0.5, 0.001, 0.0001
    ));

    // Luz de preenchimento azulada (mágica) do outro lado
    lights.push_back(std::make_shared<point_light>(
        point3(CX + 80, 120, CZ - 80), 
        color(0.2, 0.4, 0.8), 
        0.4, 0.001, 0.0001
    ));
    
    // Céu Noturno
    sky_color_top = color(0.02, 0.02, 0.05);      // Quase preto
    sky_color_bottom = color(0.05, 0.05, 0.1);    // Azul muito escuro
    
  } else {
    // --- DIA (Original) ---
    // Luz Ambiente: Clara e quente
    ambient.intensity = color(0.3, 0.3, 0.3);
    
    // Sol Principal (Direcional, Amarelo Claro)
    vec3 sun_dir = unit_vector(vec3(1.0, -1.0, -0.5)); // Vindo da esquerda/cima
    lights.push_back(std::make_shared<directional_light>(
        sun_dir,
        color(1.0, 0.95, 0.9) * 0.8 // Amarelo/Branco sol * Forte
    ));
    
    // Luz da Tocha: Menos impactante de dia, mas ainda existe
    lights.push_back(std::make_shared<point_light>(
        torch_light_pos,
        color(1.0, 0.6, 0.2),
        0.8,                   // Normal
        0.001, 0.00004
    ));
    
    // Céu Diurno Místico (Original)
    sky_color_top = color(0.15, 0.2, 0.4);
    sky_color_bottom = color(0.5, 0.4, 0.6);
  }
}

// ==================== TOGGLE DAY/NIGHT ====================
void toggle_day_night(bool set_to_night) {
  is_night_mode = set_to_night;
  setup_lighting();
  std::cout << "[Ambiente] Modo alterado para: " << (is_night_mode ? "NOITE" : "DIA") << "\n";
  need_redraw = true;
}

// ==================== CRIAÇÃO DA CENA ====================
void create_scene() {
  world.clear();
  
  // Limpar mapas de transformação
  object_states.clear();
  object_transforms.clear();
  
  // Configurar Luzes
  setup_lighting();

  // ===== MATERIAIS =====
  auto mat_metal = materials::sword_metal();
  mat_metal_ptr = mat_metal;  // Guardar referência para modificar brilho depois
  
  // Aplicar estado inicial do brilho (sem brilho)
  if (!blade_shine_enabled) {
    mat_metal->ks = color(0.1, 0.1, 0.1);  // Brilho desativado
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

  // ===== ESPADA CRAVADA NA PEDRA (AGRUPADA E TRANSFORMÁVEL) =====
  
  // 1. Criar grupo para os componentes da espada
  // As coordenadas agora são RELATIVAS ao centro da espada (Pivô na guarda/base da lâmina)
  auto sword_parts = std::make_shared<hittable_list>();
  
  const double BLADE_LENGTH = 130.0;
  
  // 1.1 Lâmina
  // Aponta para cima (Y+), base em (0,0,0)
  auto blade_mesh_obj = std::make_shared<blade_mesh>(
      point3(0, 0, 0), point3(0, BLADE_LENGTH, 0), 
      10, 3, mat_metal, "Lamina", 0.40);
      
  // Rotacionar localmente a lâmina para ficar correta no grupo
  // Rotação: 180 em Z + 90 em Y (igual ao original)
  mat4 bl_R = mat4::rotate_y(degrees_to_radians(90)) * mat4::rotate_z(degrees_to_radians(180));
  mat4 bl_Rinv = mat4::rotate_z_inverse(degrees_to_radians(180)) * mat4::rotate_y_inverse(degrees_to_radians(90));
  // Sem translação, pois o pivô é a origem local
  sword_parts->add(std::make_shared<transform>(blade_mesh_obj, bl_R, bl_Rinv));
  
  // 1.2 Guarda Principal (Horizontal X)
  // Centralizada em (0,0,0) localmente, mas deslocada um pouco para alinhar com a lâmina
  // No original: CX-27.5. Como o centro é CX, deslocamento local é -27.5 em X
  auto guard_main = std::make_shared<cylinder>(
      point3(0, 0, 0), vec3(1, 0, 0), 3.5, 55, mat_gold, "Guarda Principal");
  sword_parts->add(translate_object(guard_main, -27.5, 0, 0));
  
  // Esferas da guarda
  sword_parts->add(std::make_shared<sphere>(point3(-30, 0, 0), 5, mat_gold, "Guarda Esq"));
  sword_parts->add(std::make_shared<sphere>(point3(30, 0, 0), 5, mat_gold, "Guarda Dir"));
  
  // 1.3 Guarda Centro (Vertical Y)
  // Original: GUARD_Y - 4. Em relação a GUARD_Y, é -4.
  auto guard_center = std::make_shared<cylinder>(
      point3(0, 0, 0), vec3(0, 1, 0), 6, 8, mat_gold, "Guarda Centro");
  sword_parts->add(translate_object(guard_center, 0, -4, 0));
  
  // 1.4 Cabo (Vertical Y)
  // Original: GUARD_Y + 4. Relativo: +4
  auto handle = std::make_shared<cylinder>(
      point3(0, 0, 0), vec3(0, 1, 0), 3, 25, mat_leather, "Cabo");
  sword_parts->add(translate_object(handle, 0, 4, 0));
  
  // 1.5 Pomo (Esfera)
  // Original: GUARD_Y + 31. Relativo: +31
  sword_parts->add(std::make_shared<sphere>(point3(0, 31, 0), 4.5, mat_ruby, "Pomo"));
  
  // 1.6 Ponta Decorativa (Cone)
  // Original: GUARD_Y + 35. Relativo: +35
  auto tip_cone = cone::from_base(point3(0, 0, 0), vec3(0, 1, 0), 2.5, 8, mat_gold, "Ponta");
  sword_parts->add(translate_object(std::make_shared<cone>(tip_cone), 0, 35, 0));
  
  // 1.7 Gema Safira (Quaternion Rotation) - Lado Esquerdo da Guarda
  // Material safira azul
  auto mat_sapphire =
      std::make_shared<material>(color(0.1, 0.2, 0.8), // Azul safira
                                 0.2,                  // ka
                                 0.95,                 // ks - muito brilhante
                                 256.0,                // shininess muito alto
                                 "Sapphire Gem");
  // Esfera em origem local, depois rotacionada e transladada
  auto quaternion_sapphire = std::make_shared<sphere>(
      point3(0, 0, 0), 6, mat_sapphire, "Quaternion Sapphire Gem");
  // Rotação de 30 graus em torno do eixo diagonal (1, 1, 1)
  auto rotated_sapphire = rotate_axis_object(quaternion_sapphire, vec3(1, 1, 1),
                                             degrees_to_radians(30));
  // Posição relativa: lado esquerdo da guarda (X = -30, Y = 0, Z = 0)
  sword_parts->add(translate_object(rotated_sapphire, -30, 0, 0));
  
  // 1.8 Gema Esmeralda (Quaternion Rotation) - Lado Direito da Guarda (oposto à Safira)
  auto mat_emerald = std::make_shared<material>(
      color(0.1, 0.7, 0.2), 0.2, 0.9, 200.0, "Emerald Gem");
  auto quaternion_emerald = std::make_shared<sphere>(
      point3(0, 0, 0), 6, mat_emerald, "Quaternion Emerald");
  // Rotação de 60 graus em torno do eixo (0, 1, 1)
  auto rotated_emerald = rotate_axis_object(quaternion_emerald, vec3(0, 1, 1),
                                            degrees_to_radians(60));
  // Posição relativa: encaixada na Guarda Dir (X = +30, Y = 0, Z = 0) - igual à Safira na Guarda Esq
  sword_parts->add(translate_object(rotated_emerald, 30, 0, 0));
  
  // 2. CRIAR TRANSFORM MESTRE DA ESPADA
  // Posição inicial no mundo: (CX, GUARD_Y, CZ)
  // Topo da pedra = MOUNTAIN_HEIGHT + 85 = 130
  // GUARD_Y original = 195.
  const double GUARD_Y = 195.0;
  
  std::string sword_name = "Espada Completa";
  TransformState sword_state;
  sword_state.translation = vec3(CX, GUARD_Y, CZ);
  sword_state.rotation = vec3(0, 0, 0);
  sword_state.scale = vec3(1, 1, 1);
  
  // Criar matrizes iniciais
  mat4 sword_T = mat4::translate(sword_state.translation.x(), sword_state.translation.y(), sword_state.translation.z());
  mat4 sword_Tinv = mat4::translate_inverse(sword_state.translation.x(), sword_state.translation.y(), sword_state.translation.z());
  
  // Criar o transform
  auto sword_transform = std::make_shared<transform>(sword_parts, sword_T, sword_Tinv);
  sword_transform->name = sword_name; // Nome especial para o grupo
  
  // Registrar no mundo e nos mapas globais
  world.add(sword_transform);
  object_states[sword_name] = sword_state;
  object_transforms[sword_name] = sword_transform;
  
  // Mapear também nomes dos componentes para o pai (opcional, mas bom para pick)
  // Na verdade, o pick retorna o nome do objeto (Leaf), mas queremos selecionar o pai
  // Faremos isso na lógica de seleção do GUI Manager ou Main


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
  // ===== PILAR 1 (AGRUPADO E TRANSFORMÁVEL) =====
  // Pilar de ruína com cisalhamento + capitel no topo
  auto pillar1_parts = std::make_shared<hittable_list>();
  
  // 1.1 Cilindro do pilar com cisalhamento (coords locais)
  auto pillar1_cyl = std::make_shared<cylinder>(
      point3(0, 0, 0), vec3(0, 1, 0), 15, 120, mat_ancient_stone, "Ruined Pillar 1");
  mat4 p1_shear = mat4::shear(0.35, 0, 0, 0, 0, 0); // Cisalhamento estilo erosão
  mat4 p1_shear_inv = mat4::shear_inverse(0.35, 0, 0, 0, 0, 0);
  pillar1_parts->add(std::make_shared<transform>(pillar1_cyl, p1_shear, p1_shear_inv));
  
  // 1.2 Capitel (esfera achatada no topo, ajustada pelo shear)
  auto cap1 = std::make_shared<sphere>(point3(0, 0, 0), 20, mat_ancient_stone, "Pillar 1 Capital");
  mat4 cap1_S = mat4::scale(1.3, 0.4, 1.3);
  mat4 cap1_Sinv = mat4::scale_inverse(1.3, 0.4, 1.3);
  // Offset local: altura + deslocamento horizontal pelo shear (0.35 * 120 = 42px em X)
  mat4 cap1_T = mat4::translate(42, 120, 0);
  mat4 cap1_Tinv = mat4::translate_inverse(42, 120, 0);
  pillar1_parts->add(std::make_shared<transform>(cap1, cap1_T * cap1_S, cap1_Sinv * cap1_Tinv));
  
  // 2. CRIAR TRANSFORM MESTRE DO PILAR 1
  std::string pillar1_name = "Pilar Ruina 1";
  TransformState pillar1_state;
  pillar1_state.translation = vec3(CX - 220, 0, CZ + 150);
  pillar1_state.rotation = vec3(0, 0, 0);
  pillar1_state.scale = vec3(1, 1, 1);
  
  mat4 pillar1_T = mat4::translate(pillar1_state.translation.x(), pillar1_state.translation.y(), pillar1_state.translation.z());
  mat4 pillar1_Tinv = mat4::translate_inverse(pillar1_state.translation.x(), pillar1_state.translation.y(), pillar1_state.translation.z());
  
  auto pillar1_transform = std::make_shared<transform>(pillar1_parts, pillar1_T, pillar1_Tinv);
  pillar1_transform->name = pillar1_name;
  
  world.add(pillar1_transform);
  object_states[pillar1_name] = pillar1_state;
  object_transforms[pillar1_name] = pillar1_transform;

  // ===== PILAR 2 (AGRUPADO E TRANSFORMÁVEL) =====
  // Pilar de ruína com cisalhamento oposto + capitel no topo
  auto pillar2_parts = std::make_shared<hittable_list>();
  
  // 2.1 Cilindro do pilar com cisalhamento (coords locais)
  auto pillar2_cyl = std::make_shared<cylinder>(
      point3(0, 0, 0), vec3(0, 1, 0), 15, 130, mat_ancient_stone, "Ruined Pillar 2");
  mat4 p2_shear = mat4::shear(-0.30, 0.10, 0, 0, 0, 0); // Cisalhamento oposto
  mat4 p2_shear_inv = mat4::shear_inverse(-0.30, 0.10, 0, 0, 0, 0);
  pillar2_parts->add(std::make_shared<transform>(pillar2_cyl, p2_shear, p2_shear_inv));
  
  // 2.2 Capitel (esfera achatada no topo, ajustada pelo shear)
  // Shear: -0.30*130 = -39 em X, 0.10*130 = 13 em Z
  auto cap2 = std::make_shared<sphere>(point3(0, 0, 0), 20, mat_ancient_stone, "Pillar 2 Capital");
  mat4 cap2_S = mat4::scale(1.3, 0.4, 1.3);
  mat4 cap2_Sinv = mat4::scale_inverse(1.3, 0.4, 1.3);
  mat4 cap2_local_T = mat4::translate(-39, 130, 13);
  mat4 cap2_local_Tinv = mat4::translate_inverse(-39, 130, 13);
  pillar2_parts->add(std::make_shared<transform>(cap2, cap2_local_T * cap2_S, cap2_Sinv * cap2_local_Tinv));
  
  // 3. CRIAR TRANSFORM MESTRE DO PILAR 2
  std::string pillar2_name = "Pilar Ruina 2";
  TransformState pillar2_state;
  pillar2_state.translation = vec3(CX + 220, 0, CZ + 150);
  pillar2_state.rotation = vec3(0, 0, 0);
  pillar2_state.scale = vec3(1, 1, 1);
  
  mat4 pillar2_T = mat4::translate(pillar2_state.translation.x(), pillar2_state.translation.y(), pillar2_state.translation.z());
  mat4 pillar2_Tinv = mat4::translate_inverse(pillar2_state.translation.x(), pillar2_state.translation.y(), pillar2_state.translation.z());
  
  auto pillar2_transform = std::make_shared<transform>(pillar2_parts, pillar2_T, pillar2_Tinv);
  pillar2_transform->name = pillar2_name;
  
  world.add(pillar2_transform);
  object_states[pillar2_name] = pillar2_state;
  object_transforms[pillar2_name] = pillar2_transform;

  // Pilar 3 - Mais ao fundo, parcialmente destruído (menor e mais inclinado)
  auto pillar3 =
      std::make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 12, 80,
                                 mat_ancient_stone, "Ruined Pillar 3");
  mat4 pillar3_shear = mat4::shear(0.25, 0.30, 0, 0, 0, 0);
  mat4 pillar3_shear_inv = mat4::shear_inverse(0.25, 0.30, 0, 0, 0, 0);
  mat4 pillar3_T = mat4::translate(CX - 180, 0, CZ + 280);
  mat4 pillar3_Tinv = mat4::translate_inverse(CX - 180, 0, CZ + 280);
  world.add(std::make_shared<transform>(pillar3, pillar3_T * pillar3_shear,
                                        pillar3_shear_inv * pillar3_Tinv));

  // Pilar 4 - Quase caído (muito inclinado)
  auto pillar4 =
      std::make_shared<cylinder>(point3(0, 0, 0), vec3(0, 1, 0), 14, 100,
                                 mat_ancient_stone, "Ruined Pillar 4");
  mat4 pillar4_shear = mat4::shear(-0.45, 0.15, 0, 0, 0, 0);
  mat4 pillar4_shear_inv = mat4::shear_inverse(-0.45, 0.15, 0, 0, 0, 0);
  mat4 pillar4_T = mat4::translate(CX + 200, 0, CZ + 300);
  mat4 pillar4_Tinv = mat4::translate_inverse(CX + 200, 0, CZ + 300);
  world.add(std::make_shared<transform>(pillar4, pillar4_T * pillar4_shear,
                                        pillar4_shear_inv * pillar4_Tinv));

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

  // ===== TOCHA MEDIEVAL (AGRUPADA E TRANSFORMÁVEL) =====
  // Posição inicial usando vec4
  vec4 torch_base_pos = vec4(CX - 100, 0, CZ - 80, 1.0);  // (800, 0, 820)
  const double POLE_HEIGHT = 120.0;
  
  // 1. Criar grupo para os componentes da tocha
  // Coordenadas RELATIVAS ao centro da base (pivô no chão)
  auto torch_parts = std::make_shared<hittable_list>();
  
  // 1.1 Poste de madeira (cilindro vertical)
  torch_parts->add(std::make_shared<cylinder>(
      point3(0, 0, 0), vec3(0, 1, 0), 4, POLE_HEIGHT, mat_torch_pole, "Torch Pole"));
  
  // 1.2 Chama externa (cone laranja no topo)
  auto flame_outer = cone::from_base(
      point3(0, 0, 0), vec3(0, 1, 0), 12, 35, mat_torch_flame, "Torch Flame Outer");
  torch_parts->add(translate_object(std::make_shared<cone>(flame_outer), 0, POLE_HEIGHT, 0));
  
  // 1.3 Núcleo da chama (cone amarelo menor)
  auto flame_inner = cone::from_base(
      point3(0, 0, 0), vec3(0, 1, 0), 6, 25, mat_torch_core, "Torch Flame Core");
  torch_parts->add(translate_object(std::make_shared<cone>(flame_inner), 0, POLE_HEIGHT + 5, 0));
  
  // 2. CRIAR TRANSFORM MESTRE DA TOCHA
  std::string torch_name = "Tocha Medieval";
  TransformState torch_state;
  torch_state.translation = vec3(torch_base_pos.x(), torch_base_pos.y(), torch_base_pos.z());
  torch_state.rotation = vec3(0, 0, 0);
  torch_state.scale = vec3(1, 1, 1);
  
  mat4 torch_T = mat4::translate(torch_state.translation.x(), torch_state.translation.y(), torch_state.translation.z());
  mat4 torch_Tinv = mat4::translate_inverse(torch_state.translation.x(), torch_state.translation.y(), torch_state.translation.z());
  
  auto torch_transform = std::make_shared<transform>(torch_parts, torch_T, torch_Tinv);
  torch_transform->name = torch_name;
  
  world.add(torch_transform);
  object_states[torch_name] = torch_state;
  object_transforms[torch_name] = torch_transform;
  
  // 4. FONTE DE LUZ DA TOCHA
  // (Removido daqui e movido para setup_lighting para ser gerenciado dinamicamente)

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

  // ===== 3. GEMAS COM QUATERNION MOVIDAS PARA GRUPO DA ESPADA =====
  // (Ver sword_parts na seção da espada acima)

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
    std::cout << "G - Abrir/Fechar GUI\n";
    std::cout << "1 - Perspectiva | O - Ortografica | P - Obliqua\n";
    std::cout << "+/- - Zoom In/Out\n";
    std::cout << "Click - Pick de objeto\n";
    std::cout << "N - Alternar Dia/Noite\n";
    std::cout << "Q/ESC - Sair\n";
    std::cout << "=================\n\n";
    break;

  case 'g':
  case 'G':
    // Abrir/Fechar GUI
    GUIManager::toggle();
    glutPostRedisplay();
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

  case 'n':
  case 'N':
    // Alternar Dia/Noite
    toggle_day_night(!is_night_mode);
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

  // Inicializar GUI com ponteiros para variáveis de câmera e brilho
  // Inicializar GUI com ponteiros
  GUIManager::init(
      &cam_eye[0], &cam_at[0], &cam_up[0], &current_projection, &need_redraw, &blade_shine_enabled, &is_night_mode, &selected_transform_name
  );
  
  // Configurar callbacks da GUI
  GUIManager::setCallbacks(
      // Callback quando câmera muda
      []() {
          setup_camera();
          glutPostRedisplay();
      },
      // Callback para re-renderizar
      []() {
          need_redraw = true;
          glutPostRedisplay();
      },
      // Callback para toggle do brilho da lâmina
      [](bool increase) {
          toggle_blade_shine(increase);
          glutPostRedisplay();
      },
      // Callback para toggle Dia/Noite
      [](bool set_night) {
          toggle_day_night(set_night);
          glutPostRedisplay();
      },
      // GET Transform State
      [](const std::string& name, double* t, double* r, double* s) -> bool {
          if (object_states.find(name) == object_states.end()) return false;
          TransformState& state = object_states[name];
          t[0] = state.translation.x(); t[1] = state.translation.y(); t[2] = state.translation.z();
          r[0] = state.rotation.x(); r[1] = state.rotation.y(); r[2] = state.rotation.z();
          s[0] = state.scale.x(); s[1] = state.scale.y(); s[2] = state.scale.z();
          return true;
      },
      // SET Transform State
      [](const std::string& name, const double* t, const double* r, const double* s) {
          if (object_states.find(name) == object_states.end()) return;
          TransformState& state = object_states[name];
          state.translation = vec3(t[0], t[1], t[2]);
          state.rotation = vec3(r[0], r[1], r[2]);
          state.scale = vec3(s[0], s[1], s[2]);
          update_object_transform(name);
          
          // Se for a espada e o brilho estiver ligado, atualizar a luz!
          if (name == "Espada Completa" && blade_shine_enabled) {
               update_sword_light();
          }
          
          glutPostRedisplay();
      }
  );

  // Registrar callbacks GLUT
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special_keys);
  glutMouseFunc(mouse);
  glutReshapeFunc(reshape);

  std::cout << "\n=== CONTROLES ===\n";
  std::cout << "WASD - Mover camera\n";
  std::cout << "R/F - Subir/Descer\n";
  std::cout << "Setas - Rotacionar visao\n";
  std::cout << "G - Abrir/Fechar GUI\n";
  std::cout << "C - Reset camera\n";
  std::cout << "1/O/P - Perspectiva/Ortografica/Obliqua\n";
  std::cout << "+/- - Zoom\n";
  std::cout << "Click - Pick | H - Help\n";
  std::cout << "=================\n\n";

  // Loop principal
  glutMainLoop();

  return 0;
}
