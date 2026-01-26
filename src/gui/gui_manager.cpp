#include "../../include/gui/gui_manager.h"
#include <sstream>
#include <iomanip>
#include <iostream>

// ===== Inicialização das variáveis estáticas =====
bool GUIManager::gui_visible = false;
int GUIManager::gui_x = 10;
int GUIManager::gui_y = 10;
int GUIManager::gui_width = 300;
int GUIManager::gui_height = 320;
int GUIManager::current_tab = 0;

// Objeto selecionado
std::string GUIManager::selected_object_name = "";
std::string GUIManager::selected_material_name = "";
double GUIManager::selected_position[3] = {0, 0, 0};
double GUIManager::selected_normal[3] = {0, 0, 0};
double GUIManager::selected_distance = 0;

// Ponteiros para câmera
double* GUIManager::cam_eye_ptr = nullptr;
double* GUIManager::cam_at_ptr = nullptr;
double* GUIManager::cam_up_ptr = nullptr;
int* GUIManager::projection_type_ptr = nullptr;
bool* GUIManager::need_redraw_ptr = nullptr;

// Ponteiro para estado do brilho
bool* GUIManager::blade_shine_ptr = nullptr;

// Ponteiro para estado do ambiente
bool* GUIManager::is_night_mode_ptr = nullptr;

// Ponteiro para nome do objeto transformável
std::string* GUIManager::selected_transform_name_ptr = nullptr;

// Variáveis para Draft Mode
double GUIManager::pending_translation[3] = {0,0,0};
double GUIManager::pending_rotation[3] = {0,0,0};
double GUIManager::pending_scale[3] = {1,1,1};
bool GUIManager::has_pending_changes = false;
bool GUIManager::pending_values_loaded = false;

// Draft Mode para Câmera
double GUIManager::pending_eye[3] = {0,0,0};
double GUIManager::pending_at[3] = {0,0,0};
double GUIManager::pending_up[3] = {0,1,0};
bool GUIManager::cam_pending_loaded = false;
bool GUIManager::cam_has_pending_changes = false;

// Callbacks
std::function<void()> GUIManager::on_camera_change = nullptr;
std::function<void()> GUIManager::on_render_request = nullptr;
std::function<void(bool)> GUIManager::on_blade_shine_toggle = nullptr;
std::function<void(bool)> GUIManager::on_day_night_toggle = nullptr;
std::function<bool(const std::string&, double*, double*, double*)> GUIManager::get_transform_state = nullptr;
std::function<void(const std::string&, const double*, const double*, const double*)> GUIManager::set_transform_state = nullptr;

void GUIManager::init(double* eye, double* at, double* up, int* proj_type, bool* redraw, bool* blade_shine, bool* is_night, std::string* sel_trans_name) {
    gui_visible = false;
    gui_x = 10;
    gui_y = 10;
    current_tab = 0;
    cam_eye_ptr = eye;
    cam_at_ptr = at;
    cam_up_ptr = up;
    projection_type_ptr = proj_type;
    need_redraw_ptr = redraw;
    blade_shine_ptr = blade_shine;
    is_night_mode_ptr = is_night;
    selected_transform_name_ptr = sel_trans_name;
    
    // Resetar flags de draft (Transform)
    has_pending_changes = false;
    pending_values_loaded = false;
    
    // Resetar flags de draft (Câmera)
    cam_pending_loaded = false;
    cam_has_pending_changes = false;
}

void GUIManager::setCallbacks(
    std::function<void()> cam_change,
    std::function<void()> render_req,
    std::function<void(bool)> blade_toggle,
    std::function<void(bool)> day_night_toggle,
    std::function<bool(const std::string&, double*, double*, double*)> get_trans,
    std::function<void(const std::string&, const double*, const double*, const double*)> set_trans
) {
    on_camera_change = cam_change;
    on_render_request = render_req;
    on_blade_shine_toggle = blade_toggle;
    on_day_night_toggle = day_night_toggle;
    get_transform_state = get_trans;
    set_transform_state = set_trans;
}

void GUIManager::show(const std::string& object_name,
                      const std::string& material_name,
                      double px, double py, double pz,
                      double nx, double ny, double nz,
                      double distance) {
    selected_object_name = object_name;
    selected_material_name = material_name;
    selected_position[0] = px;
    selected_position[1] = py;
    selected_position[2] = pz;
    selected_normal[0] = nx;
    selected_normal[1] = ny;
    selected_normal[2] = nz;
    selected_distance = distance;
    gui_visible = true;
    selected_distance = distance;
    gui_visible = true;
    current_tab = 0; // Voltar para aba de objeto
    
    // Resetar flags de draft ao selecionar novo objeto
    has_pending_changes = false;
    pending_values_loaded = false;
}

void GUIManager::hide() {
    gui_visible = false;
}

void GUIManager::toggle() {
    gui_visible = !gui_visible;
}

// ===== DESENHO DE COMPONENTES =====

