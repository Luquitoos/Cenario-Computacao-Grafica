#include "../../include/gui/gui_manager.h"
#include <sstream>
#include <iomanip>

// Inicialização das variáveis estáticas
bool GUIManager::gui_visible = false;
int GUIManager::gui_x = 10;
int GUIManager::gui_y = 10;
int GUIManager::gui_width = 280;
int GUIManager::gui_height = 200;

std::string GUIManager::selected_object_name = "";
std::string GUIManager::selected_material_name = "";
double GUIManager::selected_position[3] = {0, 0, 0};
double GUIManager::selected_normal[3] = {0, 0, 0};
double GUIManager::selected_distance = 0;

void GUIManager::init() {
    gui_visible = false;
    gui_x = 10;
    gui_y = 10;
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
}

void GUIManager::hide() {
    gui_visible = false;
}

void GUIManager::toggle() {
    gui_visible = !gui_visible;
}

void GUIManager::drawRect(int x, int y, int w, int h, float r, float g, float b, float a) {
    // Salvar estado do OpenGL
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    
    // Configurar para desenho 2D
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluOrtho2D(0, viewport[2], viewport[3], 0);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    
    // Habilitar blending para transparência
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    // Desenhar retângulo preenchido
    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
    
    // Desenhar borda
    glColor4f(0.8f, 0.8f, 0.8f, 1.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(x, y);
    glVertex2i(x + w, y);
    glVertex2i(x + w, y + h);
    glVertex2i(x, y + h);
    glEnd();
    
    // Restaurar estado
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

void GUIManager::drawButton(int x, int y, int w, int h, const std::string& label) {
    // Fundo do botão
    drawRect(x, y, w, h, 0.3f, 0.3f, 0.4f, 0.9f);
    
    // Texto centralizado
    int text_x = x + (w - label.length() * 7) / 2;
    int text_y = y + h / 2 + 4;
    drawText(text_x, text_y, label, 1.0f, 1.0f, 1.0f);
}

void GUIManager::draw() {
    if (!gui_visible) return;
    
    // Fundo da janela (semi-transparente)
    drawRect(gui_x, gui_y, gui_width, gui_height, 0.1f, 0.1f, 0.15f, 0.9f);
    
    // Barra de título
    drawRect(gui_x, gui_y, gui_width, 25, 0.2f, 0.3f, 0.5f, 1.0f);
    drawText(gui_x + 10, gui_y + 17, "Propriedades do Objeto", 1.0f, 1.0f, 1.0f);
    
    // Botão fechar (X)
    drawRect(gui_x + gui_width - 25, gui_y + 2, 21, 21, 0.6f, 0.2f, 0.2f, 1.0f);
    drawText(gui_x + gui_width - 19, gui_y + 17, "X", 1.0f, 1.0f, 1.0f);
    
    // Conteúdo
    int content_y = gui_y + 35;
    int line_height = 18;
    
    // Nome do objeto
    drawText(gui_x + 10, content_y, "Objeto:", 0.7f, 0.7f, 0.7f);
    drawText(gui_x + 80, content_y, selected_object_name, 1.0f, 0.9f, 0.3f);
    content_y += line_height;
    
    // Material
    drawText(gui_x + 10, content_y, "Material:", 0.7f, 0.7f, 0.7f);
    drawText(gui_x + 80, content_y, selected_material_name, 0.5f, 0.8f, 1.0f);
    content_y += line_height;
    
    // Separador
    content_y += 5;
    
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
}

bool GUIManager::isMouseOver(int mouse_x, int mouse_y) {
    if (!gui_visible) return false;
    
    return (mouse_x >= gui_x && mouse_x <= gui_x + gui_width &&
            mouse_y >= gui_y && mouse_y <= gui_y + gui_height);
}

bool GUIManager::handleMouseClick(int mouse_x, int mouse_y, int button, int state) {
    if (!gui_visible) return false;
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN) return false;
    
    // Verificar clique no botão fechar (X)
    int close_x = gui_x + gui_width - 25;
    int close_y = gui_y + 2;
    
    if (mouse_x >= close_x && mouse_x <= close_x + 21 &&
        mouse_y >= close_y && mouse_y <= close_y + 21) {
        hide();
        return true; // Clique consumido
    }
    
    // Se clicou dentro da GUI, consumir o evento
    if (isMouseOver(mouse_x, mouse_y)) {
        return true;
    }
    
    return false;
}
