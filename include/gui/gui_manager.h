#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <string>
#include <functional>
#include <GL/freeglut.h>

// Forward declarations
class hit_record;
class material;

/**
 * @brief Gerenciador de GUI para o Ray Caster
 * 
 * Responsável por desenhar e gerenciar painéis de:
 * - Propriedades do objeto selecionado
 * - Controles de câmera (Eye, At, Up)
 * - Projeções
 * - Modificação de materiais
 */
class GUIManager {
public:
    // ===== ESTADO GERAL DA GUI =====
    static bool gui_visible;
    static int gui_x, gui_y;
    static int gui_width, gui_height;
    
    // ===== PAINEL DE OBJETO SELECIONADO =====
    static std::string selected_object_name;
    static std::string selected_material_name;
    static double selected_position[3];
    static double selected_normal[3];
    static double selected_distance;
    
    // ===== PAINEL DE CÂMERA =====
    static double* cam_eye_ptr;
    static double* cam_at_ptr;
    static double* cam_up_ptr;
    static int* projection_type_ptr;
    static bool* need_redraw_ptr;
    
    // ===== ESTADO DO BRILHO DA LÂMINA =====
    static bool* blade_shine_ptr;

    // ===== ESTADO DO AMBIENTE =====
    static bool* is_night_mode_ptr;
    
    // ===== ESTADO DE TRANSFORMAÇÃO =====
    static std::string* selected_transform_name_ptr;
    
    // Variáveis para "Draft Mode" (Edição sem aplicar imediatamente)
    static double pending_translation[3];
    static double pending_rotation[3];
    static double pending_scale[3];
    static bool has_pending_changes;
    static bool pending_values_loaded; // Se já carregamos os valores do objeto selecionado
    
    // Draft Mode para Câmera
    static double pending_eye[3];
    static double pending_at[3];
    static double pending_up[3];
    static bool cam_pending_loaded;
    static bool cam_has_pending_changes;
    
    // ===== CALLBACKS =====
    static std::function<void()> on_camera_change;
    static std::function<void()> on_render_request;
    static std::function<void(bool)> on_blade_shine_toggle;  // bool = increase
    static std::function<void(bool)> on_day_night_toggle;    // bool = set_to_night
    
    // Callbacks de Transformação
    // Get: Retorna true se objeto transformavel e preenche arrays
    static std::function<bool(const std::string&, double* trans, double* rot, double* scale)> get_transform_state;
    // Set: Atualiza estado
    static std::function<void(const std::string&, const double* trans, const double* rot, const double* scale)> set_transform_state;

    // ===== MÉTODOS PÚBLICOS =====
    
    // Inicializar a GUI com ponteiros para variáveis da main
    static void init(double* eye, double* at, double* up, int* proj_type, bool* redraw, bool* blade_shine, bool* is_night, std::string* sel_trans_name);
    
    // Configurar callbacks
    static void setCallbacks(
        std::function<void()> cam_change,
        std::function<void()> render_req,
        std::function<void(bool)> blade_toggle,
        std::function<void(bool)> day_night_toggle,
        std::function<bool(const std::string&, double*, double*, double*)> get_trans,
        std::function<void(const std::string&, const double*, const double*, const double*)> set_trans
    );
    
    // Mostrar a GUI com informações do objeto
    static void show(const std::string& object_name, 
                     const std::string& material_name,
                     double px, double py, double pz,
                     double nx, double ny, double nz,
                     double distance);
    
    // Esconder a GUI
    static void hide();
    
    // Alternar visibilidade
    static void toggle();
    
    // Desenhar a GUI na tela
    static void draw();
    
    // Verificar se o mouse está sobre a GUI
    static bool isMouseOver(int mouse_x, int mouse_y);
    
    // Processar clique do mouse na GUI
    static bool handleMouseClick(int mouse_x, int mouse_y, int button, int state);

private:
    // Tabs/seções da GUI
    static int current_tab;  // 0=Objeto, 1=Camera, 2=Projecao, 3=Ambiente
    
    // Desenhar componentes
    static void drawRect(int x, int y, int w, int h, float r, float g, float b, float a);
    static void drawText(int x, int y, const std::string& text, float r, float g, float b);
    static void drawButton(int x, int y, int w, int h, const std::string& label, bool active = false);
    static void drawSlider(int x, int y, int w, float value, float min_val, float max_val, const std::string& label);
    
    // Desenhar cada aba
    static void drawObjectTab();
    static void drawCameraTab();
    static void drawProjectionTab();
    static void drawEnvironmentTab();
    static void drawTransformTab();
    static void drawTabs();
    
    // Processar cliques em cada aba
    static bool handleObjectTabClick(int local_x, int local_y);
    static bool handleCameraTabClick(int local_x, int local_y);
    static bool handleProjectionTabClick(int local_x, int local_y);
    static bool handleEnvironmentTabClick(int local_x, int local_y);
    static bool handleTransformTabClick(int local_x, int local_y);
};

#endif // GUI_MANAGER_H