void GUIManager::drawRect(int x, int y, int w, int h, float r, float g, float b, float a) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluOrtho2D(0, viewport[2], viewport[3], 0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    // Retângulo preenchido
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
    
    // Borda
    glColor4f(0.6f, 0.6f, 0.6f, 1.0f);
    glLineWidth(1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}

void GUIManager::drawText(int x, int y, const std::string& text, float r, float g, float b) {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluOrtho2D(0, viewport[2], viewport[3], 0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    glColor3f(r, g, b);
    glRasterPos2i(x, y);
    
    for (char c : text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopAttrib();
}

void GUIManager::drawButton(int x, int y, int w, int h, const std::string& label, bool active) {
    if (active) {
        drawRect(x, y, w, h, 0.3f, 0.5f, 0.7f, 0.95f);
    } else {
        drawRect(x, y, w, h, 0.25f, 0.25f, 0.3f, 0.9f);
    }
    
    int text_x = x + (w - (int)label.length() * 7) / 2;
    int text_y = y + h / 2 + 4;
    drawText(text_x, text_y, label, 1.0f, 1.0f, 1.0f);
}

void GUIManager::drawSlider(int x, int y, int w, float value, float min_val, float max_val, const std::string& label) {
    // Label pad
    int pad_x = 0;
    if (!label.empty()) {
        drawText(x, y + 12, label, 0.8f, 0.8f, 0.8f);
        pad_x = 80;
    }
    
    // Fundo do slider
    int slider_x = x + pad_x;
    int slider_w = w - pad_x;
    drawRect(slider_x, y, slider_w, 16, 0.2f, 0.2f, 0.25f, 1.0f);
    
    // Valor normalizado
    float normalized = (value - min_val) / (max_val - min_val);
    if (normalized < 0.0f) normalized = 0.0f;
    if (normalized > 1.0f) normalized = 1.0f;
    
    int fill_w = (int)(normalized * slider_w);
    drawRect(slider_x, y, fill_w, 16, 0.4f, 0.6f, 0.8f, 1.0f);
    
    // Valor numérico
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(0) << value;
    drawText(slider_x + slider_w + 5, y + 12, ss.str(), 0.9f, 0.9f, 0.9f);
}

// ===== DESENHO DAS ABAS =====

void GUIManager::drawTabs() {
    int tab_w = gui_width / 5;
    int tab_h = 25;
    int tab_y = gui_y + 25;
    
    // Aba Objeto
    drawButton(gui_x, tab_y, tab_w, tab_h, "Obj", current_tab == 0);
    // Aba Camera
    drawButton(gui_x + tab_w, tab_y, tab_w, tab_h, "Cam", current_tab == 1);
    // Aba Projecao
    drawButton(gui_x + tab_w * 2, tab_y, tab_w, tab_h, "Proj", current_tab == 2);
    // Aba Ambiente
    drawButton(gui_x + tab_w * 3, tab_y, tab_w, tab_h, "Amb", current_tab == 3);
    // Aba Transform
    drawButton(gui_x + tab_w * 4, tab_y, tab_w, tab_h, "Transf", current_tab == 4);
}

void GUIManager::drawEnvironmentTab() {
    int content_y = gui_y + 60;
    int line_height = 22;
    
    drawText(gui_x + 10, content_y, "=== Ciclo Dia / Noite ===", 0.5f, 0.8f, 1.0f);
    content_y += line_height * 2;
    
    // Estado atual
    bool is_night = is_night_mode_ptr && *is_night_mode_ptr;
    std::string mode_str = is_night ? "DIA" : "NOITE";
    float r = is_night ? 1.0f : 0.3f;
    float g = is_night ? 1.0f : 0.3f;
    float b = is_night ? 0.3f : 1.0f;
    
    drawText(gui_x + 10, content_y, "Modo Atual:", 0.8f, 0.8f, 0.8f);
    drawText(gui_x + 100, content_y, mode_str, r, g, b);
    content_y += line_height * 2;
    
    // Botões
    drawButton(gui_x + 10, content_y, gui_width - 20, 30, "Mudar para NOITE", !is_night);
    content_y += 40;
    drawButton(gui_x + 10, content_y, gui_width - 20, 30, "Mudar para DIA", is_night);
}

void GUIManager::drawTransformTab() {
    int content_y = gui_y + 60;
    int line_height = 20;
    
    if (!selected_transform_name_ptr || selected_transform_name_ptr->empty()) {
        drawText(gui_x + 10, content_y, "Nenhum objeto", 0.8f, 0.8f, 0.8f);
        content_y += line_height;
        drawText(gui_x + 10, content_y, "transformavel selecionado.", 0.8f, 0.8f, 0.8f);
        return;
    }
    
    // Se ainda não carregamos valores para o draft, carregar agora
    if (!pending_values_loaded && get_transform_state) {
        if (get_transform_state(*selected_transform_name_ptr, pending_translation, pending_rotation, pending_scale)) {
            pending_values_loaded = true;
            has_pending_changes = false;
        } else {
             drawText(gui_x + 10, content_y, "Erro ao ler transform.", 1.0f, 0.3f, 0.3f);
             return;
        }
    }
    
    drawText(gui_x + 10, content_y, "Objeto: " + *selected_transform_name_ptr, 0.5f, 0.8f, 1.0f);
    content_y += line_height + 5;
    
    // Indicador de Alterações Pendentes
    if (has_pending_changes) {
         drawText(gui_x + 10, content_y, "* Alteracoes Pendentes *", 1.0f, 0.5f, 0.0f);
    } else {
         drawText(gui_x + 10, content_y, "Valores Sincronizados", 0.0f, 1.0f, 0.0f);
    }
    content_y += line_height;
    
    // Translação
    // Range seguro para cada eixo
    float t_min[3] = {500.0f, 0.0f, 500.0f};
    float t_max[3] = {1300.0f, 500.0f, 1300.0f};
    
    drawText(gui_x + 10, content_y, "Translacao", 0.8f, 0.8f, 0.8f);
    content_y += line_height;
    
    // Layout: X: [-] [+] [===slider===]
    // Posições: Label(10), Btn-(35), Btn+(60), Slider(90 até fim)
    std::string axes[3] = {"X:", "Y:", "Z:"};
    for(int i=0; i<3; i++) {
        // Label
        drawText(gui_x + 10, content_y + 12, axes[i], 0.9f, 0.9f, 0.9f);
        
        // Botão - (x:30)
        drawButton(gui_x + 30, content_y, 22, 18, "-");
        
        // Botão + (x:54)
        drawButton(gui_x + 54, content_y, 22, 18, "+");
        
        // Slider (do x=78 até o fim)
        int slider_x = gui_x + 78;
        int slider_w = gui_width - 88;
        drawSlider(slider_x, content_y, slider_w, (float)pending_translation[i], t_min[i], t_max[i], "");
        
        content_y += 25;
    }
    
    content_y += 10;
    
    // Rotação
    drawText(gui_x + 10, content_y, "Rotacao (graus)", 0.8f, 0.8f, 0.8f);
    content_y += line_height;
    
    for(int i=0; i<3; i++) {
        // Label
        drawText(gui_x + 10, content_y + 12, axes[i], 0.9f, 0.9f, 0.9f);
        
        // Botão - (x:30)
        drawButton(gui_x + 30, content_y, 22, 18, "-");
        
        // Botão + (x:54)
        drawButton(gui_x + 54, content_y, 22, 18, "+");
        
        // Slider (x:78)
        int slider_x = gui_x + 78;
        int slider_w = gui_width - 88;
        drawSlider(slider_x, content_y, slider_w, (float)pending_rotation[i], 0.0f, 360.0f, "");
        
        content_y += 25;
    }
    
    // Botão APLICAR
    content_y += 10;
    // Destaque se houver mudanças
    if (has_pending_changes) {
        // Botão Verde Brilhante
        drawRect(gui_x + 10, content_y, gui_width - 20, 35, 0.2f, 0.8f, 0.2f, 1.0f);
        drawText(gui_x + gui_width/2 - 30, content_y + 22, "APLICAR", 0.0f, 0.0f, 0.0f);
        // Borda
        drawRect(gui_x + 10, content_y, gui_width - 20, 35, 0.2f, 0.8f, 0.2f, 0.0f); // Hack para borda se drawRect suportasse (apenas fill)
    } else {
        drawButton(gui_x + 10, content_y, gui_width - 20, 35, "Aplicar");
    }
}

void GUIManager::drawObjectTab() {
    int content_y = gui_y + 60;
    int line_height = 18;
    
    // Nome do objeto
    drawText(gui_x + 10, content_y, "Objeto:", 0.7f, 0.7f, 0.7f);
    drawText(gui_x + 80, content_y, selected_object_name, 1.0f, 0.9f, 0.3f);
    content_y += line_height;
    
    // Material
    drawText(gui_x + 10, content_y, "Material:", 0.7f, 0.7f, 0.7f);
    drawText(gui_x + 80, content_y, selected_material_name, 0.5f, 0.8f, 1.0f);
    content_y += line_height + 5;
    
    // Posição
    std::ostringstream pos_stream;
    pos_stream << std::fixed << std::setprecision(1) 
               << "(" << selected_position[0] << ", " 
               << selected_position[1] << ", " 
               << selected_position[2] << ")";
    drawText(gui_x + 10, content_y, "Posicao:", 0.7f, 0.7f, 0.7f);
    content_y += line_height;
    drawText(gui_x + 20, content_y, pos_stream.str(), 0.9f, 0.9f, 0.9f);
    content_y += line_height;
    
    // Normal
    std::ostringstream norm_stream;
    norm_stream << std::fixed << std::setprecision(2) 
                << "(" << selected_normal[0] << ", " 
                << selected_normal[1] << ", " 
                << selected_normal[2] << ")";
    drawText(gui_x + 10, content_y, "Normal:", 0.7f, 0.7f, 0.7f);
    content_y += line_height;
    drawText(gui_x + 20, content_y, norm_stream.str(), 0.9f, 0.9f, 0.9f);
    content_y += line_height;
    
    // Distância
    std::ostringstream dist_stream;
    dist_stream << std::fixed << std::setprecision(2) << selected_distance;
    drawText(gui_x + 10, content_y, "Distancia:", 0.7f, 0.7f, 0.7f);
    drawText(gui_x + 90, content_y, dist_stream.str(), 0.9f, 0.9f, 0.9f);
    content_y += line_height + 10;
    
    // Controles de brilho (se for Lâmina da Espada)
    if (selected_object_name.find("Lamina") != std::string::npos ||
        selected_object_name.find("lamina") != std::string::npos ||
        selected_material_name.find("Metal") != std::string::npos ||
        selected_material_name.find("Sword") != std::string::npos) {
        
        // Mostrar estado atual do brilho
        drawText(gui_x + 10, content_y, "=== Brilho da Lamina ===", 0.5f, 0.8f, 1.0f);
        content_y += line_height;
        
        // Estado: LIGADO ou DESLIGADO
        std::string shine_status = (blade_shine_ptr && *blade_shine_ptr) ? "LIGADO" : "DESLIGADO";
        float status_r = (blade_shine_ptr && *blade_shine_ptr) ? 0.3f : 0.8f;
        float status_g = (blade_shine_ptr && *blade_shine_ptr) ? 1.0f : 0.3f;
        float status_b = (blade_shine_ptr && *blade_shine_ptr) ? 0.3f : 0.3f;
        
        drawText(gui_x + 10, content_y, "Estado:", 0.7f, 0.7f, 0.7f);
        drawText(gui_x + 80, content_y, shine_status, status_r, status_g, status_b);
        content_y += line_height + 5;
        
        // Botões
        drawButton(gui_x + 10, content_y, 130, 25, "Ligar Brilho");
        drawButton(gui_x + 150, content_y, 130, 25, "Desligar Brilho");
    }
}

void GUIManager::drawCameraTab() {
    int content_y = gui_y + 60;
    int line_height = 24;      // Aumentado um pouco
    int btn_w = 25;            // Largura botão
    int btn_h = 20;            // Altura botão
    
    // Colunas (X fixo)
    int col_label = gui_x + 20;
    int col_btn_minus = gui_x + 50;
    int col_val_text = gui_x + 85; 
    int col_btn_plus = gui_x + 160;
    
    if (!cam_eye_ptr || !cam_at_ptr) {
        drawText(gui_x + 10, content_y, "Camera nao inicializada", 1.0f, 0.3f, 0.3f);
        return;
    }
    
    // Carregar valores reais nos pendentes na primeira vez
    if (!cam_pending_loaded) {
        pending_eye[0] = cam_eye_ptr[0]; pending_eye[1] = cam_eye_ptr[1]; pending_eye[2] = cam_eye_ptr[2];
        pending_at[0] = cam_at_ptr[0]; pending_at[1] = cam_at_ptr[1]; pending_at[2] = cam_at_ptr[2];
        if (cam_up_ptr) {
            pending_up[0] = cam_up_ptr[0]; pending_up[1] = cam_up_ptr[1]; pending_up[2] = cam_up_ptr[2];
        }
        cam_pending_loaded = true;
        cam_has_pending_changes = false;
    }
    
    // Indicador de alterações pendentes
    if (cam_has_pending_changes) {
        drawText(gui_x + 10, content_y, "* Alteracoes pendentes *", 1.0f, 0.8f, 0.2f);
        content_y += line_height;
    } else {
        content_y += line_height; // Espaço reservado
    }
    
    // ===== EYE POINT =====
    drawText(gui_x + 10, content_y, "=== Eye (Posicao) ===", 0.5f, 0.8f, 1.0f);
    content_y += line_height;
    
    // Eye X
    drawText(col_label, content_y + 5, "X:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    std::ostringstream eye_x_ss; eye_x_ss << std::fixed << std::setprecision(0) << pending_eye[0];
    drawText(col_val_text, content_y + 5, eye_x_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 2;
    
    // Eye Y
    drawText(col_label, content_y + 5, "Y:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    std::ostringstream eye_y_ss; eye_y_ss << std::fixed << std::setprecision(0) << pending_eye[1];
    drawText(col_val_text, content_y + 5, eye_y_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 2;
    
    // Eye Z
    drawText(col_label, content_y + 5, "Z:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    std::ostringstream eye_z_ss; eye_z_ss << std::fixed << std::setprecision(0) << pending_eye[2];
    drawText(col_val_text, content_y + 5, eye_z_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 20; // Espaçamento aumentado (8 -> 20)
    
    // ===== AT POINT =====
    drawText(gui_x + 10, content_y, "=== At (Alvo) ===", 0.5f, 0.8f, 1.0f);
    content_y += line_height;
    
    // At X
    drawText(col_label, content_y + 5, "X:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    std::ostringstream at_x_ss; at_x_ss << std::fixed << std::setprecision(0) << pending_at[0];
    drawText(col_val_text, content_y + 5, at_x_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 2;
    
    // At Y
    drawText(col_label, content_y + 5, "Y:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    std::ostringstream at_y_ss; at_y_ss << std::fixed << std::setprecision(0) << pending_at[1];
    drawText(col_val_text, content_y + 5, at_y_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 2;
    
    // At Z
    drawText(col_label, content_y + 5, "Z:", 0.8f, 0.8f, 0.8f);
    drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
    std::ostringstream at_z_ss; at_z_ss << std::fixed << std::setprecision(0) << pending_at[2];
    drawText(col_val_text, content_y + 5, at_z_ss.str(), 1.0f, 1.0f, 1.0f);
    drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
    content_y += line_height + 20; // Espaçamento aumentado (8 -> 20)
    
    // ===== UP VECTOR =====
    drawText(gui_x + 10, content_y, "=== Up (Orientacao) ===", 0.5f, 0.8f, 1.0f);
    content_y += line_height;
    
    if (cam_up_ptr) {
        // Up X
        drawText(col_label, content_y + 5, "X:", 0.8f, 0.8f, 0.8f);
        drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
        std::ostringstream up_x_ss; up_x_ss << std::fixed << std::setprecision(2) << pending_up[0];
        drawText(col_val_text, content_y + 5, up_x_ss.str(), 1.0f, 1.0f, 1.0f);
        drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
        content_y += line_height + 2;
        
        // Up Y
        drawText(col_label, content_y + 5, "Y:", 0.8f, 0.8f, 0.8f);
        drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
        std::ostringstream up_y_ss; up_y_ss << std::fixed << std::setprecision(2) << pending_up[1];
        drawText(col_val_text, content_y + 5, up_y_ss.str(), 1.0f, 1.0f, 1.0f);
        drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
        content_y += line_height + 2;
        
        // Up Z
        drawText(col_label, content_y + 5, "Z:", 0.8f, 0.8f, 0.8f);
        drawButton(col_btn_minus, content_y, btn_w, btn_h, "-");
        std::ostringstream up_z_ss; up_z_ss << std::fixed << std::setprecision(2) << pending_up[2];
        drawText(col_val_text, content_y + 5, up_z_ss.str(), 1.0f, 1.0f, 1.0f);
        drawButton(col_btn_plus, content_y, btn_w, btn_h, "+");
        content_y += line_height + 8;
    } else {
        drawText(gui_x + 10, content_y, "Up nao configurado", 0.6f, 0.6f, 0.6f);
        content_y += line_height + 8;
    }
    
    // Botão Aplicar e Re-renderizar (só ativo se há mudanças)
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
    
    drawText(gui_x + 10, content_y, "Selecione a projecao:", 0.8f, 0.8f, 0.8f);
    content_y += 25;
    
    int current_proj = projection_type_ptr ? *projection_type_ptr : 0;
    
    // Perspectiva
    drawButton(gui_x + 10, content_y, gui_width - 20, btn_h, "Perspectiva", current_proj == 0);
    content_y += btn_spacing;
    
    // Ortográfica
    drawButton(gui_x + 10, content_y, gui_width - 20, btn_h, "Ortografica", current_proj == 1);
    content_y += btn_spacing;
    
    // Oblíqua
    drawButton(gui_x + 10, content_y, gui_width - 20, btn_h, "Obliqua", current_proj == 2);
    content_y += btn_spacing + 20;
    
    // Instruções
    drawText(gui_x + 10, content_y, "Teclas: 1=Persp, O=Orto, P=Obliq", 0.6f, 0.6f, 0.6f);
}

void GUIManager::draw() {
    if (!gui_visible) return;
    
    // Fundo da janela
    drawRect(gui_x, gui_y, gui_width, gui_height, 0.1f, 0.1f, 0.15f, 0.92f);
    
    // Barra de título
    drawRect(gui_x, gui_y, gui_width, 25, 0.2f, 0.3f, 0.5f, 1.0f);
    drawText(gui_x + 10, gui_y + 17, "Painel de Controle", 1.0f, 1.0f, 1.0f);
    
    // Botão fechar (X)
    drawRect(gui_x + gui_width - 25, gui_y + 2, 21, 21, 0.6f, 0.2f, 0.2f, 1.0f);
    drawText(gui_x + gui_width - 19, gui_y + 17, "X", 1.0f, 1.0f, 1.0f);
    
    // Abas
    drawTabs();
    
    // Conteúdo da aba atual
    switch (current_tab) {
        case 0: drawObjectTab(); break;
        case 1: drawCameraTab(); break;
        case 2: drawProjectionTab(); break;
        case 3: drawEnvironmentTab(); break;
        case 4: drawTransformTab(); break;
    }
}

bool GUIManager::isMouseOver(int mouse_x, int mouse_y) {
    if (!gui_visible) return false;
    return (mouse_x >= gui_x && mouse_x <= gui_x + gui_width &&
            mouse_y >= gui_y && mouse_y <= gui_y + gui_height);
}

// ===== HANDLERS DE CLIQUE =====

bool GUIManager::handleObjectTabClick(int local_x, int local_y) {
    // Verificar botões de brilho (se visíveis)
    if (selected_object_name.find("Lamina") != std::string::npos ||
        selected_object_name.find("lamina") != std::string::npos ||
        selected_material_name.find("Metal") != std::string::npos ||
        selected_material_name.find("Sword") != std::string::npos) {
        
        // Posição dos botões: após 7 linhas de info + 2 linhas de header brilho + separador
        int btn_y = 60 + 18 * 9 + 15;  // Ajustado para nova posição
        
        // Ligar Brilho
        if (local_x >= 10 && local_x <= 140 && local_y >= btn_y && local_y <= btn_y + 25) {
            std::cout << "[GUI] Ligar brilho da lamina\n";
            if (on_blade_shine_toggle) {
                on_blade_shine_toggle(true);  // Ligar
            }
            return true;
        }
        
        // Desligar Brilho
        if (local_x >= 150 && local_x <= 280 && local_y >= btn_y && local_y <= btn_y + 25) {
            std::cout << "[GUI] Desligar brilho da lamina\n";
            if (on_blade_shine_toggle) {
                on_blade_shine_toggle(false);  // Desligar
            }
            return true;
        }
    }
    return false;
}

bool GUIManager::handleCameraTabClick(int local_x, int local_y) {
    if (!cam_eye_ptr || !cam_at_ptr) return false;
    
    int line_height = 24;     // TEM QUE SER IGUAL AO DRAW
    int btn_h = 20;
    int btn_w = 25;
    double step_pos = 20.0;
    double step_up = 0.1;
    
    // Coordenadas locais (subtraindo gui_x)
    int x_btn_minus = 50;
    int x_btn_plus = 160;
    
    // Offset Y se há indicador de alterações pendentes
    int pending_offset = cam_has_pending_changes ? line_height : 0;
    
    // ===== EYE CONTROLS =====
    // Título Eye começa em 60 + pending_offset
    // Primeira linha (X): 60 + pending_offset + line_height
    int eye_start_y = 60 + pending_offset + line_height; 
    
    // Eye X (linha 0)
    int row_y = eye_start_y;
    if (local_y >= row_y && local_y <= row_y + btn_h) {
        if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
            pending_eye[0] -= step_pos;
            cam_has_pending_changes = true;
            return true;
        }
        if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
            pending_eye[0] += step_pos;
            cam_has_pending_changes = true;
            return true;
        }
    }
    
    // Eye Y (linha 1)
    row_y += line_height + 2;
    if (local_y >= row_y && local_y <= row_y + btn_h) {
        if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
            pending_eye[1] -= step_pos;
            cam_has_pending_changes = true;
            return true;
        }
        if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
            pending_eye[1] += step_pos;
            cam_has_pending_changes = true;
            return true;
        }
    }
    
    // Eye Z (linha 2)
    row_y += line_height + 2;
    if (local_y >= row_y && local_y <= row_y + btn_h) {
        if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
            pending_eye[2] -= step_pos;
            cam_has_pending_changes = true;
            return true;
        }
        if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
            pending_eye[2] += step_pos;
            cam_has_pending_changes = true;
            return true;
        }
    }
    
    // ===== AT CONTROLS =====
    // Título At começa em row_y (anterior) + line_height + 20
    // Primeira linha (X): titulo + line_height
    int at_start_y = row_y + line_height + 20 + line_height; 
    
    // At X
    row_y = at_start_y;
    if (local_y >= row_y && local_y <= row_y + btn_h) {
        if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
            pending_at[0] -= step_pos;
            cam_has_pending_changes = true;
            return true;
        }
        if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
            pending_at[0] += step_pos;
            cam_has_pending_changes = true;
            return true;
        }
    }
    
    // At Y
    row_y += line_height + 2;
    if (local_y >= row_y && local_y <= row_y + btn_h) {
        if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
            pending_at[1] -= step_pos;
            cam_has_pending_changes = true;
            return true;
        }
        if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
            pending_at[1] += step_pos;
            cam_has_pending_changes = true;
            return true;
        }
    }
    
    // At Z
    row_y += line_height + 2;
    if (local_y >= row_y && local_y <= row_y + btn_h) {
        if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
            pending_at[2] -= step_pos;
            cam_has_pending_changes = true;
            return true;
        }
        if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
            pending_at[2] += step_pos;
            cam_has_pending_changes = true;
            return true;
        }
    }
    
    // ===== UP CONTROLS =====
    int up_start_y = row_y + line_height + 20 + line_height;
    
    if (cam_up_ptr) {
        // Up X
        row_y = up_start_y;
        if (local_y >= row_y && local_y <= row_y + btn_h) {
            if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
                pending_up[0] -= step_up;
                cam_has_pending_changes = true;
                return true;
            }
            if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
                pending_up[0] += step_up;
                cam_has_pending_changes = true;
                return true;
            }
        }
        
        // Up Y
        row_y += line_height + 2;
        if (local_y >= row_y && local_y <= row_y + btn_h) {
            if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
                pending_up[1] -= step_up;
                cam_has_pending_changes = true;
                return true;
            }
            if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
                pending_up[1] += step_up;
                cam_has_pending_changes = true;
                return true;
            }
        }
        
        // Up Z
        row_y += line_height + 2;
        if (local_y >= row_y && local_y <= row_y + btn_h) {
            if (local_x >= x_btn_minus && local_x <= x_btn_minus + btn_w) {
                pending_up[2] -= step_up;
                cam_has_pending_changes = true;
                return true;
            }
            if (local_x >= x_btn_plus && local_x <= x_btn_plus + btn_w) {
                pending_up[2] += step_up;
                cam_has_pending_changes = true;
                return true;
            }
        }
        
        row_y += line_height + 8;
    } else {
        row_y = up_start_y + line_height + 8;
    }
    
    // ===== BOTÃO APLICAR E RENDERIZAR =====
    int apply_btn_y = row_y;
    if (local_y >= apply_btn_y && local_y <= apply_btn_y + 28 && local_x >= 10 && local_x <= gui_width - 10) {
        // Copiar valores pendentes para os reais
        cam_eye_ptr[0] = pending_eye[0]; cam_eye_ptr[1] = pending_eye[1]; cam_eye_ptr[2] = pending_eye[2];
        cam_at_ptr[0] = pending_at[0]; cam_at_ptr[1] = pending_at[1]; cam_at_ptr[2] = pending_at[2];
        if (cam_up_ptr) {
            cam_up_ptr[0] = pending_up[0]; cam_up_ptr[1] = pending_up[1]; cam_up_ptr[2] = pending_up[2];
        }
        
        // Resetar flag de pendentes
        cam_has_pending_changes = false;
        
        // Chamar callbacks para atualizar câmera e renderizar
        if (on_camera_change) on_camera_change();
        if (on_render_request) on_render_request();
        return true;
    }
    
    return false;
}

