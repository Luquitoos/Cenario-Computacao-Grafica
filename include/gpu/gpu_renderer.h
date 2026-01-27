#ifndef GPU_RENDERER_H
#define GPU_RENDERER_H

#include <string>

// Verifica se GPU com compute shaders está disponível
bool gpu_init();
bool gpu_is_available();

// Envia dados da cena para GPU
void gpu_upload_scene();

// Renderiza usando compute shaders
void gpu_render();
void gpu_render_preview();

// Libera recursos GPU
void gpu_cleanup();

// Info da GPU
std::string gpu_get_info();

#endif
