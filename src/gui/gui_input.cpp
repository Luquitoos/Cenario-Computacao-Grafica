#include "../../include/gui/gui_manager.h"
#include <iostream>

using namespace std;

bool GUIManager::isMouseOver(int mouse_x, int mouse_y) {
  if (!gui_visible)
    return false;
  return (mouse_x >= gui_x && mouse_x <= gui_x + gui_width &&
          mouse_y >= gui_y && mouse_y <= gui_y + gui_height);
}

bool GUIManager::handleObjectTabClick(int local_x, int local_y) {

  if (selected_object_name.find("Lamina") != string::npos ||
      selected_object_name.find("lamina") != string::npos ||
      selected_material_name.find("Metal") != string::npos ||
      selected_material_name.find("Sword") != string::npos) {

    int btn_y = 60 + 18 * 9 + 15;

    if (local_x >= 10 && local_x <= 140 && local_y >= btn_y &&
        local_y <= btn_y + 25) {
      cout << "[GUI] Ligar brilho da lamina\n";
      if (on_blade_shine_toggle) {
        on_blade_shine_toggle(true);
      }
      return true;
    }

    if (local_x >= 150 && local_x <= 280 && local_y >= btn_y &&
        local_y <= btn_y + 25) {
      cout << "[GUI] Desligar brilho da lamina\n";
      if (on_blade_shine_toggle) {
        on_blade_shine_toggle(false);
      }
      return true;
    }
  }
  return false;
}

