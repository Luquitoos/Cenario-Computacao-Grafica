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
 * - 6: Imagem 500x500 pixels
 */

#include <iostream>
#include <vector>
#include <memory>
#include <string>
#include <cmath>

#include "../include/utils.h"
#include "../include/vec3.h"
#include "../include/vec4.h"
#include "../include/mat4.h"
#include "../include/quaternion.h"
#include "../include/ray.h"
#include "../include/color.h"
#include "../include/texture.h"
#include "../include/material.h"
#include "../include/hittable.h"
#include "../include/hittable_list.h"
#include "../include/sphere.h"
#include "../include/plane.h"
#include "../include/cylinder.h"
#include "../include/cone.h"
#include "../include/triangle.h"
#include "../include/mesh.h"
#include "../include/transform.h"
#include "../include/light.h"
#include "../include/camera.h"

#include <GL/freeglut.h>

// ==================== CONFIGURAÇÕES GLOBAIS ====================
const int IMAGE_WIDTH = 500;
const int IMAGE_HEIGHT = 500;
GLubyte* PixelBuffer = nullptr;

// Cena
hittable_list world;
camera cam;
ambient_light ambient;
std::vector<std::shared_ptr<light>> lights;

// Estado da interface
int current_projection = 0;  // 0=perspectiva, 1=ortográfica, 2=oblíqua
int vanishing_points = 3;    // 1, 2 ou 3 pontos de fuga
bool need_redraw = true;
std::string picked_object = "";

// ==================== MODELO DE ILUMINAÇÃO PHONG ====================
color calculate_lighting(const hit_record& rec, const ray& r,
                         const hittable_list& world) {
    color result(0, 0, 0);
    
    // Componente ambiente
    color diffuse_color = rec.mat->get_diffuse(rec.u, rec.v, rec.p);
    result = result + rec.mat->ka * diffuse_color;
    
    // Para cada fonte de luz
    for (const auto& light_ptr : lights) {
        vec3 L = light_ptr->get_direction(rec.p);
        double light_dist = light_ptr->get_distance(rec.p);
        
        // SOMBRAS (Requisito 4)
        ray shadow_ray(rec.p + 0.001 * rec.normal, L);
        hit_record shadow_rec;
        if (world.hit(shadow_ray, 0.001, light_dist - 0.001, shadow_rec)) {
            continue;  // Ponto está na sombra desta luz
        }
        
        color light_intensity = light_ptr->get_intensity(rec.p);
        
        // Componente difusa
        double diff = std::max(0.0, dot(rec.normal, L));
        result = result + diffuse_color * light_intensity * diff;
        
        // Componente especular (Blinn-Phong)
        vec3 V = unit_vector(-r.direction());
        vec3 H = unit_vector(L + V);
        double spec = std::pow(std::max(0.0, dot(rec.normal, H)), rec.mat->shininess);
        result = result + rec.mat->ks * light_intensity * spec;
    }
    
    return result.clamp();
}

// ==================== RAY CASTING ====================
color ray_color(const ray& r, const hittable_list& world) {
    hit_record rec;
    
    if (world.hit(r, 0.001, infinity, rec)) {
        return calculate_lighting(rec, r, world);
    }
    
    // Cor de fundo (gradiente de céu)
    vec3 unit_direction = unit_vector(r.direction());
    double t = 0.5 * (unit_direction.y() + 1.0);
    color sky_top(0.4, 0.5, 0.7);
    color sky_bottom(0.7, 0.75, 0.8);
    return sky_bottom * (1.0 - t) + sky_top * t;
}

