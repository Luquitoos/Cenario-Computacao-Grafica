#include "../../include/globals.h"
#include "../../include/gui/gui_manager.h"
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

int GUIManager::selected_light_index = 0;

void GUIManager::drawTabs() {
  int tab_w = gui_width / 6;
  int tab_h = 25;
  int tab_y = gui_y + 25;

  drawButton(gui_x, tab_y, tab_w, tab_h, "Obj", current_tab == 0);

  drawButton(gui_x + tab_w, tab_y, tab_w, tab_h, "Cam", current_tab == 1);

  drawButton(gui_x + tab_w * 2, tab_y, tab_w, tab_h, "Proj", current_tab == 2);

  drawButton(gui_x + tab_w * 3, tab_y, tab_w, tab_h, "Amb", current_tab == 3);

  drawButton(gui_x + tab_w * 4, tab_y, tab_w, tab_h, "Transf",
             current_tab == 4);

  drawButton(gui_x + tab_w * 5, tab_y, tab_w, tab_h, "Luz", current_tab == 5);
}

void GUIManager::drawLightingTab() {
  int content_y = gui_y + 60;
  int line_height = 20;

  drawText(gui_x + 10, content_y, "=== Gerenciador de Luzes ===", 0.5f, 0.8f,
           1.0f);
  content_y += line_height + 5;

  drawText(gui_x + 10, content_y, "Selecione uma luz:", 0.8f, 0.8f, 0.8f);
  content_y += line_height;

  int btn_h = 20;
  int lights_per_row = 1;
  int btn_w = (gui_width - 20) / lights_per_row;

  for (size_t i = 0; i < lights.size(); ++i) {
    bool is_selected = (static_cast<int>(i) == selected_light_index);
    string label = lights[i]->name;
    if (label.empty())
      label = "Luz " + to_string(i);

    if (!lights[i]->enabled)
      label += " (OFF)";

    if (content_y + btn_h > gui_y + gui_height - 200)
      break;

    drawButton(gui_x + 10, content_y, btn_w, btn_h, label, is_selected);
    content_y += btn_h + 2;
  }

  content_y += 10;
  if (selected_light_index >= 0 && selected_light_index < (int)lights.size()) {
    auto l = lights[selected_light_index];

    string toggle_label = l->enabled ? "Desativar Luz" : "Ativar Luz";
    drawButton(gui_x + 10, content_y, gui_width - 20, 25, toggle_label,
               l->enabled);
    content_y += 30;

    drawText(gui_x + 10, content_y, "Intensidade (RGB):", 0.9f, 0.9f, 0.9f);
    content_y += line_height;

    stringstream ss_int;
    ss_int << fixed << setprecision(2) << l->intensity.r << " "
           << l->intensity.g << " " << l->intensity.b;
    drawText(gui_x + 10, content_y, ss_int.str(), l->intensity.r,
             l->intensity.g, l->intensity.b);

    drawSlider(gui_x + 10, content_y + 5, 150, l->intensity.r, 0.0f, 2.0f, "R");
    content_y += 20;
    // Green
    drawSlider(gui_x + 10, content_y + 5, 150, l->intensity.g, 0.0f, 2.0f, "G");
    content_y += 20;
    // Blue
    drawSlider(gui_x + 10, content_y + 5, 150, l->intensity.b, 0.0f, 2.0f, "B");
    content_y += 25;

    point3 pos = l->get_position();

    if (abs(pos.x()) < 10000 && abs(pos.y()) < 10000 && abs(pos.z()) < 10000) {
      drawText(gui_x + 10, content_y, "Posicao (X, Y, Z):", 0.9f, 0.9f, 0.9f);
      content_y += line_height;

      drawSlider(gui_x + 10, content_y + 5, 150, pos.x(), 0.0f, 2000.0f, "X");
      content_y += 20;

      drawSlider(gui_x + 10, content_y + 5, 150, pos.y(), 0.0f, 1000.0f, "Y");
      content_y += 20;

      drawSlider(gui_x + 10, content_y + 5, 150, pos.z(), 0.0f, 2000.0f, "Z");
      content_y += 25;
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
                            pending_rotation, pending_scale)) {
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

    // Sliders for Translation (using drawSlider helper logic visualized)
    // Assuming drawSlider exists or we use buttons for now as in Transf tab
    // The visual shows generic sliders.
    // Re-use logic or just rely on existing Transf tab structure
    // Wait, Transf tab uses buttons + and -
    // The user requested a tab where they can "change place" and "increase
    // intensity with a bar" So "bar" implies slider. I need to ensure
    // drawSlider implementation exists or logic for it. I see `drawSlider`
    // declared in header. I will assume it's implemented (I saw it declared in
    // step 997 but not implemented in file view in step 1001? Wait. Step 1001
    // showed lines 1-100. Step 1002 lines 400-454. I missed checking if
    // drawSlider IS implemented. If it's not implemented, I need to implement
    // it. I'll add `drawSlider` implementation if missing.
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

  drawRect(gui_x, gui_y, gui_width, gui_height, 0.1f, 0.1f, 0.15f, 0.92f);
  drawRect(gui_x, gui_y, gui_width, 25, 0.2f, 0.3f, 0.5f, 1.0f);

  drawText(gui_x + 10, gui_y + 17, "Painel de Controle", 1.0f, 1.0f, 1.0f);
  drawRect(gui_x + gui_width - 25, gui_y + 2, 21, 21, 0.6f, 0.2f, 0.2f, 1.0f);
  drawText(gui_x + gui_width - 19, gui_y + 17, "X", 1.0f, 1.0f, 1.0f);

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
  }
}
