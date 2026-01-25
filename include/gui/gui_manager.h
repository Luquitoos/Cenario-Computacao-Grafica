#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <string>
#include <GL/freeglut.h>

// Forward declarations
class hit_record;
class material;

/**
 * @brief Gerenciador de GUI para o Ray Caster
 * 
 * Responsável por desenhar e gerenciar uma janela de propriedades
 * que aparece quando um objeto é selecionado via pick.
 */
class GUIManager {
public:
    // Estado da GUI
    static bool gui_visible;
    static int gui_x, gui_y;
    static int gui_width, gui_height;
    
    // Informações do objeto selecionado
    static std::string selected_object_name;
    static std::string selected_material_name;
    static double selected_position[3];
    static double selected_normal[3];
    static double selected_distance;
    
    // Inicializar a GUI
    static void init();
    
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
    
    // Desenhar a GUI na tela (chamado no display)
    static void draw();
    
    // Verificar se o mouse está sobre a GUI
    static bool isMouseOver(int mouse_x, int mouse_y);
    
    // Processar clique do mouse na GUI
    static bool handleMouseClick(int mouse_x, int mouse_y, int button, int state);

private:
    // Desenhar um retângulo com fundo
    static void drawRect(int x, int y, int w, int h, float r, float g, float b, float a);
    
    // Desenhar texto
    static void drawText(int x, int y, const std::string& text, float r, float g, float b);
    
    // Desenhar um botão
    static void drawButton(int x, int y, int w, int h, const std::string& label);
};

#endif // GUI_MANAGER_H