// ==================== FUNÇÃO DE PICK (Requisito 5.1) ====================
void perform_pick(int mouse_x, int mouse_y) {
    // Converter coordenadas de tela para coordenadas normalizadas
    double u = double(mouse_x) / IMAGE_WIDTH;
    double v = double(IMAGE_HEIGHT - mouse_y) / IMAGE_HEIGHT;  // Inverter Y
    
    ray r = cam.get_ray(u, v);
    hit_record rec;
    
    std::cout << "\n========== PICK ==========\n";
    std::cout << "Mouse: (" << mouse_x << ", " << mouse_y << ")\n";
    std::cout << "UV: (" << u << ", " << v << ")\n";
    
    if (world.hit(r, 0.001, infinity, rec)) {
        picked_object = rec.object_name;
        std::cout << "OBJETO: " << rec.object_name << "\n";
        std::cout << "Material: " << rec.mat->name << "\n";
        std::cout << "Posicao: (" << rec.p.x() << ", " << rec.p.y() << ", " << rec.p.z() << ")\n";
        std::cout << "Normal: (" << rec.normal.x() << ", " << rec.normal.y() << ", " << rec.normal.z() << ")\n";
        std::cout << "Distancia (t): " << rec.t << "\n";
    } else {
        picked_object = "Fundo (Ceu)";
        std::cout << "OBJETO: Fundo (Ceu)\n";
    }
    std::cout << "==========================\n";
}

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
    auto mat_floor = materials::floor();
    auto mat_wall = materials::wall();

    // ===== CONSTANTES DE POSIÇÃO (PRIMEIRO OCTANTE) =====
    // Centro da cena em X=250, Z=250, Y começa em 0
    const double CX = 250.0;
    const double CZ = 250.0;
    
    // ===== PLANOS (CHÃO E PAREDES) =====
    world.add(std::make_shared<plane>(
        point3(0, 0, 0), vec3(0, 1, 0), mat_floor, "Chao"));
    
    world.add(std::make_shared<plane>(
        point3(0, 0, 500), vec3(0, 0, -1), mat_wall, "Parede Fundo"));
    
    world.add(std::make_shared<plane>(
        point3(0, 0, 0), vec3(0, 0, 1), mat_wall, "Parede Frente"));
    
    world.add(std::make_shared<plane>(
        point3(500, 0, 0), vec3(-1, 0, 0), mat_wall, "Parede Direita"));
    
    world.add(std::make_shared<plane>(
        point3(0, 0, 0), vec3(1, 0, 0), mat_wall, "Parede Esquerda"));

    // ===== PEDRA BASE (MALHA) =====
    // Pedra grande onde a espada está cravada
    auto stone_base = std::make_shared<box_mesh>(
        point3(CX - 40, 0, CZ - 30),
        point3(CX + 40, 50, CZ + 30),
        mat_stone, "Pedra Base"
    );
    world.add(stone_base);
    
    // Pedra menor em cima (onde a espada entra)
    auto stone_top = std::make_shared<box_mesh>(
        point3(CX - 25, 50, CZ - 20),
        point3(CX + 25, 70, CZ + 20),
        mat_stone, "Pedra Topo"
    );
    world.add(stone_top);

    // ===== ESPADA =====
    // A espada está inclinada levemente (rotação em X)
    const double sword_angle = degrees_to_radians(10);  // 10 graus de inclinação
    
    // 1. Lâmina (MALHA) - cravada na pedra, ponta para cima
    auto blade_base = std::make_shared<blade_mesh>(
        point3(0, 0, 0),           // Base da lâmina (centro)
        point3(0, 120, 0),         // Ponta
        10, 3,                      // Largura, espessura
        mat_metal, "Lamina da Espada"
    );
    // Aplicar transformação: rotação + translação
    mat4 blade_R = mat4::rotate_z(sword_angle);
    mat4 blade_T = mat4::translate(CX, 30, CZ);
    mat4 blade_M = blade_T * blade_R;
    mat4 blade_Rinv = mat4::rotate_z_inverse(sword_angle);
    mat4 blade_Tinv = mat4::translate_inverse(CX, 30, CZ);
    mat4 blade_Minv = blade_Rinv * blade_Tinv;
    world.add(std::make_shared<transform>(blade_base, blade_M, blade_Minv));

    // 2. Guarda (CILINDRO) - crossguard
    auto guard = std::make_shared<cylinder>(
        point3(0, 0, 0),           // Centro
        vec3(1, 0, 0),             // Eixo horizontal
        3, 50,                      // Raio, comprimento
        mat_gold, "Guarda da Espada"
    );
    mat4 guard_T = mat4::translate(CX - 25, 145, CZ);
    mat4 guard_Tinv = mat4::translate_inverse(CX - 25, 145, CZ);
    world.add(std::make_shared<transform>(guard, guard_T, guard_Tinv));

    // 3. Cabo (CILINDRO) - punho
    auto handle = std::make_shared<cylinder>(
        point3(0, 0, 0),
        vec3(0, 1, 0),            // Eixo vertical
        4, 35,                     // Raio, altura
        mat_leather, "Cabo da Espada"
    );
    mat4 handle_R = mat4::rotate_z(sword_angle);
    mat4 handle_T = mat4::translate(CX, 145, CZ);
    mat4 handle_M = handle_T * handle_R;
    mat4 handle_Rinv = mat4::rotate_z_inverse(sword_angle);
    mat4 handle_Tinv = mat4::translate_inverse(CX, 145, CZ);
    mat4 handle_Minv = handle_Rinv * handle_Tinv;
    world.add(std::make_shared<transform>(handle, handle_M, handle_Minv));

    // 4. Pomo (ESFERA) - gema no topo
    auto pommel = std::make_shared<sphere>(
        point3(CX + 5, 185, CZ), 8, mat_ruby, "Gema do Pomo"
    );
    world.add(pommel);

    // ===== DECORAÇÕES =====
    
    // CONE decorativo (ponta do pomo)
    auto pommel_tip = cone::from_base(
        point3(0, 0, 0),
        vec3(0, 1, 0),
        4, 12,
        mat_gold, "Ponta do Pomo"
    );
    mat4 tip_T = mat4::translate(CX + 5, 193, CZ);
    mat4 tip_Tinv = mat4::translate_inverse(CX + 5, 193, CZ);
    world.add(std::make_shared<transform>(
        std::make_shared<cone>(pommel_tip), tip_T, tip_Tinv));

    // Esferas decorativas ao redor da pedra
    world.add(std::make_shared<sphere>(
        point3(CX - 60, 15, CZ - 40), 15, mat_stone, "Pedra Decorativa 1"));
    world.add(std::make_shared<sphere>(
        point3(CX + 65, 12, CZ + 35), 12, mat_stone, "Pedra Decorativa 2"));
    world.add(std::make_shared<sphere>(
        point3(CX + 50, 10, CZ - 50), 10, mat_stone, "Pedra Decorativa 3"));

    // ===== DEMONSTRAÇÃO DE TRANSFORMAÇÕES =====
    
    // CISALHAMENTO (Requisito 1.4.4)
    auto sheared_box = std::make_shared<box_mesh>(
        point3(0, 0, 0), point3(30, 30, 30), mat_stone, "Caixa Cisalhada"
    );
    auto sheared = shear_object(sheared_box, 0.3, 0, 0, 0, 0, 0);
    mat4 shear_T = mat4::translate(CX + 100, 0, CZ - 80);
    mat4 shear_Tinv = mat4::translate_inverse(CX + 100, 0, CZ - 80);
    world.add(std::make_shared<transform>(sheared, shear_T, shear_Tinv));

    // ROTAÇÃO EM EIXO ARBITRÁRIO (Requisito 1.4.2 - Quatérnios)
    auto rotated_box = std::make_shared<box_mesh>(
        point3(-15, -15, -15), point3(15, 15, 15), mat_gold, "Caixa Rotacionada"
    );
    vec3 arbitrary_axis(1, 1, 1);  // Eixo diagonal
    auto rotated = rotate_axis_object(rotated_box, arbitrary_axis, degrees_to_radians(30));
    mat4 rot_T = mat4::translate(CX - 100, 30, CZ + 80);
    mat4 rot_Tinv = mat4::translate_inverse(CX - 100, 30, CZ + 80);
    world.add(std::make_shared<transform>(rotated, rot_T, rot_Tinv));

    // ESPELHO (Requisito 1.4.5)
    auto original_sphere = std::make_shared<sphere>(
        point3(CX - 80, 25, CZ), 20, mat_ruby, "Esfera Original"
    );
    world.add(original_sphere);
    
    // Esfera espelhada em relação ao plano YZ passando por CX
    auto reflected = reflect_object(original_sphere, point3(CX, 0, CZ), vec3(1, 0, 0));
    world.add(reflected);

    // ===== ILUMINAÇÃO =====
    
    // Luz ambiente (Requisito 1.5.4)
    ambient = ambient_light(0.2, 0.2, 0.25);
    
    // Luz pontual principal (Requisito 1.5.1)
    lights.push_back(std::make_shared<point_light>(
        point3(CX + 50, 300, CZ - 100),
        color(1.0, 0.95, 0.9),
        1.0, 0.0001, 0.00001
    ));
    
    // Luz spot iluminando a espada (Requisito 1.5.2)
    lights.push_back(std::make_shared<spot_light>(
        point3(CX, 350, CZ),
        vec3(0, -1, 0),
        color(1.0, 0.9, 0.7),
        degrees_to_radians(15),
        degrees_to_radians(25),
        1.0, 0.0001, 0.00001
    ));
    
    // Luz direcional (sol) (Requisito 1.5.3)
    lights.push_back(std::make_shared<directional_light>(
        vec3(-0.3, -1.0, -0.5),
        color(0.4, 0.4, 0.5)
    ));

    // ===== CÂMERA =====
    setup_camera();
}

