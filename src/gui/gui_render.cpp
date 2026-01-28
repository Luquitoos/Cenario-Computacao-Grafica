#include "../../include/globals.h"
#include "../../include/gui/gui_manager.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

int GUIManager::selected_light_index = 0;

void GUIManager::drawTabs() {
  int tab_w = gui_width / 7;
  int tab_h = 25;
  int tab_y = gui_y + 25;

  drawButton(gui_x, tab_y, tab_w, tab_h, "Obj", current_tab == 0);

  drawButton(gui_x + tab_w, tab_y, tab_w, tab_h, "Cam", current_tab == 1);

  drawButton(gui_x + tab_w * 2, tab_y, tab_w, tab_h, "Proj", current_tab == 2);

  drawButton(gui_x + tab_w * 3, tab_y, tab_w, tab_h, "Amb", current_tab == 3);

  drawButton(gui_x + tab_w * 4, tab_y, tab_w, tab_h, "Transf",
             current_tab == 4);

  drawButton(gui_x + tab_w * 5, tab_y, tab_w, tab_h, "Cisal", current_tab == 6);

  drawButton(gui_x + tab_w * 6, tab_y, tab_w, tab_h, "Luz", current_tab == 5);
}

void GUIManager::drawLightingTab() {
  int content_y = gui_y + 60;
  int line_height = 20;

  int padding = 10;
  int gap = 10;
  int col_w, left_x, middle_x, right_x;

  if (!is_night_mode) {

    int local_width = gui_width + 200;
    col_w = (local_width - padding * 2 - gap * 2) / 3;
    left_x = gui_x + padding;
    middle_x = gui_x + padding + col_w + gap;
    right_x = gui_x + padding + col_w * 2 + gap * 2;
  } else {

    col_w = (gui_width - padding * 2 - gap) / 2;
    left_x = gui_x + padding;
    middle_x = -1;
    right_x = gui_x + padding + col_w + gap;
  }

  int left_y = content_y;
  int right_y = content_y;

  drawText(left_x, left_y, "=== Gerenciador de Luzes ===", 0.5f, 0.8f, 1.0f);
  left_y += line_height + 8;

  drawText(left_x, left_y, "Lista de luzes:", 0.8f, 0.8f, 0.8f);
  left_y += line_height + 4;

  int btn_h = 20;
  int btn_w = col_w;

  int list_text_pad = 5;

  {
    bool is_selected = (selected_light_index == -1);
    string label = ambient.name.empty() ? "Ambient" : ambient.name;
    if (!ambient.enabled)
      label += " (OFF)";

    drawRect(left_x, left_y, btn_w, btn_h, is_selected ? 0.25f : 0.15f,
             is_selected ? 0.35f : 0.2f, is_selected ? 0.55f : 0.25f, 1.0f);
    drawText(left_x + list_text_pad, left_y + 14, label, 0.95f, 0.95f, 0.95f);

    left_y += btn_h + 2;
  }

  int current_list_x = left_x;
  for (size_t i = 0; i < lights.size(); ++i) {
    bool is_selected = (static_cast<int>(i) == selected_light_index);
    string label = lights[i]->name;
    if (label.empty())
      label = "Luz " + to_string(i);

    if (!lights[i]->enabled)
      label += " (OFF)";

    if (left_y + btn_h > gui_y + gui_height - 10) {

      if (current_list_x == left_x && middle_x != -1) {
        current_list_x = middle_x;
        left_y = content_y + line_height + 8 + line_height + 4 + btn_h + 2;
      } else {
        break;
      }
    }

    drawRect(current_list_x, left_y, btn_w, btn_h, is_selected ? 0.25f : 0.15f,
             is_selected ? 0.35f : 0.2f, is_selected ? 0.55f : 0.25f, 1.0f);
    drawText(current_list_x + list_text_pad, left_y + 14, label, 0.95f, 0.95f,
             0.95f);
    left_y += btn_h + 2;
  }

  if (selected_light_index != last_selected_light_index) {
    light_pending_loaded = false;
    light_has_pending_changes = false;
    last_selected_light_index = selected_light_index;
  }

  if (!light_pending_loaded) {
    if (selected_light_index == -1) {
      pending_light_intensity[0] = ambient.intensity.r;
      pending_light_intensity[1] = ambient.intensity.g;
      pending_light_intensity[2] = ambient.intensity.b;
      pending_light_position_buf[0] = 0;
      pending_light_position_buf[1] = 0;
      pending_light_position_buf[2] = 0;
      pending_light_reach = -1.0;
    } else if (selected_light_index >= 0 &&
               selected_light_index < (int)lights.size()) {
      auto l = lights[selected_light_index];
      pending_light_intensity[0] = l->intensity.r;
      pending_light_intensity[1] = l->intensity.g;
      pending_light_intensity[2] = l->intensity.b;

      point3 p = l->get_position();
      pending_light_position_buf[0] = p.x();
      pending_light_position_buf[1] = p.y();
      pending_light_position_buf[2] = p.z();

      pending_light_reach = l->reach;
    }

    light_pending_loaded = true;
  }

  drawText(right_x, right_y, "Propriedades:", 0.8f, 0.8f, 0.8f);
  right_y += line_height;

  auto drawStepperRow = [&](const string &label, double &val, double min_v,
                            double max_v, int precision, bool showLabel) {
    int row_h = 22;
    int btn_w_local = 22;
    int btn_h_local = 18;

    int label_w = showLabel ? 28 : 0;

    int x_label = right_x;
    int x_minus = right_x + label_w;
    int x_slider = x_minus + btn_w_local + 4;
    int slider_w = col_w - label_w - btn_w_local * 2 - 8;
    int x_plus = x_slider + slider_w + 4;

    if (showLabel) {
      drawText(x_label, right_y + 12, label + ":", 0.9f, 0.9f, 0.9f);
    }

    drawButton(x_minus, right_y, btn_w_local, btn_h_local, "-");
    drawSlider(x_slider, right_y, slider_w, (float)val, (float)min_v,
               (float)max_v, "");
    drawButton(x_plus, right_y, btn_w_local, btn_h_local, "+");

    ostringstream ss;
    ss << fixed << setprecision(precision) << val;
    drawText(right_x + col_w - 60, right_y + 12, ss.str(), 0.85f, 0.85f, 0.85f);

    right_y += row_h;
  };

  if (selected_light_index == -1) {
    string toggle_label = ambient.enabled ? "Desativar Luz" : "Ativar Luz";
    drawButton(right_x, right_y, col_w, 25, toggle_label, ambient.enabled);
    right_y += 40;

    drawText(right_x, right_y, "Intensidade (RGB):", 0.9f, 0.9f, 0.9f);
    drawText(right_x, right_y, "Intensidade (RGB):", 0.9f, 0.9f, 0.9f);
    right_y += line_height + 8;

    drawStepperRow("R", pending_light_intensity[0], 0.0, 2.0, 2, true);
    drawStepperRow("G", pending_light_intensity[1], 0.0, 2.0, 2, true);
    drawStepperRow("B", pending_light_intensity[2], 0.0, 2.0, 2, true);

    if (light_has_pending_changes) {
      drawButton(right_x, right_y + 10, col_w, 28, "APLICAR", true);
    } else {
      drawButton(right_x, right_y + 10, col_w, 28, "Aplicar");
    }
    return;
  }

  if (selected_light_index >= 0 && selected_light_index < (int)lights.size()) {
    auto l = lights[selected_light_index];

    string toggle_label = l->enabled ? "Desativar Luz" : "Ativar Luz";
    drawButton(right_x, right_y, col_w, 25, toggle_label, l->enabled);
    right_y += 40;

    drawText(right_x, right_y, "Intensidade (RGB):", 0.9f, 0.9f, 0.9f);
    drawText(right_x, right_y, "Intensidade (RGB):", 0.9f, 0.9f, 0.9f);
    right_y += line_height + 8;

    drawStepperRow("R", pending_light_intensity[0], 0.0, 2.0, 2, true);
    drawStepperRow("G", pending_light_intensity[1], 0.0, 2.0, 2, true);
    drawStepperRow("B", pending_light_intensity[2], 0.0, 2.0, 2, true);

    point3 pos(pending_light_position_buf[0], pending_light_position_buf[1],
               pending_light_position_buf[2]);

    if (abs(pos.x()) < 10000 && abs(pos.y()) < 10000 && abs(pos.z()) < 10000) {
      right_y += 10;
      drawText(right_x, right_y, "Posicao (X, Y, Z):", 0.9f, 0.9f, 0.9f);
      drawText(right_x, right_y, "Posicao (X, Y, Z):", 0.9f, 0.9f, 0.9f);
      right_y += line_height + 8;

      drawStepperRow("X", pending_light_position_buf[0], 0.0, 2000.0, 0, true);
      drawStepperRow("Y", pending_light_position_buf[1], 0.0, 1000.0, 0, true);
      drawStepperRow("Z", pending_light_position_buf[2], 0.0, 2000.0, 0, true);
    }

    if (l->supports_reach()) {
      right_y += 6;
      drawText(right_x, right_y, "Alcance/Espalhamento:", 0.9f, 0.9f, 0.9f);
      right_y += line_height;

      if (pending_light_reach < 0)
        pending_light_reach = 0;
      drawStepperRow("R", pending_light_reach, 0.0, 2000.0, 0, true);
    }

    if (light_has_pending_changes) {
      drawButton(right_x, right_y + 10, col_w, 28, "APLICAR", true);
    } else {
      drawButton(right_x, right_y + 10, col_w, 28, "Aplicar");
    }
  }
}