bool GUIManager::handleCameraTabClick(int local_x, int local_y) {
  if (!cam_eye_ptr || !cam_at_ptr)
    return false;

  int line_height = 24;
  int btn_h = 20;
  int btn_w = 25;
  double step_pos = 20.0;
  double step_up = 0.1;

  int x_btn_minus = 50;
  int x_btn_plus = 160;

  int pending_offset = cam_has_pending_changes ? line_height : 0;

  int eye_start_y = 60 + pending_offset + line_height;

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

  int at_start_y = row_y + line_height + 20 + line_height;

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

  int up_start_y = row_y + line_height + 20 + line_height;

  if (cam_up_ptr) {

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

  int apply_btn_y = row_y;
  if (local_y >= apply_btn_y && local_y <= apply_btn_y + 28 && local_x >= 10 &&
      local_x <= gui_width - 10) {

    cam_eye_ptr[0] = pending_eye[0];
    cam_eye_ptr[1] = pending_eye[1];
    cam_eye_ptr[2] = pending_eye[2];
    cam_at_ptr[0] = pending_at[0];
    cam_at_ptr[1] = pending_at[1];
    cam_at_ptr[2] = pending_at[2];
    if (cam_up_ptr) {
      cam_up_ptr[0] = pending_up[0];
      cam_up_ptr[1] = pending_up[1];
      cam_up_ptr[2] = pending_up[2];
    }

    cam_has_pending_changes = false;

    if (on_camera_change)
      on_camera_change();
    if (on_render_request)
      on_render_request();
    return true;
  }

  return false;
}

bool GUIManager::handleProjectionTabClick(int local_x, int local_y) {
  if (!projection_type_ptr)
    return false;

  int btn_h = 30;
  int btn_spacing = 35;
  int start_y = 60 + 25;

  if (local_y >= start_y && local_y <= start_y + btn_h) {
    *projection_type_ptr = 0;
    cout << "[GUI] Projecao: Perspectiva\n";
    if (on_camera_change)
      on_camera_change();
    if (need_redraw_ptr)
      *need_redraw_ptr = true;
    return true;
  }
  start_y += btn_spacing;

  if (local_y >= start_y && local_y <= start_y + btn_h) {
    *projection_type_ptr = 1;
    cout << "[GUI] Projecao: Ortografica\n";
    if (on_camera_change)
      on_camera_change();
    if (need_redraw_ptr)
      *need_redraw_ptr = true;
    return true;
  }
  start_y += btn_spacing;

  if (local_y >= start_y && local_y <= start_y + btn_h) {
    *projection_type_ptr = 2;
    cout << "[GUI] Projecao: Obliqua\n";
    if (on_camera_change)
      on_camera_change();
    if (need_redraw_ptr)
      *need_redraw_ptr = true;
    return true;
  }

  return false;
}

bool GUIManager::handleEnvironmentTabClick(int local_x, int local_y) {
  int content_y = 60 + 22 * 4;

  int btn_dia_y = content_y;
  if (local_y >= btn_dia_y && local_y <= btn_dia_y + 30 && local_x >= 10 &&
      local_x <= gui_width - 10) {
    cout << "[GUI] Mudar para DIA\n";
    if (on_day_night_toggle)
      on_day_night_toggle(false);
    return true;
  }

  int btn_noite_y = content_y + 40;
  if (local_y >= btn_noite_y && local_y <= btn_noite_y + 30 && local_x >= 10 &&
      local_x <= gui_width - 10) {
    cout << "[GUI] Mudar para NOITE\n";
    if (on_day_night_toggle)
      on_day_night_toggle(true);
    return true;
  }

  return false;
}

bool GUIManager::handleTransformTabClick(int local_x, int local_y) {
  if (!selected_transform_name_ptr || selected_transform_name_ptr->empty() ||
      !set_transform_state)
    return false;

  if (!pending_values_loaded)
    return false;

  int content_y = 60 + 25 + 20 + 20;
  float t_min[3] = {500.0f, 0.0f, 500.0f};
  float t_max[3] = {1300.0f, 500.0f, 1300.0f};

  for (int i = 0; i < 3; i++) {

    if (local_y >= content_y && local_y <= content_y + 18) {
      if (local_x >= 30 && local_x <= 52) {
        pending_translation[i] -= 10.0;
        if (pending_translation[i] < t_min[i])
          pending_translation[i] = t_min[i];
        has_pending_changes = true;
        return true;
      }

      if (local_x >= 54 && local_x <= 76) {
        pending_translation[i] += 10.0;
        if (pending_translation[i] > t_max[i])
          pending_translation[i] = t_max[i];
        has_pending_changes = true;
        return true;
      }

      int slider_w = gui_width - 88;
      if (local_x >= 78 && local_x <= 78 + slider_w) {
        float ratio = (float)(local_x - 78) / (float)slider_w;
        if (ratio < 0.0f)
          ratio = 0.0f;
        if (ratio > 1.0f)
          ratio = 1.0f;
        pending_translation[i] = t_min[i] + ratio * (t_max[i] - t_min[i]);
        has_pending_changes = true;
        return true;
      }
    }
    content_y += 25;
  }

  content_y += 10;
  content_y += 20;

  for (int i = 0; i < 3; i++) {
    if (local_y >= content_y && local_y <= content_y + 18) {

      if (local_x >= 30 && local_x <= 52) {
        pending_rotation[i] -= 5.0;
        if (pending_rotation[i] < 0)
          pending_rotation[i] += 360;
        has_pending_changes = true;
        return true;
      }

      if (local_x >= 54 && local_x <= 76) {
        pending_rotation[i] += 5.0;
        if (pending_rotation[i] >= 360)
          pending_rotation[i] -= 360;
        has_pending_changes = true;
        return true;
      }

      int slider_w = gui_width - 88;
      if (local_x >= 78 && local_x <= 78 + slider_w) {
        float ratio = (float)(local_x - 78) / (float)slider_w;
        if (ratio < 0.0f)
          ratio = 0.0f;
        if (ratio > 1.0f)
          ratio = 1.0f;
        pending_rotation[i] = 0.0f + ratio * 360.0f;
        has_pending_changes = true;
        return true;
      }
    }
    content_y += 25;
  }

  content_y += 10;
  if (local_y >= content_y && local_y <= content_y + 35) {
    if (local_x >= 10 && local_x <= gui_width - 10) {

      set_transform_state(*selected_transform_name_ptr, pending_translation,
                          pending_rotation, pending_scale);
      has_pending_changes = false;
      cout << "[GUI] Transformacoes APLICADAS!\n";
      return true;
    }
  }

  return false;
}

bool GUIManager::handleMouseClick(int mouse_x, int mouse_y, int button,
                                  int state) {
  if (!gui_visible)
    return false;
  if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN)
    return false;

  int close_x = gui_x + gui_width - 25;
  int close_y = gui_y + 2;

  if (mouse_x >= close_x && mouse_x <= close_x + 21 && mouse_y >= close_y &&
      mouse_y <= close_y + 21) {
    hide();
    return true;
  }

  int tab_w = gui_width / 5;
  int tab_y = gui_y + 25;
  int tab_h = 25;

  if (mouse_y >= tab_y && mouse_y <= tab_y + tab_h) {
    if (mouse_x >= gui_x && mouse_x < gui_x + tab_w) {
      current_tab = 0;
      gui_height = 320;
      return true;
    } else if (mouse_x >= gui_x + tab_w && mouse_x < gui_x + tab_w * 2) {
      current_tab = 1;
      gui_height = 550;
      return true;
    } else if (mouse_x >= gui_x + tab_w * 2 && mouse_x < gui_x + tab_w * 3) {
      current_tab = 2;
      gui_height = 320;
      return true;
    } else if (mouse_x >= gui_x + tab_w * 3 && mouse_x < gui_x + tab_w * 4) {
      current_tab = 3;
      gui_height = 320;
      return true;
    } else if (mouse_x >= gui_x + tab_w * 4 && mouse_x <= gui_x + gui_width) {
      current_tab = 4;
      gui_height = 480;

      pending_values_loaded = false;
      return true;
    }
  }

  int local_x = mouse_x - gui_x;
  int local_y = mouse_y - gui_y;

  switch (current_tab) {
  case 0:
    if (handleObjectTabClick(local_x, local_y))
      return true;
    break;
  case 1:
    if (handleCameraTabClick(local_x, local_y))
      return true;
    break;
  case 2:
    if (handleProjectionTabClick(local_x, local_y))
      return true;
    break;
  case 3:
    if (handleEnvironmentTabClick(local_x, local_y))
      return true;
    break;
  case 4:
    if (handleTransformTabClick(local_x, local_y))
      return true;
    break;
  }

  if (isMouseOver(mouse_x, mouse_y)) {
    return true;
  }

  return false;
}
