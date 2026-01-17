#ifndef QUATERNION_H
#define QUATERNION_H

#include "vec3.h"
#include "mat4.h"
#include <cmath>

// QUATÉRNIOS para rotação em eixo arbitrário (Requisito 1.4.2)
class quaternion {
public:
    double w, x, y, z;

    quaternion() : w(1), x(0), y(0), z(0) {}
    quaternion(double w, double x, double y, double z) : w(w), x(x), y(y), z(z) {}

    // Criar quatérnio a partir de eixo e ângulo
    static quaternion from_axis_angle(const vec3& axis, double angle_rad) {
        vec3 n = unit_vector(axis);
        double half_angle = angle_rad / 2.0;
        double s = std::sin(half_angle);
        return quaternion(
            std::cos(half_angle),
            n.x() * s,
            n.y() * s,
            n.z() * s
        );
    }

    // Multiplicação de quatérnios
    quaternion operator*(const quaternion& q) const {
        return quaternion(
            w*q.w - x*q.x - y*q.y - z*q.z,
            w*q.x + x*q.w + y*q.z - z*q.y,
            w*q.y - x*q.z + y*q.w + z*q.x,
            w*q.z + x*q.y - y*q.x + z*q.w
        );
    }

    // Conjugado
    quaternion conjugate() const {
        return quaternion(w, -x, -y, -z);
    }

    // Norma
    double norm() const {
        return std::sqrt(w*w + x*x + y*y + z*z);
    }

    // Normalizar
    quaternion normalize() const {
        double n = norm();
        return quaternion(w/n, x/n, y/n, z/n);
    }

    // Rotacionar um ponto usando o quatérnio
    vec3 rotate(const vec3& v) const {
        quaternion p(0, v.x(), v.y(), v.z());
        quaternion result = (*this) * p * conjugate();
        return vec3(result.x, result.y, result.z);
    }

    // Converter para matriz de rotação 4x4
    mat4 to_matrix() const {
        double xx = x*x, yy = y*y, zz = z*z;
        double xy = x*y, xz = x*z, yz = y*z;
        double wx = w*x, wy = w*y, wz = w*z;

        return mat4(
            1 - 2*(yy + zz),     2*(xy - wz),     2*(xz + wy), 0,
                2*(xy + wz), 1 - 2*(xx + zz),     2*(yz - wx), 0,
                2*(xz - wy),     2*(yz + wx), 1 - 2*(xx + yy), 0,
                          0,               0,               0, 1
        );
    }

    // Matriz inversa (é só usar o conjugado)
    mat4 to_matrix_inverse() const {
        return conjugate().to_matrix();
    }
};

// Função helper: criar matriz de rotação em eixo arbitrário usando quatérnios
inline mat4 rotate_axis(const vec3& axis, double angle_rad) {
    quaternion q = quaternion::from_axis_angle(axis, angle_rad);
    return q.to_matrix();
}

inline mat4 rotate_axis_inverse(const vec3& axis, double angle_rad) {
    quaternion q = quaternion::from_axis_angle(axis, angle_rad);
    return q.to_matrix_inverse();
}

#endif
