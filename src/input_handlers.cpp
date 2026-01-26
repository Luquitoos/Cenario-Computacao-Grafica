#include "../include/input_handlers.h"
#include "../include/globals.h"
#include "../include/gui/gui_manager.h"
#include "../include/renderer.h"
#include "../include/scene_setup.h"
#include <GL/freeglut.h>
#include <iostream>
#include <string>

using namespace std;

void perform_pick(int mouse_x, int mouse_y) {
  int window_height = glutGet(GLUT_WINDOW_HEIGHT);
  int image_y_start = window_height - IMAGE_HEIGHT;

  if (mouse_x < 0 || mouse_x >= IMAGE_WIDTH || mouse_y < image_y_start ||
      mouse_y >= window_height) {
    cout << "\n[Pick] Clique fora da area da imagem\n";
    return;
  }

  int image_mouse_y = mouse_y - image_y_start;
  double u = double(mouse_x) / IMAGE_WIDTH;
  double v = double(IMAGE_HEIGHT - image_mouse_y) / IMAGE_HEIGHT;

  ray r = cam.get_ray(u, v);
  hit_record rec;

  cout << "\n========== PICK ==========\n";
  cout << "Mouse: (" << mouse_x << ", " << mouse_y << ")\n";
  cout << "Image Mouse Y: " << image_mouse_y << "\n";
  cout << "UV: (" << u << ", " << v << ")\n";

  if (world.hit(r, 0.001, numeric_limits<double>::infinity(), rec)) {
    picked_object = rec.object_name;
    cout << "OBJETO: " << rec.object_name << "\n";
    cout << "Material: " << rec.mat->name << "\n";
    cout << "Posicao: (" << rec.p.x() << ", " << rec.p.y() << ", " << rec.p.z()
         << ")\n";
    cout << "Normal: (" << rec.normal.x() << ", " << rec.normal.y() << ", "
         << rec.normal.z() << ")\n";
    cout << "Distancia (t): " << rec.t << "\n";

    GUIManager::show(rec.object_name, rec.mat->name, rec.p.x(), rec.p.y(),
                     rec.p.z(), rec.normal.x(), rec.normal.y(), rec.normal.z(),
                     rec.t);

    selected_transform_name = "";

    if (rec.object_name == "Quaternion Sapphire Gem" ||
        rec.object_name == "Sapphire Gem") {
      selected_transform_name = "Sapphire Gem";
    } else if (rec.object_name == "Quaternion Emerald" ||
               rec.object_name == "Emerald Gem") {
      selected_transform_name = "Emerald Gem";
    } else if (rec.object_name == "Pomo" || rec.object_name == "Ruby Gem") {
      selected_transform_name = "Ruby Gem";
    }

    else if (rec.object_name == "Lamina" ||
             rec.object_name == "Guarda Principal" ||
             rec.object_name == "Guarda Esq" ||
             rec.object_name == "Guarda Dir" ||
             rec.object_name == "Guarda Centro" || rec.object_name == "Cabo" ||
             rec.object_name == "Ponta") {
      selected_transform_name = "Espada Completa";
    }

    else if (rec.object_name == "Torch Pole" ||
             rec.object_name == "Torch Flame Outer" ||
             rec.object_name == "Torch Flame Core") {
      selected_transform_name = "Tocha Medieval";
    }

    else if (rec.object_name == "Ruined Pillar 1" ||
             rec.object_name == "Pillar 1 Capital") {
      selected_transform_name = "Pilar Ruina 1";
    }

    else if (rec.object_name == "Ruined Pillar 2" ||
             rec.object_name == "Pillar 2 Capital") {
      selected_transform_name = "Pilar Ruina 2";
    }

    else if (rec.object_name == "Tree Trunk" ||
             rec.object_name == "Tree Branch" ||
             rec.object_name == "Tree Leaves") {

      double CX = 900.0, CZ = 900.0;
      point3 tree1_pos(CX - 280, 0, CZ - 120);
      point3 tree2_pos(CX + 300, 0, CZ + 80);

      double dist1 = (rec.p - tree1_pos).length();
      double dist2 = (rec.p - tree2_pos).length();

      if (dist1 < dist2) {
        selected_transform_name = "Arvore 1";
      } else {
        selected_transform_name = "Arvore 2";
      }
    }

    else if (rec.object_name == "Nucleo Montanha" ||
             rec.object_name == "Pedra Base" ||
             rec.object_name == "Pedra Topo") {
      selected_transform_name = "Rocha Principal";
    }

    else if (rec.object_name == "Lake Rock") {
      selected_transform_name = "Lake Rocks";
    }

    else if (rec.object_name == "Mushroom Stem" ||
             rec.object_name == "Mushroom Cap") {

      double min_dist = 1e10;
      for (auto &[name, state] : object_states) {
        if (name.find("Mushroom ") == 0) {
          double dist =
              (rec.p - point3(state.translation.x(), state.translation.y(),
                              state.translation.z()))
                  .length();
          if (dist < min_dist) {
            min_dist = dist;
            selected_transform_name = name;
          }
        }
      }
    }

    else if (rec.object_name == "Colony Stem" ||
             rec.object_name == "Colony Cap") {
      double min_dist = 1e10;
      for (auto &[name, state] : object_states) {
        if (name.find("Colony ") == 0) {
          double dist =
              (rec.p - point3(state.translation.x(), state.translation.y(),
                              state.translation.z()))
                  .length();
          if (dist < min_dist) {
            min_dist = dist;
            selected_transform_name = name;
          }
        }
      }
    }

    else if (object_transforms.find(rec.object_name) !=
             object_transforms.end()) {
      selected_transform_name = rec.object_name;
    }

    if (!selected_transform_name.empty()) {
      cout << "Selecionado Transformavel: " << selected_transform_name << "\n";
    }

  } else {
    picked_object = "Fundo (Ceu)";
    selected_transform_name = "";
    cout << "OBJETO: Fundo (Ceu)\n";
    GUIManager::hide();
  }
  cout << "==========================\n";
}