bool GUIManager::handleProjectionTabClick(int local_x, int local_y) {
    if (!projection_type_ptr) return false;
    
    int btn_h = 30;
    int btn_spacing = 35;
    int start_y = 60 + 25;
    
    // Perspectiva
    if (local_y >= start_y && local_y <= start_y + btn_h) {
        *projection_type_ptr = 0;
        std::cout << "[GUI] Projecao: Perspectiva\n";
        if (on_camera_change) on_camera_change();
        if (need_redraw_ptr) *need_redraw_ptr = true;
        return true;
    }
    start_y += btn_spacing;
    
    // Ortográfica
    if (local_y >= start_y && local_y <= start_y + btn_h) {
        *projection_type_ptr = 1;
        std::cout << "[GUI] Projecao: Ortografica\n";
        if (on_camera_change) on_camera_change();
        if (need_redraw_ptr) *need_redraw_ptr = true;
        return true;
    }
    start_y += btn_spacing;
    
    // Oblíqua
    if (local_y >= start_y && local_y <= start_y + btn_h) {
        *projection_type_ptr = 2;
        std::cout << "[GUI] Projecao: Obliqua\n";
        if (on_camera_change) on_camera_change();
        if (need_redraw_ptr) *need_redraw_ptr = true;
        return true;
    }
    
    return false;
}

