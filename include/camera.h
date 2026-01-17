#ifndef CAMERA_H
#define CAMERA_H

#include "vec3.h"
#include "ray.h"
#include <cmath>

// Tipo de projeção
enum class ProjectionType {
    PERSPECTIVE,    // Obrigatório
    ORTHOGRAPHIC,   // Bônus +0.5
    OBLIQUE         // Bônus +0.5
};

// CÂMERA (Requisito 2)
class camera {
public:
    // Parâmetros da câmera (Requisito 2.1)
    point3 eye;           // Posição da câmera
    point3 at;            // Ponto de mira
    vec3 up;              // Vetor "para cima"
    
    // Parâmetros adicionais (Requisito 2.2)
    double focal_distance; // Distância focal (d)
    double xmin, xmax;     // Janela horizontal
    double ymin, ymax;     // Janela vertical
    
    ProjectionType projection;
    
    // Parâmetros para projeção oblíqua
    double oblique_angle;
    double oblique_strength;

    // Base da câmera (sistema de coordenadas)
    vec3 w, u, v;  // w = back, u = right, v = up

    camera() {
        setup(
            point3(0, 0, 5),
            point3(0, 0, 0),
            vec3(0, 1, 0),
            1.0,
            -1, 1, -1, 1,
            ProjectionType::PERSPECTIVE
        );
    }

    camera(const point3& lookfrom, const point3& lookat, const vec3& vup,
           double d, double x0, double x1, double y0, double y1,
           ProjectionType proj = ProjectionType::PERSPECTIVE) {
        setup(lookfrom, lookat, vup, d, x0, x1, y0, y1, proj);
    }

    void setup(const point3& lookfrom, const point3& lookat, const vec3& vup,
               double d, double x0, double x1, double y0, double y1,
               ProjectionType proj = ProjectionType::PERSPECTIVE) {
        eye = lookfrom;
        at = lookat;
        up = vup;
        focal_distance = d;
        xmin = x0;
        xmax = x1;
        ymin = y0;
        ymax = y1;
        projection = proj;
        oblique_angle = 0.5;
        oblique_strength = 0.5;

        compute_basis();
    }

    void compute_basis() {
        w = unit_vector(eye - at);  // Aponta para trás
        u = unit_vector(cross(up, w));  // Aponta para a direita
        v = cross(w, u);  // Aponta para cima (ortogonal)
    }

    // Gerar raio para um pixel (coordenadas normalizadas 0-1)
    ray get_ray(double s, double t) const {
        // Coordenadas na janela
        double x = xmin + s * (xmax - xmin);
        double y = ymin + t * (ymax - ymin);

        switch (projection) {
            case ProjectionType::PERSPECTIVE:
                return get_perspective_ray(x, y);
            case ProjectionType::ORTHOGRAPHIC:
                return get_orthographic_ray(x, y);
            case ProjectionType::OBLIQUE:
                return get_oblique_ray(x, y);
            default:
                return get_perspective_ray(x, y);
        }
    }

    // PROJEÇÃO PERSPECTIVA (Requisito 3.1)
    ray get_perspective_ray(double x, double y) const {
        // Ponto na janela (plano de projeção)
        point3 pixel_pos = eye - focal_distance * w + x * u + y * v;
        vec3 direction = pixel_pos - eye;
        return ray(eye, direction);
    }

    // PROJEÇÃO ORTOGRÁFICA (Requisito 3.2 - Bônus +0.5)
    ray get_orthographic_ray(double x, double y) const {
        // Origem do raio na janela
        point3 origin = eye + x * u + y * v;
        // Direção sempre perpendicular à janela
        vec3 direction = -w;
        return ray(origin, direction);
    }

    // PROJEÇÃO OBLÍQUA (Requisito 3.3 - Bônus +0.5)
    ray get_oblique_ray(double x, double y) const {
        // Similar a ortográfica, mas com direção angulada
        point3 origin = eye + x * u + y * v;
        // Adiciona componente oblíquo
        double shear_x = oblique_strength * std::cos(oblique_angle);
        double shear_y = oblique_strength * std::sin(oblique_angle);
        vec3 direction = unit_vector(-w + shear_x * u + shear_y * v);
        return ray(origin, direction);
    }

    // ZOOM IN - diminuir campo de visão (Requisito 3.1.1.2)
    void zoom_in(double factor = 0.8) {
        // Opção 2: diminuir tamanho da janela
        double cx = (xmin + xmax) / 2;
        double cy = (ymin + ymax) / 2;
        double hw = (xmax - xmin) / 2 * factor;
        double hh = (ymax - ymin) / 2 * factor;
        xmin = cx - hw;
        xmax = cx + hw;
        ymin = cy - hh;
        ymax = cy + hh;
    }

    // ZOOM OUT - aumentar campo de visão (Requisito 3.1.1.1)
    void zoom_out(double factor = 1.25) {
        zoom_in(factor);  // Inverso do zoom in
    }

    // Alterar distância focal (outra forma de zoom)
    void set_focal_distance(double d) {
        focal_distance = d;
    }

    // Mover câmera para mais perto do alvo (zoom via aproximação)
    void move_closer(double amount) {
        vec3 dir = unit_vector(at - eye);
        eye = eye + dir * amount;
        compute_basis();
    }

    // Mover câmera para mais longe do alvo
    void move_away(double amount) {
        move_closer(-amount);
    }

    // === PERSPECTIVAS COM PONTOS DE FUGA (Requisito 3.1.2) ===

    // Configurar para 1 ponto de fuga (câmera alinhada com um eixo)
    // A câmera olha diretamente ao longo de um eixo
    void setup_one_vanishing_point(const point3& target, double distance) {
        // Câmera olhando diretamente para Z (ou qualquer eixo)
        at = target;
        eye = target + vec3(0, 0, distance);
        up = vec3(0, 1, 0);
        compute_basis();
    }

    // 2 pontos de fuga (câmera rotacionada em torno de Y apenas)
    void setup_two_vanishing_points(const point3& target, double distance, double angle_y) {
        at = target;
        double x = distance * std::sin(angle_y);
        double z = distance * std::cos(angle_y);
        eye = target + vec3(x, 0, z);
        up = vec3(0, 1, 0);
        compute_basis();
    }

    // 3 pontos de fuga (câmera rotacionada em Y e X/Z)
    void setup_three_vanishing_points(const point3& target, double distance, 
                                       double angle_y, double angle_elevation) {
        at = target;
        double y = distance * std::sin(angle_elevation);
        double horiz_dist = distance * std::cos(angle_elevation);
        double x = horiz_dist * std::sin(angle_y);
        double z = horiz_dist * std::cos(angle_y);
        eye = target + vec3(x, y, z);
        up = vec3(0, 1, 0);
        compute_basis();
    }
};

#endif