void setup_camera() {
    const double CX = 250.0;
    const double CZ = 250.0;
    
    double window_size = 150.0;

    switch (current_projection) {
        case 0:  // Perspectiva
            cam.projection = ProjectionType::PERSPECTIVE;
            switch (vanishing_points) {
                case 1:  // 1 ponto de fuga - olhando de frente
                    cam.setup(
                        point3(CX, 100, CZ - 300),  // Eye - de frente
                        point3(CX, 80, CZ),          // At
                        vec3(0, 1, 0),               // Up
                        100.0,
                        -window_size, window_size, -window_size, window_size,
                        ProjectionType::PERSPECTIVE
                    );
                    break;
                case 2:  // 2 pontos de fuga - rotacionado em Y
                    cam.setup(
                        point3(CX + 200, 100, CZ - 200),
                        point3(CX, 80, CZ),
                        vec3(0, 1, 0),
                        100.0,
                        -window_size, window_size, -window_size, window_size,
                        ProjectionType::PERSPECTIVE
                    );
                    break;
                case 3:  // 3 pontos de fuga - rotacionado em Y e elevado
                default:
                    cam.setup(
                        point3(CX + 180, 200, CZ - 180),
                        point3(CX, 60, CZ),
                        vec3(0, 1, 0),
                        100.0,
                        -window_size, window_size, -window_size, window_size,
                        ProjectionType::PERSPECTIVE
                    );
                    break;
            }
            break;
            
        case 1:  // Ortográfica
            cam.setup(
                point3(CX + 200, 150, CZ - 200),
                point3(CX, 80, CZ),
                vec3(0, 1, 0),
                100.0,
                -window_size, window_size, -window_size, window_size,
                ProjectionType::ORTHOGRAPHIC
            );
            break;
            
        case 2:  // Oblíqua
            cam.setup(
                point3(CX, 150, CZ - 300),
                point3(CX, 80, CZ),
                vec3(0, 1, 0),
                100.0,
                -window_size, window_size, -window_size, window_size,
                ProjectionType::OBLIQUE
            );
            cam.oblique_angle = 0.5;
            cam.oblique_strength = 0.3;
            break;
    }
}