bool GUIManager::handleEnvironmentTabClick(int local_x, int local_y) {
    int content_y = 60 + 22 * 4; // Ajustar conforme drawEnvironmentTab
    
    // Altura aproximada do botão DIA
    int btn_dia_y = content_y;
    if (local_y >= btn_dia_y && local_y <= btn_dia_y + 30 && local_x >= 10 && local_x <= gui_width - 10) {
        std::cout << "[GUI] Mudar para DIA\n";
        if (on_day_night_toggle) on_day_night_toggle(false); // false = dia
        return true;
    }
    
    // Altura aproximada do botão NOITE
    int btn_noite_y = content_y + 40;
    if (local_y >= btn_noite_y && local_y <= btn_noite_y + 30 && local_x >= 10 && local_x <= gui_width - 10) {
        std::cout << "[GUI] Mudar para NOITE\n";
        if (on_day_night_toggle) on_day_night_toggle(true); // true = noite
        return true;
    }
    
    return false;
}

bool GUIManager::handleTransformTabClick(int local_x, int local_y) {
    if (!selected_transform_name_ptr || selected_transform_name_ptr->empty() || !set_transform_state) return false;
    
    // Se não carregado, tentar carregar
    if (!pending_values_loaded) return false;
    
    // Sincronizar layout com drawTransformTab
    // Base: 60, Obj Info: +25, Status: +20, Trans Header: +20
    int content_y = 60 + 25 + 20 + 20;
    
    // Variáveis de range para cada eixo
    float t_min[3] = {500.0f, 0.0f, 500.0f};
    float t_max[3] = {1300.0f, 500.0f, 1300.0f};
    
    // Novo layout: Btn-(30), Btn+(54), Slider(78 até fim)
    // Altura de cada linha: 25px
    for(int i=0; i<3; i++) {
        // Botão - (x: 30-52, altura: 18)
        if (local_y >= content_y && local_y <= content_y + 18) {
            if (local_x >= 30 && local_x <= 52) {
                pending_translation[i] -= 10.0;
                if (pending_translation[i] < t_min[i]) pending_translation[i] = t_min[i];
                has_pending_changes = true;
                return true;
            }
            // Botão + (x: 54-76)
            if (local_x >= 54 && local_x <= 76) {
                pending_translation[i] += 10.0;
                if (pending_translation[i] > t_max[i]) pending_translation[i] = t_max[i];
                has_pending_changes = true;
                return true;
            }
            // Slider (x: 78 até gui_width-10)
            int slider_w = gui_width - 88;
            if (local_x >= 78 && local_x <= 78 + slider_w) {
                float ratio = (float)(local_x - 78) / (float)slider_w;
                if (ratio < 0.0f) ratio = 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;
                pending_translation[i] = t_min[i] + ratio * (t_max[i] - t_min[i]);
                has_pending_changes = true;
                return true;
            }
        }
        content_y += 25;
    }
    
    content_y += 10; // Spacing
    content_y += 20; // Rot Header
    
    // Lógica para Rotação
    for(int i=0; i<3; i++) {
        if (local_y >= content_y && local_y <= content_y + 18) {
            // Botão - (x: 30-52)
            if (local_x >= 30 && local_x <= 52) {
                pending_rotation[i] -= 5.0;
                if (pending_rotation[i] < 0) pending_rotation[i] += 360;
                has_pending_changes = true;
                return true;
            }
            // Botão + (x: 54-76)
            if (local_x >= 54 && local_x <= 76) {
                pending_rotation[i] += 5.0;
                if (pending_rotation[i] >= 360) pending_rotation[i] -= 360;
                has_pending_changes = true;
                return true;
            }
            // Slider (x: 78)
            int slider_w = gui_width - 88;
            if (local_x >= 78 && local_x <= 78 + slider_w) {
                float ratio = (float)(local_x - 78) / (float)slider_w;
                if (ratio < 0.0f) ratio = 0.0f;
                if (ratio > 1.0f) ratio = 1.0f;
                pending_rotation[i] = 0.0f + ratio * 360.0f;
                has_pending_changes = true;
                return true;
            }
        }
        content_y += 25;
    }
    
    // Botão APLICAR
    content_y += 10;
    if (local_y >= content_y && local_y <= content_y + 35) {
        if (local_x >= 10 && local_x <= gui_width - 10) {
            // Aplicar mudanças!
            set_transform_state(
                *selected_transform_name_ptr,
                pending_translation,
                pending_rotation,
                pending_scale
            );
            has_pending_changes = false;
            std::cout << "[GUI] Transformacoes APLICADAS!\n";
            return true;
        }
    }
    
    return false;
}

