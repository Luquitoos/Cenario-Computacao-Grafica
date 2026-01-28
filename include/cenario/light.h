#ifndef LIGHT_H
#define LIGHT_H

#include "../colors/color.h"
#include "../vectors/vec3.h"
#include <cmath>
#include <string>

class light {
public:
  // [Requisito 1.5] Fontes Luminosas (Obrigatório)
  // Classe base abstrata para todos os tipos de luzes.
  // Suporta intensidade (cor) e identificação.
  color intensity;
  std::string name;
  bool enabled;

  // Raio/alcance opcional para luzes que suportam decaimento com a distância.
  // Se <= 0, é tratado como infinito / ignorado.
  double reach;

  light(std::string n = "Light")
      : intensity(1, 1, 1), name(n), enabled(true), reach(-1.0) {}
  light(const color &i, std::string n = "Light")
      : intensity(i), name(n), enabled(true), reach(-1.0) {}

  virtual ~light() = default;
  virtual vec3 get_direction(const point3 &point) const = 0;
  virtual double get_distance(const point3 &point) const = 0;
  virtual color get_intensity(const point3 &point) const = 0;
  virtual point3 get_position() const = 0;

  // Setters para a GUI
  virtual void set_position(const point3 &p) {}
  virtual void set_direction(const vec3 &d) {}
  virtual void set_reach(double r) { reach = r; }
  virtual bool supports_reach() const { return false; }
};

// [Requisito 1.5.4] Luz Ambiente (Obrigatório)
// Luz constante que ilumina a cena uniformemente, simulando iluminação global
// indireta.
class ambient_light {
public:
  color intensity;
  std::string name;
  bool enabled;

  ambient_light(std::string n = "Ambient")
      : intensity(0.1, 0.1, 0.1), name(n), enabled(true) {}
  ambient_light(const color &i, std::string n = "Ambient")
      : intensity(i), name(n), enabled(true) {}
  ambient_light(double r, double g, double b, std::string n = "Ambient")
      : intensity(r, g, b), name(n), enabled(true) {}
};

// [Requisito 1.5.1] Luz Pontual (Obrigatório)
// Emite luz em todas as direções a partir de uma posição (position).
// Possui atenuação com a distância (c1, c2, c3).
class point_light : public light {
public:
  point3 position;
  double c1, c2, c3; // Coeficientes de atenuação

  point_light(std::string n = "Point Light")
      : light(n), position(0, 0, 0), c1(1), c2(0), c3(0) {}

  point_light(const point3 &pos, const color &i, double att_c1 = 1.0,
              double att_c2 = 0.0, double att_c3 = 0.0,
              std::string n = "Point Light")
      : light(i, n), position(pos), c1(att_c1), c2(att_c2), c3(att_c3) {}

  vec3 get_direction(const point3 &point) const override {
    return unit_vector(position - point);
  }

  double get_distance(const point3 &point) const override {
    return (position - point).length();
  }

  color get_intensity(const point3 &point) const override {
    if (!enabled)
      return color(0, 0, 0);

    double d = get_distance(point);
    if (supports_reach() && reach > 0.0 && d > reach)
      return color(0, 0, 0);

    double attenuation = 1.0 / (c1 + c2 * d + c3 * d * d);
    return intensity * attenuation;
  }

  bool supports_reach() const override { return true; }

  point3 get_position() const override { return position; }
  void set_position(const point3 &p) override { position = p; }
};

// [Requisito 1.5.2] Luz Spot (Obrigatório)
// Emite luz a partir de uma posição, restrita a um cone de direção.
// Possui ângulos nterno (full intensity) e externo (cutoff) para suavização
// (soft edge).
class spot_light : public light {
public:
  point3 position;
  vec3 direction;
  double inner_angle;
  double outer_angle;
  double c1, c2, c3;

  spot_light(std::string n = "Spot Light")
      : light(n), position(0, 0, 0), direction(0, -1, 0), inner_angle(0.3),
        outer_angle(0.5), c1(1), c2(0), c3(0) {}

  spot_light(const point3 &pos, const vec3 &dir, const color &i,
             double inner_ang, double outer_ang, double att_c1 = 1.0,
             double att_c2 = 0.0, double att_c3 = 0.0,
             std::string n = "Spot Light")
      : light(i, n), position(pos), direction(unit_vector(dir)),
        inner_angle(inner_ang), outer_angle(outer_ang), c1(att_c1), c2(att_c2),
        c3(att_c3) {}

  vec3 get_direction(const point3 &point) const override {
    return unit_vector(position - point);
  }

  double get_distance(const point3 &point) const override {
    return (position - point).length();
  }

  color get_intensity(const point3 &point) const override {
    if (!enabled)
      return color(0, 0, 0);

    double d = get_distance(point);
    if (supports_reach() && reach > 0.0 && d > reach)
      return color(0, 0, 0);

    vec3 L = unit_vector(point - position);
    double cos_angle = dot(L, direction);
    double angle = std::acos(cos_angle);

    if (angle > outer_angle) {
      return color(0, 0, 0);
    }

    double attenuation = 1.0 / (c1 + c2 * d + c3 * d * d);

    if (angle < inner_angle) {
      return intensity * attenuation;
    }

    double falloff = (outer_angle - angle) / (outer_angle - inner_angle);
    return intensity * attenuation * falloff;
  }

  bool supports_reach() const override { return true; }

  point3 get_position() const override { return position; }
  void set_position(const point3 &p) override { position = p; }
  void set_direction(const vec3 &d) override { direction = unit_vector(d); }
};

// [Requisito 1.5.3] Luz Direcional (Obrigatório)
// Simula uma fonte de luz infinitamente distante (como o Sol).
// Os raios de luz são paralelos e definidos apenas por uma direção (direction).
class directional_light : public light {
public:
  // Para a GUI: armazena uma posição + alvo para permitir alterar a direção
  // reposicionando a luz (direção = normalize(alvo - posição)).
  point3 position;
  point3 target;
  vec3 direction;

  directional_light(std::string n = "Directional Light")
      : light(n), position(0, 100, 0), target(0, 0, 0), direction(0, -1, 0) {
    update_direction();
  }

  directional_light(const vec3 &dir, const color &i,
                    std::string n = "Directional Light")
      : light(i, n), position(0, 0, 0), target(unit_vector(dir)),
        direction(unit_vector(dir)) {}

  void update_direction() {
    vec3 v = target - position;
    if (v.length_squared() < 1e-12)
      v = vec3(0, -1, 0);
    direction = unit_vector(v);
  }

  vec3 get_direction(const point3 &point) const override { return -direction; }

  double get_distance(const point3 &point) const override { return 1e30; }

  color get_intensity(const point3 &point) const override {
    if (!enabled)
      return color(0, 0, 0);
    return intensity;
  }

  point3 get_position() const override { return position; }

  void set_position(const point3 &p) override {
    position = p;
    update_direction();
  }

  void set_direction(const vec3 &d) override {
    direction = unit_vector(d);
    target = position + direction;
  }

  void set_target(const point3 &t) {
    target = t;
    update_direction();
  }

  point3 get_target() const { return target; }
};

#endif
