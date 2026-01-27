#include "../include/gpu/gpu_renderer.h"
#include "../include/globals.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// Estado do GPU renderer
static bool gpu_available = false;
static bool gpu_initialized = false;
static GLuint compute_program = 0;
static GLuint output_texture = 0;
static GLuint sphere_ssbo = 0;
static GLuint light_ssbo = 0;

// Estruturas para envio à GPU (precisam ser alinhadas para std430)
struct GPU_Sphere {
    float center[3];
    float radius;
    float color[3];
    float padding;
};

struct GPU_Light {
    float position[3];
    float intensity;
    float color[3];
    int light_type; // 0 = directional, 1 = point
};

// Carrega e compila shader de arquivo
static string load_shader_source(const string& path) {
    ifstream file(path);
    if (!file.is_open()) {
        cerr << "[GPU] Erro ao abrir shader: " << path << endl;
        return "";
    }
    stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Compila compute shader
static GLuint compile_compute_shader(const string& source) {
    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        cerr << "[GPU] Erro de compilacao do shader:\n" << log << endl;
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);
    
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, 512, nullptr, log);
        cerr << "[GPU] Erro de linkagem do programa:\n" << log << endl;
        return 0;
    }
    
    glDeleteShader(shader);
    return program;
}

bool gpu_init() {
    if (gpu_initialized) return gpu_available;
    
    // Inicializa GLEW
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "[GPU] Erro ao inicializar GLEW: " << glewGetErrorString(err) << endl;
        gpu_available = false;
        gpu_initialized = true;
        return false;
    }
    
    // Verifica suporte a compute shaders (OpenGL 4.3+)
    if (!GLEW_ARB_compute_shader) {
        cout << "[GPU] Compute shaders nao suportados. Usando CPU.\n";
        gpu_available = false;
        gpu_initialized = true;
        return false;
    }
    
    // Verifica versão OpenGL
    const char* version = (const char*)glGetString(GL_VERSION);
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    cout << "[GPU] OpenGL: " << version << endl;
    cout << "[GPU] Renderer: " << renderer << endl;
    
    // Carrega e compila compute shader
    string shader_source = load_shader_source("shaders/raycast.comp");
    if (shader_source.empty()) {
        gpu_available = false;
        gpu_initialized = true;
        return false;
    }
    
    compute_program = compile_compute_shader(shader_source);
    if (compute_program == 0) {
        gpu_available = false;
        gpu_initialized = true;
        return false;
    }
    
    // Cria textura de saída
    glGenTextures(1, &output_texture);
    glBindTexture(GL_TEXTURE_2D, output_texture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, IMAGE_WIDTH, IMAGE_HEIGHT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    // Cria SSBOs
    glGenBuffers(1, &sphere_ssbo);
    glGenBuffers(1, &light_ssbo);
    
    gpu_available = true;
    gpu_initialized = true;
    cout << "[GPU] Compute shaders inicializados com sucesso!\n";
    return true;
}

bool gpu_is_available() {
    return gpu_available;
}

string gpu_get_info() {
    if (!gpu_initialized) return "GPU nao inicializada";
    if (!gpu_available) return "GPU sem suporte a compute shaders";
    
    const char* renderer = (const char*)glGetString(GL_RENDERER);
    return string(renderer);
}

