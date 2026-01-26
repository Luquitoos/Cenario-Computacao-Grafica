#ifndef INPUT_HANDLERS_H
#define INPUT_HANDLERS_H

void keyboard(unsigned char key, int x, int y);
void special_keys(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void reshape(int w, int h);
void display();

void perform_pick(int mouse_x, int mouse_y);

#endif