// ==================== RENDERIZAÇÃO ====================
void render() {
    std::cout << "Renderizando " << IMAGE_WIDTH << "x" << IMAGE_HEIGHT << " pixels...\n";
    
    for (int j = 0; j < IMAGE_HEIGHT; j++) {
        if (j % 50 == 0) {
            std::cout << "Linha " << j << "/" << IMAGE_HEIGHT << "\r" << std::flush;
        }
        
        for (int i = 0; i < IMAGE_WIDTH; i++) {
            double u = double(i) / (IMAGE_WIDTH - 1);
            double v = double(j) / (IMAGE_HEIGHT - 1);
            
            ray r = cam.get_ray(u, v);
            color pixel_color = ray_color(r, world);
            
            // Converter para buffer (Y invertido para OpenGL)
            int flipped_j = IMAGE_HEIGHT - 1 - j;
            int idx = (flipped_j * IMAGE_WIDTH + i) * 3;
            
            PixelBuffer[idx] = pixel_color.r_byte();
            PixelBuffer[idx + 1] = pixel_color.g_byte();
            PixelBuffer[idx + 2] = pixel_color.b_byte();
        }
    }
    
    std::cout << "Renderizacao concluida!               \n";
    need_redraw = false;
}

// ==================== CALLBACKS GLUT ====================
void display() {
    if (need_redraw) {
        render();
    }
    
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE, PixelBuffer);
    
    // Mostrar informações na tela
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(-0.95f, 0.9f);
    
    std::string info = "Projecao: ";
    switch (current_projection) {
        case 0: info += "Perspectiva (" + std::to_string(vanishing_points) + " PF)"; break;
        case 1: info += "Ortografica"; break;
        case 2: info += "Obliqua"; break;
    }
    for (char c : info) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
    
    if (!picked_object.empty()) {
        glRasterPos2f(-0.95f, 0.8f);
        std::string pick_info = "Ultimo Pick: " + picked_object;
        for (char c : pick_info) {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
        }
    }
    
    glFlush();
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 27:  // ESC
        case 'q':
        case 'Q':
            std::cout << "Encerrando...\n";
            exit(0);
            break;
            
        case '1':  // 1 ponto de fuga
            current_projection = 0;
            vanishing_points = 1;
            setup_camera();
            need_redraw = true;
            std::cout << "Perspectiva com 1 ponto de fuga\n";
            break;
            
        case '2':  // 2 pontos de fuga
            current_projection = 0;
            vanishing_points = 2;
            setup_camera();
            need_redraw = true;
            std::cout << "Perspectiva com 2 pontos de fuga\n";
            break;
            
        case '3':  // 3 pontos de fuga
            current_projection = 0;
            vanishing_points = 3;
            setup_camera();
            need_redraw = true;
            std::cout << "Perspectiva com 3 pontos de fuga\n";
            break;
            
        case 'o':
        case 'O':  // Projeção ortográfica
            current_projection = 1;
            setup_camera();
            need_redraw = true;
            std::cout << "Projecao Ortografica\n";
            break;
            
        case 'b':
        case 'B':  // Projeção oblíqua
            current_projection = 2;
            setup_camera();
            need_redraw = true;
            std::cout << "Projecao Obliqua\n";
            break;
            
        case '+':
        case '=':  // Zoom in
            cam.zoom_in(0.8);
            need_redraw = true;
            std::cout << "Zoom In\n";
            break;
            
        case '-':
        case '_':  // Zoom out
            cam.zoom_out(1.25);
            need_redraw = true;
            std::cout << "Zoom Out\n";
            break;
            
        case 'h':
        case 'H':  // Help
            std::cout << "\n=== CONTROLES ===\n";
            std::cout << "1, 2, 3 - Perspectiva com 1/2/3 pontos de fuga\n";
            std::cout << "O - Projecao Ortografica\n";
            std::cout << "B - Projecao Obliqua\n";
            std::cout << "+/- - Zoom In/Out\n";
            std::cout << "Click - Pick de objeto\n";
            std::cout << "Q/ESC - Sair\n";
            std::cout << "=================\n\n";
            break;
    }
    
    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
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
int main(int argc, char** argv) {
    std::cout << "============================================\n";
    std::cout << "  COMPUTACAO GRAFICA - ESPADA NA PEDRA\n";
    std::cout << "============================================\n\n";
    
    // Inicializar buffer
    PixelBuffer = new GLubyte[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
    
    // Criar cena
    std::cout << "Criando cena...\n";
    create_scene();
    
    // Inicializar GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(IMAGE_WIDTH, IMAGE_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Computacao Grafica - Espada na Pedra");
    
    // Configurar OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Registrar callbacks
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutReshapeFunc(reshape);
    
    std::cout << "\n=== CONTROLES ===\n";
    std::cout << "1, 2, 3 - Perspectiva com 1/2/3 pontos de fuga\n";
    std::cout << "O - Projecao Ortografica\n";
    std::cout << "B - Projecao Obliqua\n";
    std::cout << "+/- - Zoom In/Out\n";
    std::cout << "Click - Pick de objeto\n";
    std::cout << "Q/ESC - Sair\n";
    std::cout << "=================\n\n";
    
    // Render inicial
    render();
    
    // Loop principal
    glutMainLoop();
    
    // Cleanup
    delete[] PixelBuffer;
    
    return 0;
}