void GUIManager::drawEnvironmentTab() {
  int content_y = gui_y + 60;
  int line_height = 22;

  drawText(gui_x + 10, content_y, "=== Ciclo Dia / Noite ===", 0.5f, 0.8f,
           1.0f);
  content_y += line_height * 2;

  bool is_night = is_night_mode_ptr && *is_night_mode_ptr;
  string mode_str = is_night ? "DIA" : "NOITE";
  float r = is_night ? 1.0f : 0.3f;
  float g = is_night ? 1.0f : 0.3f;
  float b = is_night ? 0.3f : 1.0f;

  drawText(gui_x + 10, content_y, "Modo Atual:", 0.8f, 0.8f, 0.8f);
  drawText(gui_x + 100, content_y, mode_str, r, g, b);
  content_y += line_height * 2;

  drawButton(gui_x + 10, content_y, gui_width - 20, 30, "Mudar para NOITE",
             !is_night);
  content_y += 40;
  drawButton(gui_x + 10, content_y, gui_width - 20, 30, "Mudar para DIA",
             is_night);
}

void GUIManager::drawTransformTab() {
  int content_y = gui_y + 60;
  int line_height = 20;

  if (!selected_transform_name_ptr || selected_transform_name_ptr->empty()) {
    drawText(gui_x + 10, content_y, "Nenhum objeto", 0.8f, 0.8f, 0.8f);
    content_y += line_height;
    drawText(gui_x + 10, content_y, "transformavel selecionado.", 0.8f, 0.8f,
             0.8f);
    return;
  }

  if (!pending_values_loaded && get_transform_state) {
    if (get_transform_state(*selected_transform_name_ptr, pending_translation,
                            pending_rotation, pending_scale, pending_shear)) {
      pending_values_loaded = true;
      has_pending_changes = false;
    } else {
      drawText(gui_x + 10, content_y, "Erro ao ler transform.", 1.0f, 0.3f,
               0.3f);
      return;
    }
  }

  drawText(gui_x + 10, content_y, "Objeto: " + *selected_transform_name_ptr,
           0.5f, 0.8f, 1.0f);
  content_y += line_height + 5;

  if (has_pending_changes) {
    drawText(gui_x + 10, content_y, "* Alteracoes Pendentes *", 1.0f, 0.5f,
             0.0f);
  } else {
    drawText(gui_x + 10, content_y, "Valores Sincronizados", 0.0f, 1.0f, 0.0f);
  }
  content_y += line_height;

  float t_min[3] = {500.0f, 0.0f, 500.0f};
  float t_max[3] = {1300.0f, 500.0f, 1300.0f};

  drawText(gui_x + 10, content_y, "Translacao", 0.8f, 0.8f, 0.8f);
  content_y += line_height;

  string axes[3] = {"X:", "Y:", "Z:"};
  for (int i = 0; i < 3; i++) {

    drawText(gui_x + 10, content_y + 12, axes[i], 0.9f, 0.9f, 0.9f);

    drawButton(gui_x + 30, content_y, 22, 18, "-");

    drawButton(gui_x + 54, content_y, 22, 18, "+");

    int slider_x = gui_x + 78;
    int slider_w = gui_width - 125;
    drawSlider(slider_x, content_y, slider_w, (float)pending_translation[i],
               t_min[i], t_max[i], "");

    content_y += 25;
  }
  content_y += 10;

  drawText(gui_x + 10, content_y, "Rotacao (graus)", 0.8f, 0.8f, 0.8f);
  content_y += line_height;

  for (int i = 0; i < 3; i++) {

    drawText(gui_x + 10, content_y + 12, axes[i], 0.9f, 0.9f, 0.9f);
    drawButton(gui_x + 30, content_y, 22, 18, "-");
    drawButton(gui_x + 54, content_y, 22, 18, "+");

    int slider_x = gui_x + 78;
    int slider_w = gui_width - 125;
    drawSlider(slider_x, content_y, slider_w, (float)pending_rotation[i], 0.0f,
               360.0f, "");

    content_y += 25;
  }

  content_y += 10;

  if (has_pending_changes) {

    drawRect(gui_x + 10, content_y, gui_width - 20, 35, 0.2f, 0.8f, 0.2f, 1.0f);
    drawText(gui_x + gui_width / 2 - 30, content_y + 22, "APLICAR", 0.0f, 0.0f,
             0.0f);

    drawRect(gui_x + 10, content_y, gui_width - 20, 35, 0.2f, 0.8f, 0.2f, 0.0f);
  } else {
    drawButton(gui_x + 10, content_y, gui_width - 20, 35, "Aplicar");
  }

  content_y += 40;
  drawButton(gui_x + 10, content_y, gui_width - 20, 30, "Reset Objects");
}

