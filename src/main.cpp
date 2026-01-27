
#include <GL/freeglut.h>
#include <iostream>

#include "../include/globals.h"
#include "../include/gui/gui_manager.h"
#include "../include/input_handlers.h"
#include "../include/renderer.h"
#include "../include/scene_setup.h"
#include "../include/gpu/gpu_renderer.h"

using namespace std;

int main(int argc, char **argv) {
  cout << "============================================\n";
  cout << "  COMPUTACAO GRAFICA - ESPADA NA PEDRA\n";
  cout << "  Resolucao: " << IMAGE_WIDTH << "x" << IMAGE_HEIGHT << "\n";
  cout << "============================================\n\n";

  PixelBuffer = new unsigned char[IMAGE_WIDTH * IMAGE_HEIGHT * 3];
  memset(PixelBuffer, 0, IMAGE_WIDTH * IMAGE_HEIGHT * 3);

  cout << "Criando cena...\n";
  create_scene();
  
  cout << "Construindo BVH para aceleracao...\n";
  build_scene_bvh();

  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
  glutInitWindowSize(IMAGE_WIDTH, IMAGE_HEIGHT);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("CG - Espada na Pedra (Ray Caster)");

  // Inicializa GPU (após criar janela OpenGL)
  cout << "Inicializando GPU...\n";
  if (gpu_init()) {
    cout << "[GPU] " << gpu_get_info() << " - ATIVADA\n";
    gpu_upload_scene();
  } else {
    cout << "[GPU] Compute shaders indisponiveis. Usando CPU + OpenMP.\n";
  }

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(special_keys);
  glutMouseFunc(mouse);

  GUIManager::init(&cam_eye[0], &cam_at[0], &cam_up[0], &current_projection,
                   &need_redraw, &blade_shine_enabled, &is_night_mode,
                   &selected_transform_name, &vanishing_points_preset);

  GUIManager::setCallbacks(

      []() {
        setup_camera();
        glutPostRedisplay();
      },

      []() {
        need_redraw = true;
        glutPostRedisplay();
      },

      [](bool state) { toggle_blade_shine(state); },

      [](bool is_night) { toggle_day_night(is_night); },

      [](int preset) {
        apply_vanishing_point_preset(preset);
        setup_camera();
        glutPostRedisplay();
      },

      [](const string &name, double *t, double *r, double *s) -> bool {
        if (object_states.find(name) != object_states.end()) {
          TransformState &st = object_states[name];
          t[0] = st.translation.x();
          t[1] = st.translation.y();
          t[2] = st.translation.z();
          r[0] = st.rotation.x();
          r[1] = st.rotation.y();
          r[2] = st.rotation.z();
          s[0] = st.scale.x();
          s[1] = st.scale.y();
          s[2] = st.scale.z();
          return true;
        }
        return false;
      },

      [](const string &name, const double *t, const double *r,
         const double *s) {
        if (object_states.find(name) != object_states.end()) {
          TransformState &st = object_states[name];
          st.translation = vec3(t[0], t[1], t[2]);
          st.rotation = vec3(r[0], r[1], r[2]);
          st.scale = vec3(s[0], s[1], s[2]);
          update_object_transform(name);

          if (name == "Espada Completa") {
            update_sword_light();
          }
        }
      });

  cout << "\nSistema inicializado. Pressione H para lista de comandos.\n";

  glutMainLoop();

  return 0;
}
