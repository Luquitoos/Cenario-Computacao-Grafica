#include "../../include/gui/gui_manager.h"
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

bool GUIManager::gui_visible = false;
int GUIManager::gui_x = 10;
int GUIManager::gui_y = 10;
int GUIManager::gui_width = 300;
int GUIManager::gui_height = 320;
int GUIManager::current_tab = 0;

string GUIManager::selected_object_name = "";
string GUIManager::selected_material_name = "";
double GUIManager::selected_position[3] = {0, 0, 0};
double GUIManager::selected_normal[3] = {0, 0, 0};
double GUIManager::selected_distance = 0;

double *GUIManager::cam_eye_ptr = nullptr;
double *GUIManager::cam_at_ptr = nullptr;
double *GUIManager::cam_up_ptr = nullptr;
int *GUIManager::projection_type_ptr = nullptr;
bool *GUIManager::need_redraw_ptr = nullptr;

bool *GUIManager::blade_shine_ptr = nullptr;

bool *GUIManager::is_night_mode_ptr = nullptr;

string *GUIManager::selected_transform_name_ptr = nullptr;

double GUIManager::pending_translation[3] = {0, 0, 0};
double GUIManager::pending_rotation[3] = {0, 0, 0};
double GUIManager::pending_scale[3] = {1, 1, 1};
bool GUIManager::has_pending_changes = false;
bool GUIManager::pending_values_loaded = false;

double GUIManager::pending_eye[3] = {0, 0, 0};
double GUIManager::pending_at[3] = {0, 0, 0};
double GUIManager::pending_up[3] = {0, 1, 0};
bool GUIManager::cam_pending_loaded = false;
bool GUIManager::cam_has_pending_changes = false;

function<void()> GUIManager::on_camera_change = nullptr;
function<void()> GUIManager::on_render_request = nullptr;
function<void(bool)> GUIManager::on_blade_shine_toggle = nullptr;
function<void(bool)> GUIManager::on_day_night_toggle = nullptr;
function<void(int)> GUIManager::on_vanishing_point_change = nullptr;
int *GUIManager::vanishing_points_preset_ptr = nullptr;
function<bool(const string &, double *, double *, double *)>
    GUIManager::get_transform_state = nullptr;
function<void(const string &, const double *, const double *, const double *)>
    GUIManager::set_transform_state = nullptr;

void GUIManager::init(double *eye, double *at, double *up, int *proj_type,
                      bool *redraw, bool *blade_shine, bool *is_night,
                      string *sel_trans_name, int *vp_preset) {
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
  vanishing_points_preset_ptr = vp_preset;

  has_pending_changes = false;
  pending_values_loaded = false;

  cam_pending_loaded = false;
  cam_has_pending_changes = false;
}

void GUIManager::setCallbacks(
    function<void()> cam_change, function<void()> render_req,
    function<void(bool)> blade_toggle, function<void(bool)> day_night_toggle,
    function<void(int)> vp_change,
    function<bool(const string &, double *, double *, double *)> get_trans,
    function<void(const string &, const double *, const double *,
                  const double *)>
        set_trans) {
  on_camera_change = cam_change;
  on_render_request = render_req;
  on_blade_shine_toggle = blade_toggle;
  on_day_night_toggle = day_night_toggle;
  on_vanishing_point_change = vp_change;
  get_transform_state = get_trans;
  set_transform_state = set_trans;
}

void GUIManager::show(const string &object_name, const string &material_name,
                      double px, double py, double pz, double nx, double ny,
                      double nz, double distance) {
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
  current_tab = 0;

  has_pending_changes = false;
  pending_values_loaded = false;
}

void GUIManager::hide() { gui_visible = false; }

void GUIManager::toggle() { gui_visible = !gui_visible; }