void GUIManager::drawObjectTab() {
  int content_y = gui_y + 60;
  int line_height = 18;

  drawText(gui_x + 10, content_y, "Objeto:", 0.7f, 0.7f, 0.7f);
  drawText(gui_x + 80, content_y, selected_object_name, 1.0f, 0.9f, 0.3f);
  content_y += line_height;

  drawText(gui_x + 10, content_y, "Material:", 0.7f, 0.7f, 0.7f);
  drawText(gui_x + 80, content_y, selected_material_name, 0.5f, 0.8f, 1.0f);
  content_y += line_height + 5;

  ostringstream pos_stream;
  pos_stream << fixed << setprecision(1) << "(" << selected_position[0] << ", "
             << selected_position[1] << ", " << selected_position[2] << ")";
  drawText(gui_x + 10, content_y, "Posicao:", 0.7f, 0.7f, 0.7f);
  content_y += line_height;
  drawText(gui_x + 20, content_y, pos_stream.str(), 0.9f, 0.9f, 0.9f);
  content_y += line_height;

  ostringstream norm_stream;
  norm_stream << fixed << setprecision(2) << "(" << selected_normal[0] << ", "
              << selected_normal[1] << ", " << selected_normal[2] << ")";
  drawText(gui_x + 10, content_y, "Normal:", 0.7f, 0.7f, 0.7f);
  content_y += line_height;
  drawText(gui_x + 20, content_y, norm_stream.str(), 0.9f, 0.9f, 0.9f);
  content_y += line_height;

  ostringstream dist_stream;
  dist_stream << fixed << setprecision(2) << selected_distance;
  drawText(gui_x + 10, content_y, "Distancia:", 0.7f, 0.7f, 0.7f);
  drawText(gui_x + 90, content_y, dist_stream.str(), 0.9f, 0.9f, 0.9f);
  content_y += line_height + 10;

  if (selected_object_name.find("Lamina") != string::npos ||
      selected_object_name.find("lamina") != string::npos ||
      selected_material_name.find("Metal") != string::npos ||
      selected_material_name.find("Sword") != string::npos) {

    drawText(gui_x + 10, content_y, "=== Brilho da Lamina ===", 0.5f, 0.8f,
             1.0f);
    content_y += line_height;

    string shine_status =
        (blade_shine_ptr && *blade_shine_ptr) ? "LIGADO" : "DESLIGADO";
    float status_r = (blade_shine_ptr && *blade_shine_ptr) ? 0.3f : 0.8f;
    float status_g = (blade_shine_ptr && *blade_shine_ptr) ? 1.0f : 0.3f;
    float status_b = (blade_shine_ptr && *blade_shine_ptr) ? 0.3f : 0.3f;

    drawText(gui_x + 10, content_y, "Estado:", 0.7f, 0.7f, 0.7f);
    drawText(gui_x + 80, content_y, shine_status, status_r, status_g, status_b);
    content_y += line_height + 5;

    drawButton(gui_x + 10, content_y, 130, 25, "Ligar Brilho");
    drawButton(gui_x + 150, content_y, 130, 25, "Desligar Brilho");
  }
}