void display() {
  if (need_redraw) {
    render();
  }

  glClear(GL_COLOR_BUFFER_BIT);
  glRasterPos2i(-1, -1);
  glDrawPixels(IMAGE_WIDTH, IMAGE_HEIGHT, GL_RGB, GL_UNSIGNED_BYTE,
               PixelBuffer);

  glColor3f(1.0f, 1.0f, 0.0f);
  glRasterPos2f(-0.98f, 0.92f);

  string info = "Projecao: ";
  switch (current_projection) {
  case 0:
    info += "Perspectiva (" + to_string(vanishing_points) + " PF)";
    break;
  case 1:
    info += "Ortografica";
    break;
  case 2:
    info += "Obliqua";
    break;
  }
  for (char c : info) {
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
  }

  if (!picked_object.empty()) {
    glRasterPos2f(-0.98f, 0.85f);
    string pick_info = "Pick: " + picked_object;
    for (char c : pick_info) {
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
  }

  GUIManager::draw();

  glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y) {
  bool changed = false;
  vec3 forward = unit_vector(cam_at - cam_eye);
  vec3 right = unit_vector(cross(forward, vec3(0, 1, 0)));

  switch (key) {
  case 27:
  case 'q':
  case 'Q':
    cout << "Encerrando...\n";
    if (PixelBuffer)
      delete[] PixelBuffer;
    exit(0);
    break;

  case 'w':
  case 'W':
    cam_eye = cam_eye + forward * cam_speed;
    cam_at = cam_at + forward * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 's':
  case 'S':
    cam_eye = cam_eye - forward * cam_speed;
    cam_at = cam_at - forward * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 'a':
  case 'A':
    cam_eye = cam_eye - right * cam_speed;
    cam_at = cam_at - right * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 'd':
  case 'D':
    cam_eye = cam_eye + right * cam_speed;
    cam_at = cam_at + right * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 'r':
  case 'R':
    cam_eye[1] += cam_speed;
    cam_at[1] += cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case 'f':
  case 'F':
    cam_eye[1] -= cam_speed;
    cam_at[1] -= cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case '1':
    current_projection = 0;
    vanishing_points = 1;
    setup_camera();
    need_redraw = true;
    changed = true;
    cout << "Perspectiva\n";
    break;

  case 'o':
  case 'O':
    current_projection = 1;
    setup_camera();
    need_redraw = true;
    changed = true;
    cout << "Projecao Ortografica\n";
    break;

  case 'p':
  case 'P':
    current_projection = 2;
    setup_camera();
    need_redraw = true;
    changed = true;
    cout << "Projecao Obliqua\n";
    break;

  case '+':
  case '=':
    cam.zoom_in(0.8);
    need_redraw = true;
    changed = true;
    cout << "Zoom In\n";
    break;

  case '-':
  case '_':
    cam.zoom_out(1.25);
    need_redraw = true;
    changed = true;
    cout << "Zoom Out\n";
    break;

  case 'h':
  case 'H':
    cout << "\n=== CONTROLES ===\n";
    cout << "WASD - Mover camera (frente/tras/esq/dir)\n";
    cout << "R/F - Subir/Descer camera\n";
    cout << "Setas - Rotacionar camera\n";
    cout << "G - Abrir/Fechar GUI\n";
    cout << "1 - Perspectiva | O - Ortografica | P - Obliqua\n";
    cout << "+/- - Zoom In/Out\n";
    cout << "Click - Pick de objeto\n";
    cout << "N - Alternar Dia/Noite\n";
    cout << "Q/ESC - Sair\n";
    cout << "=================\n\n";
    break;

  case 'g':
  case 'G':
    GUIManager::toggle();
    glutPostRedisplay();
    break;

  case 'c':
  case 'C':
    cam_eye = point3(400, 200, 100);
    cam_at = point3(250, 100, 250);
    setup_camera();
    need_redraw = true;
    changed = true;
    cout << "Camera resetada\n";
    break;

  case 'n':
  case 'N':
    toggle_day_night(!is_night_mode);
    break;
  }

  if (changed) {
    glutPostRedisplay();
  }
}

void special_keys(int key, int x, int y) {
  bool changed = false;
  vec3 up(0, 1, 0);
  vec3 forward = unit_vector(cam_at - cam_eye);
  vec3 right = unit_vector(cross(forward, vec3(0, 1, 0)));

  switch (key) {
  case GLUT_KEY_UP:
    cam_at = cam_at + up * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case GLUT_KEY_DOWN:
    cam_at = cam_at - up * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case GLUT_KEY_LEFT:
    cam_at = cam_at - right * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;

  case GLUT_KEY_RIGHT:
    cam_at = cam_at + right * cam_speed;
    setup_camera();
    need_redraw = true;
    changed = true;
    break;
  }

  if (changed) {
    glutPostRedisplay();
  }
}

void mouse(int button, int state, int x, int y) {
  if (GUIManager::handleMouseClick(x, y, button, state)) {
    glutPostRedisplay();
    return;
  }

  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    perform_pick(x, y);
    glutPostRedisplay();
  }
}

void reshape(int w, int h) {
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(-1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