void gpu_upload_scene() {
    if (!gpu_available) return;
    
    // Coleta esferas da cena
    vector<GPU_Sphere> gpu_spheres;
    
    // Percorre objetos e extrai esferas
    for (const auto& obj : world.objects) {
        // Tenta identificar esferas pelo nome
        string name = obj->get_name();
        // Por simplicidade, vamos criar dados de teste
        // Em implementação real, precisaria extrair dados de cada tipo de objeto
    }
    
    // Dados de teste: 3 esferas
    if (gpu_spheres.empty()) {
        GPU_Sphere s1 = {{0, 0, -5}, 1.0f, {1, 0, 0}, 0};
        GPU_Sphere s2 = {{2, 0, -5}, 1.0f, {0, 1, 0}, 0};
        GPU_Sphere s3 = {{-2, 0, -5}, 1.0f, {0, 0, 1}, 0};
        gpu_spheres.push_back(s1);
        gpu_spheres.push_back(s2);
        gpu_spheres.push_back(s3);
    }
    
    // Upload esferas para GPU
    int count = (int)gpu_spheres.size();
    size_t buffer_size = sizeof(int) * 4 + sizeof(GPU_Sphere) * gpu_spheres.size();
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, sphere_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, buffer_size, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &count);
    if (!gpu_spheres.empty()) {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 4, 
                        sizeof(GPU_Sphere) * gpu_spheres.size(), gpu_spheres.data());
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, sphere_ssbo);
    
    // Upload luzes
    vector<GPU_Light> gpu_lights;
    for (const auto& light : lights) {
        GPU_Light gl;
        // Luz direcional padrão
        gl.position[0] = 1.0f;
        gl.position[1] = 1.0f;
        gl.position[2] = 1.0f;
        gl.intensity = 1.0f;
        gl.color[0] = 1.0f;
        gl.color[1] = 1.0f;
        gl.color[2] = 1.0f;
        gl.light_type = 0;
        gpu_lights.push_back(gl);
    }
    
    if (gpu_lights.empty()) {
        GPU_Light default_light = {{1, 1, 1}, 0.8f, {1, 1, 1}, 0};
        gpu_lights.push_back(default_light);
    }
    
    int light_count = (int)gpu_lights.size();
    size_t light_buffer_size = sizeof(int) * 4 + sizeof(GPU_Light) * gpu_lights.size();
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, light_buffer_size, nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &light_count);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(int) * 4, 
                    sizeof(GPU_Light) * gpu_lights.size(), gpu_lights.data());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, light_ssbo);
}

void gpu_render() {
    if (!gpu_available) return;
    
    glUseProgram(compute_program);
    
    // Set uniforms da câmera
    vec3 cam_lower_left = cam.lower_left_corner;
    vec3 cam_horiz = cam.horizontal;
    vec3 cam_vert = cam.vertical;
    
    glUniform3f(glGetUniformLocation(compute_program, "cam_origin"), 
                cam_eye[0], cam_eye[1], cam_eye[2]);
    glUniform3f(glGetUniformLocation(compute_program, "cam_lower_left"),
                cam_lower_left.x(), cam_lower_left.y(), cam_lower_left.z());
    glUniform3f(glGetUniformLocation(compute_program, "cam_horizontal"),
                cam_horiz.x(), cam_horiz.y(), cam_horiz.z());
    glUniform3f(glGetUniformLocation(compute_program, "cam_vertical"),
                cam_vert.x(), cam_vert.y(), cam_vert.z());
    glUniform1i(glGetUniformLocation(compute_program, "image_width"), IMAGE_WIDTH);
    glUniform1i(glGetUniformLocation(compute_program, "image_height"), IMAGE_HEIGHT);
    
    // Cores do céu
    glUniform3f(glGetUniformLocation(compute_program, "sky_color_top"),
                sky_color_top.r(), sky_color_top.g(), sky_color_top.b());
    glUniform3f(glGetUniformLocation(compute_program, "sky_color_bottom"),
                sky_color_bottom.r(), sky_color_bottom.g(), sky_color_bottom.b());
    
    // Bind textura de saída
    glBindImageTexture(0, output_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);
    
    // Dispatch compute shader
    GLuint groups_x = (IMAGE_WIDTH + 15) / 16;
    GLuint groups_y = (IMAGE_HEIGHT + 15) / 16;
    glDispatchCompute(groups_x, groups_y, 1);
    
    // Aguarda conclusão
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    
    // Copia resultado para PixelBuffer
    glBindTexture(GL_TEXTURE_2D, output_texture);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, PixelBuffer);
    
    cout << "[GPU] Frame renderizado via compute shader\n";
    need_redraw = false;
    frame_cached = true;
}

void gpu_render_preview() {
    // Para preview, ainda usa CPU (mais simples)
    // GPU é melhor para frames completos
}

void gpu_cleanup() {
    if (compute_program) glDeleteProgram(compute_program);
    if (output_texture) glDeleteTextures(1, &output_texture);
    if (sphere_ssbo) glDeleteBuffers(1, &sphere_ssbo);
    if (light_ssbo) glDeleteBuffers(1, &light_ssbo);
    
    compute_program = 0;
    output_texture = 0;
    sphere_ssbo = 0;
    light_ssbo = 0;
    gpu_available = false;
}