void GUIManager::drawCameraTab() {
  int content_y = gui_y + 60;
  int line_height = 24;
  int btn_w = 25;
  int btn_h = 20;

  int col_label = gui_x + 20;
  int col_btn_minus = gui_x + 50;
  int col_val_text = gui_x + 85;
  int col_btn_plus = gui_x + 160;

  if (!cam_eye_ptr || !cam_at_ptr) {
    drawText(gui_x + 10, content_y, "Camera nao inicializada", 1.0f, 0.3f,
             0.3f);
    return;
  }

  if (!cam_pending_loaded) {
    pending_eye[0] = cam_eye_ptr[0];
    pending_eye[1] = cam_eye_ptr[1];
    pending_eye[2] = cam_eye_ptr[2];
    pending_at[0] = cam_at_ptr[0];
    pending_at[1] = cam_at_ptr[1];
    pending_at[2] = cam_at_ptr[2];
    if (cam_up_ptr) {
      pending_up[0] = cam_up_ptr[0];
      pending_up[1] = cam_up_ptr[1];
      pending_up[2] = cam_up_ptr[2];
    }
    cam_pending_loaded = true;
    cam_has_pending_changes = false;
  }

  if (cam_has_pending_changes) {
    drawText(gui_x + 10, content_y, "* Alteracoes pendentes *", 1.0f, 0.8f,
             0.2f);
    content_y += line_height;
  } else {
    content_y += line_height;
  }

  drawText(gui_x + 10, content_y, "=== Eye (Posicao) ===", 0.5f, 0.8f, 1.0f);
  content_y += line_height;

  drawText(col_label, content_y + 5, "X:", 0.8f, 0.8f, 0.8f);
  drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
  ostringstream eye_x_ss;
  eye_x_ss << fixed << setprecision(0) << pending_eye[0];
  drawText(col_val_text, content_y + 5, eye_x_ss.str(), 1.0f, 1.0f, 1.0f);
  drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
  content_y += line_height + 2;

  drawText(col_label, content_y + 5, "Y:", 0.8f, 0.8f, 0.8f);
  drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
  ostringstream eye_y_ss;
  eye_y_ss << fixed << setprecision(0) << pending_eye[1];
  drawText(col_val_text, content_y + 5, eye_y_ss.str(), 1.0f, 1.0f, 1.0f);
  drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
  content_y += line_height + 2;

  drawText(col_label, content_y + 5, "Z:", 0.8f, 0.8f, 0.8f);
  drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
  ostringstream eye_z_ss;
  eye_z_ss << fixed << setprecision(0) << pending_eye[2];
  drawText(col_val_text, content_y + 5, eye_z_ss.str(), 1.0f, 1.0f, 1.0f);
  drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
  content_y += line_height + 20;

  drawText(gui_x + 10, content_y, "=== At (Alvo) ===", 0.5f, 0.8f, 1.0f);
  content_y += line_height;

  drawText(col_label, content_y + 5, "X:", 0.8f, 0.8f, 0.8f);
  drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
  ostringstream at_x_ss;
  at_x_ss << fixed << setprecision(0) << pending_at[0];
  drawText(col_val_text, content_y + 5, at_x_ss.str(), 1.0f, 1.0f, 1.0f);
  drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
  content_y += line_height + 2;

  drawText(col_label, content_y + 5, "Y:", 0.8f, 0.8f, 0.8f);
  drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
  ostringstream at_y_ss;
  at_y_ss << fixed << setprecision(0) << pending_at[1];
  drawText(col_val_text, content_y + 5, at_y_ss.str(), 1.0f, 1.0f, 1.0f);
  drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
  content_y += line_height + 2;

  drawText(col_label, content_y + 5, "Z:", 0.8f, 0.8f, 0.8f);
  drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
  ostringstream at_z_ss;
  at_z_ss << fixed << setprecision(0) << pending_at[2];
  drawText(col_val_text, content_y + 5, at_z_ss.str(), 1.0f, 1.0f, 1.0f);
  drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
  content_y += line_height + 20;

  drawText(gui_x + 10, content_y, "=== Up (Orientacao) ===", 0.5f, 0.8f, 1.0f);
  content_y += line_height;

  if (cam_up_ptr) {

    drawText(col_label, content_y + 5, "X:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    ostringstream up_x_ss;
    up_x_ss << fixed << setprecision(2) << pending_up[0];
    drawText(col_val_text, content_y + 5, up_x_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 2;

    drawText(col_label, content_y + 5, "Y:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    ostringstream up_y_ss;
    up_y_ss << fixed << setprecision(2) << pending_up[1];
    drawText(col_val_text, content_y + 5, up_y_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 2;

    drawText(col_label, content_y + 5, "Z:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    ostringstream up_z_ss;
    up_z_ss << fixed << setprecision(2) << pending_up[2];
    drawText(col_val_text, content_y + 5, up_z_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 8;
  } else {
    drawText(gui_x + 10, content_y, "Up nao configurado", 0.6f, 0.6f, 0.6f);
    content_y += line_height + 8;
  }

  // --- Botão Reset Camera ---
  drawButton(gui_x + 10, content_y, gui_width - 20, 28, "Reset Camera");
  content_y += 35;

  if (cam_has_pending_changes) {
    drawButton(gui_x + 10, content_y, gui_width - 20, 28, "APLICAR", true);
  } else {
    drawButton(gui_x + 10, content_y, gui_width - 20, 28, "Aplicar");
  }
}

void GUIManager::drawProjectionTab() {
  int content_y = gui_y + 60;
  int btn_h = 30;
  int btn_spacing = 35;

  drawText(gui_x + 10, content_y, "=== Tipo de Projecao ===", 0.5f, 0.8f, 1.0f);
  content_y += 25;

  int current_proj = projection_type_ptr ? *projection_type_ptr : 0;

  drawButton(gui_x + 10, content_y, gui_width - 20, btn_h, "Perspectiva",
             current_proj == 0);
  content_y += btn_spacing;

  drawButton(gui_x + 10, content_y, gui_width - 20, btn_h, "Ortografica",
             current_proj == 1);
  content_y += btn_spacing;

  drawButton(gui_x + 10, content_y, gui_width - 20, btn_h, "Obliqua",
             current_proj == 2);
  content_y += btn_spacing + 15;

  if (current_proj == 0) {
    drawText(gui_x + 10, content_y, "=== Pontos de Fuga ===", 0.5f, 0.8f, 1.0f);
    content_y += 25;

    int vp_preset =
        vanishing_points_preset_ptr ? *vanishing_points_preset_ptr : 0;

    int btn_w = (gui_width - 30) / 2;

    drawButton(gui_x + 10, content_y, btn_w, 28, "Livre", vp_preset == 0);
    drawButton(gui_x + 15 + btn_w, content_y, btn_w, 28, "1 Ponto",
               vp_preset == 1);
    content_y += 33;

    drawButton(gui_x + 10, content_y, btn_w, 28, "2 Pontos", vp_preset == 2);
    drawButton(gui_x + 15 + btn_w, content_y, btn_w, 28, "3 Pontos",
               vp_preset == 3);
    content_y += 40;

    drawText(gui_x + 10, content_y, "Modo Ativo:", 0.7f, 0.7f, 0.7f);
    content_y += 18;

    switch (vp_preset) {
    case 0:
      drawText(gui_x + 10, content_y, "LIVRE - Camera manual", 1.0f, 1.0f,
               0.5f);
      content_y += 18;
      drawText(gui_x + 10, content_y, "Use a aba Camera para", 0.6f, 0.6f,
               0.6f);
      content_y += 15;
      drawText(gui_x + 10, content_y, "posicionar livremente.", 0.6f, 0.6f,
               0.6f);
      break;
    case 1:
      drawText(gui_x + 10, content_y, "1 PONTO DE FUGA", 0.3f, 1.0f, 0.3f);
      content_y += 18;
      drawText(gui_x + 10, content_y, "Camera frontal.", 0.6f, 0.6f, 0.6f);
      content_y += 15;
      drawText(gui_x + 10, content_y, "Apenas eixo Z converge.", 0.6f, 0.6f,
               0.6f);
      break;
    case 2:
      drawText(gui_x + 10, content_y, "2 PONTOS DE FUGA", 0.3f, 0.8f, 1.0f);
      content_y += 18;
      drawText(gui_x + 10, content_y, "Camera em diagonal.", 0.6f, 0.6f, 0.6f);
      content_y += 15;
      drawText(gui_x + 10, content_y, "Eixos X e Z convergem.", 0.6f, 0.6f,
               0.6f);
      break;
    case 3:
      drawText(gui_x + 10, content_y, "3 PONTOS DE FUGA", 1.0f, 0.5f, 1.0f);
      content_y += 18;
      drawText(gui_x + 10, content_y, "Camera elevada + diagonal.", 0.6f, 0.6f,
               0.6f);
      content_y += 15;
      drawText(gui_x + 10, content_y, "Todos eixos convergem.", 0.6f, 0.6f,
               0.6f);
      break;
    }
  } else {
    drawText(gui_x + 10, content_y, "Pontos de fuga:", 0.5f, 0.5f, 0.5f);
    content_y += 18;
    drawText(gui_x + 10, content_y, "(Apenas em Perspectiva)", 0.4f, 0.4f,
             0.4f);
  }
}

void GUIManager::draw() {
  if (!gui_visible)
    return;

  int bg_width = gui_width;
  if (current_tab == 5 && !is_night_mode) {
    bg_width += 200;
  }

  drawRect(gui_x, gui_y, bg_width, gui_height, 0.1f, 0.1f, 0.15f, 0.92f);
  drawRect(gui_x, gui_y, bg_width, 25, 0.2f, 0.3f, 0.5f, 1.0f);

  drawText(gui_x + 10, gui_y + 17, "Painel de Controle", 1.0f, 1.0f, 1.0f);
  drawRect(gui_x + bg_width - 25, gui_y + 2, 21, 21, 0.6f, 0.2f, 0.2f, 1.0f);
  drawText(gui_x + bg_width - 19, gui_y + 17, "X", 1.0f, 1.0f, 1.0f);

  drawTabs();

  switch (current_tab) {
  case 0:
    drawObjectTab();
    break;
  case 1:
    drawCameraTab();
    break;
  case 2:
    drawProjectionTab();
    break;
  case 3:
    drawEnvironmentTab();
    break;
  case 4:
    drawTransformTab();
    break;
  case 5:
    drawLightingTab();
    break;
  case 6:
    GUIManager::drawShearTab();
    break;
  }
}

void GUIManager::drawShearTab() {
  int content_y = gui_y + 60;
  int line_height = 20;

  if (!selected_transform_name_ptr || selected_transform_name_ptr->empty()) {
    drawText(gui_x + 10, content_y, "Nenhum objeto", 0.8f, 0.8f, 0.8f);
    content_y += line_height;
    drawText(gui_x + 10, content_y, "transformavel selecionado.", 0.8f, 0.8f,
             0.8f);
    return;
  }

  if (!pending_values_loaded && get_transform_state) {
    if (get_transform_state(*selected_transform_name_ptr, pending_translation,
                            pending_rotation, pending_scale, pending_shear)) {
      pending_values_loaded = true;
      has_pending_changes = false;
    } else {
      drawText(gui_x + 10, content_y, "Erro ao ler transform.", 1.0f, 0.3f,
               0.3f);
      return;
    }
  }

  drawText(gui_x + 10, content_y, "Objeto: " + *selected_transform_name_ptr,
           0.5f, 0.8f, 1.0f);
  content_y += line_height + 5;

  if (has_pending_changes) {
    drawText(gui_x + 10, content_y, "* Alteracoes Pendentes *", 1.0f, 0.5f,
             0.0f);
  } else {
    drawText(gui_x + 10, content_y, "Valores Sincronizados", 0.0f, 1.0f, 0.0f);
  }
  content_y += line_height;

  auto drawStepper = [&](int x, int y, double &val, double min_v, double max_v,
                         const string &label) {
    drawText(x, y + 12, label, 0.9f, 0.9f, 0.9f);
    drawButton(x + 25, y, 22, 18, "-");
    drawSlider(x + 50, y, 100, (float)val, (float)min_v, (float)max_v, "");
    drawButton(x + 155, y, 22, 18, "+");

    ostringstream ss;
    ss << fixed << setprecision(2) << val;
    drawText(x + 190, y + 12, ss.str(), 0.85f, 0.85f, 0.85f);
  };

  string labels[6] = {"XY", "XZ", "YX", "YZ", "ZX", "ZY"};
  for (int i = 0; i < 6; i++) {
    drawStepper(gui_x + 10, content_y, pending_shear[i], -5.0, 5.0, labels[i]);
    content_y += 25;
  }

  content_y += 10;

  if (has_pending_changes) {
    drawRect(gui_x + 10, content_y, gui_width - 20, 35, 0.2f, 0.8f, 0.2f, 1.0f);
    drawText(gui_x + gui_width / 2 - 30, content_y + 22, "APLICAR", 0.0f, 0.0f,
             0.0f);
    drawRect(gui_x + 10, content_y, gui_width - 20, 35, 0.2f, 0.8f, 0.2f, 0.0f);
  } else {
    drawButton(gui_x + 10, content_y, gui_width - 20, 35, "Aplicar");
  }

  content_y += 40;
  drawButton(gui_x + 10, content_y, gui_width - 20, 30, "Reset Objects");
}
