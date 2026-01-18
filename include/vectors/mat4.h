#ifndef MAT4_H
#define MAT4_H

#include "vec4.h"
#include "vec3.h"
#include <cmath>

class mat4 {
public:
    double m[4][4];

    // Construtor identidade
    mat4() {
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                m[i][j] = (i == j) ? 1.0 : 0.0;
    }

    // Construtor com valores
    mat4(double m00, double m01, double m02, double m03,
         double m10, double m11, double m12, double m13,
         double m20, double m21, double m22, double m23,
         double m30, double m31, double m32, double m33) {
        m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
        m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
        m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
        m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
    }

    // Multiplicação matriz x vetor
    vec4 operator*(const vec4& v) const {
        return vec4(
            m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2] + m[0][3]*v[3],
            m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2] + m[1][3]*v[3],
            m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2] + m[2][3]*v[3],
            m[3][0]*v[0] + m[3][1]*v[1] + m[3][2]*v[2] + m[3][3]*v[3]
        );
    }

    // Multiplicação matriz x matriz
    mat4 operator*(const mat4& other) const {
        mat4 result;
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                result.m[i][j] = 0;
                for (int k = 0; k < 4; k++) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }

    // ==================== TRANSFORMAÇÕES ====================

    // TRANSLAÇÃO (Requisito 1.4.1)
    static mat4 translate(double tx, double ty, double tz) {
        return mat4(
            1, 0, 0, tx,
            0, 1, 0, ty,
            0, 0, 1, tz,
            0, 0, 0, 1
        );
    }

    static mat4 translate(const vec3& t) {
        return translate(t.x(), t.y(), t.z());
    }

    static mat4 translate_inverse(double tx, double ty, double tz) {
        return translate(-tx, -ty, -tz);
    }

    // ESCALA (Requisito 1.4.3)
    static mat4 scale(double sx, double sy, double sz) {
        return mat4(
            sx, 0, 0, 0,
            0, sy, 0, 0,
            0, 0, sz, 0,
            0, 0, 0, 1
        );
    }

    static mat4 scale_inverse(double sx, double sy, double sz) {
        return scale(1.0/sx, 1.0/sy, 1.0/sz);
    }

    // ROTAÇÃO EM X (Requisito 1.4.2)
    static mat4 rotate_x(double angle_rad) {
        double c = std::cos(angle_rad);
        double s = std::sin(angle_rad);
        return mat4(
            1, 0, 0, 0,
            0, c, -s, 0,
            0, s, c, 0,
            0, 0, 0, 1
        );
    }

    static mat4 rotate_x_inverse(double angle_rad) {
        return rotate_x(-angle_rad);
    }

    // ROTAÇÃO EM Y
    static mat4 rotate_y(double angle_rad) {
        double c = std::cos(angle_rad);
        double s = std::sin(angle_rad);
        return mat4(
            c, 0, s, 0,
            0, 1, 0, 0,
            -s, 0, c, 0,
            0, 0, 0, 1
        );
    }

    static mat4 rotate_y_inverse(double angle_rad) {
        return rotate_y(-angle_rad);
    }

    // ROTAÇÃO EM Z
    static mat4 rotate_z(double angle_rad) {
        double c = std::cos(angle_rad);
        double s = std::sin(angle_rad);
        return mat4(
            c, -s, 0, 0,
            s, c, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        );
    }

    static mat4 rotate_z_inverse(double angle_rad) {
        return rotate_z(-angle_rad);
    }

    // CISALHAMENTO (Requisito 1.4.4 - Bônus +0.5)
    // Cisalhamento em X baseado em Y e Z
    static mat4 shear(double xy, double xz, double yx, double yz, double zx, double zy) {
        return mat4(
            1, xy, xz, 0,
            yx, 1, yz, 0,
            zx, zy, 1, 0,
            0, 0, 0, 1
        );
    }

    static mat4 shear_inverse(double xy, double xz, double yx, double yz, double zx, double zy) {
        // Para cisalhamentos pequenos, a inversa aproximada é a negação
        return shear(-xy, -xz, -yx, -yz, -zx, -zy);
    }

    // ESPELHO EM PLANO ARBITRÁRIO (Requisito 1.4.5 - Bônus +0.5)
    // Espelha em relação a um plano que passa pela origem com normal n
    static mat4 reflect(const vec3& n) {
        vec3 nn = unit_vector(n);
        double a = nn.x(), b = nn.y(), c = nn.z();
        return mat4(
            1-2*a*a, -2*a*b, -2*a*c, 0,
            -2*a*b, 1-2*b*b, -2*b*c, 0,
            -2*a*c, -2*b*c, 1-2*c*c, 0,
            0, 0, 0, 1
        );
    }

    // Espelho em relação a um plano arbitrário (ponto + normal)
    static mat4 reflect_plane(const vec3& point, const vec3& normal) {
        mat4 T = translate(-point.x(), -point.y(), -point.z());
        mat4 R = reflect(normal);
        mat4 Tinv = translate(point.x(), point.y(), point.z());
        return Tinv * R * T;
    }

    // Transposta (útil para normais)
    mat4 transpose() const {
        mat4 result;
        for (int i = 0; i < 4; i++)
            for (int j = 0; j < 4; j++)
                result.m[i][j] = m[j][i];
        return result;
    }

    // Matriz identidade
    static mat4 identity() {
        return mat4();
    }
};

#endif
