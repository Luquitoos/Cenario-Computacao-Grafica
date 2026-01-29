#include "../../include/gui/gui_manager.h"
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

void GUIManager::drawRect(int x, int y, int w, int h, float r, float g, float b,
                          float a) {
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

  glColor4f(r, g, b, a);
  glBegin(GL_QUADS);
  glVertex2i(x, y);
  glVertex2i(x + w, y);
  glVertex2i(x + w, y + h);
  glVertex2i(x, y + h);
  glEnd();

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

void GUIManager::drawText(int x, int y, const string &text, float r, float g,
                          float b) {
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

void GUIManager::drawButton(int x, int y, int w, int h, const string &label,
                            bool active) {
  if (active) {
    drawRect(x, y, w, h, 0.3f, 0.5f, 0.7f, 0.95f);
  } else {
    drawRect(x, y, w, h, 0.25f, 0.25f, 0.3f, 0.9f);
  }

  int text_x = x + (w - (int)label.length() * 7) / 2;
  int text_y = y + h / 2 + 4;
  drawText(text_x, text_y, label, 1.0f, 1.0f, 1.0f);
}

void GUIManager::drawSlider(int x, int y, int w, float value, float min_val,
                            float max_val, const string &label) {

  int pad_x = 0;
  if (!label.empty()) {
    drawText(x, y + 12, label, 0.8f, 0.8f, 0.8f);
    pad_x = 80;
  }

  int slider_x = x + pad_x;
  int slider_w = w - pad_x;
  drawRect(slider_x, y, slider_w, 16, 0.2f, 0.2f, 0.25f, 1.0f);

  float normalized = (value - min_val) / (max_val - min_val);
  if (normalized < 0.0f)
    normalized = 0.0f;
  if (normalized > 1.0f)
    normalized = 1.0f;

  int fill_w = (int)(normalized * slider_w);
  drawRect(slider_x, y, fill_w, 16, 0.4f, 0.6f, 0.8f, 1.0f);

  ostringstream ss;
  ss << fixed << setprecision(0) << value;
  drawText(slider_x + slider_w + 5, y + 12, ss.str(), 0.9f, 0.9f, 0.9f);
}