bool GUIManager::handleMouseClick(int mouse_x, int mouse_y, int button, int state) {
    if (!gui_visible) return false;
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return false;
    
    // Verificar botão fechar
    int close_x = gui_x + gui_width - 25;
    int close_y = gui_y + 2;
    
    if (mouse_x >= close_x && mouse_x <= close_x + 21 &&
        mouse_y >= close_y && mouse_y <= close_y + 21) {
        hide();
        return true;
    }
    
    // Verificar clique nas abas
    int tab_w = gui_width / 5;
    int tab_y = gui_y + 25;
    int tab_h = 25;
    
    if (mouse_y >= tab_y && mouse_y <= tab_y + tab_h) {
        if (mouse_x >= gui_x && mouse_x < gui_x + tab_w) {
            current_tab = 0;
            gui_height = 320; // Altura normal
            return true;
        } else if (mouse_x >= gui_x + tab_w && mouse_x < gui_x + tab_w * 2) {
            current_tab = 1;
            gui_height = 550; // Altura expandida para aba Camera (controles Eye, At, Up)
            return true;
        } else if (mouse_x >= gui_x + tab_w * 2 && mouse_x < gui_x + tab_w * 3) {
            current_tab = 2;
            gui_height = 320; // Altura normal
            return true;
        } else if (mouse_x >= gui_x + tab_w * 3 && mouse_x < gui_x + tab_w * 4) {
            current_tab = 3;
            gui_height = 320; // Altura normal
            return true;
        } else if (mouse_x >= gui_x + tab_w * 4 && mouse_x <= gui_x + gui_width) {
            current_tab = 4;
            gui_height = 480; // Altura expandida para aba de Transformação
            // Ao entrar na aba Transform, forçar recarregamento na próxima draw
            pending_values_loaded = false;
            return true;
        }
    }
    
    // Processar cliques no conteúdo da aba
    int local_x = mouse_x - gui_x;
    int local_y = mouse_y - gui_y;
    
    switch (current_tab) {
        case 0: 
            if (handleObjectTabClick(local_x, local_y)) return true;
            break;
        case 1: 
            if (handleCameraTabClick(local_x, local_y)) return true;
            break;
        case 2: 
            if (handleProjectionTabClick(local_x, local_y)) return true;
            break;
        case 3: 
            if (handleEnvironmentTabClick(local_x, local_y)) return true;
            break;
        case 4: 
            if (handleTransformTabClick(local_x, local_y)) return true;
            break;
    }
    
    // Se clicou dentro da GUI, consumir o evento
    if (isMouseOver(mouse_x, mouse_y)) {
        return true;
    }
    
    return false;
}
