#ifndef GL_LOADER_H
#define GL_LOADER_H

#include <GL/freeglut.h>
#include <string>
#include <iostream>
#include <cstddef>

// Definições necessárias para Compute Shaders e SSBOs
#define GL_COMPUTE_SHADER 0x91B9
#define GL_SHADER_STORAGE_BUFFER 0x90D2
#define GL_READ_ONLY 0x88B8
#define GL_WRITE_ONLY 0x88B9
#define GL_READ_WRITE 0x88BA
#define GL_RGBA8 0x8058
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_NEAREST 0x2600
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT 0x00000020
#define GL_LINK_STATUS 0x8B82
#define GL_COMPILE_STATUS 0x8B81
#define GL_INFO_LOG_LENGTH 0x8B84

// Tipos requeridos
typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

// Tipos de funo OpenGL
typedef GLuint (APIENTRY *PFNGLCREATESHADERPROC) (GLenum type);
typedef void (APIENTRY *PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const char* const* string, const GLint* length);
typedef void (APIENTRY *PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef GLuint (APIENTRY *PFNGLCREATEPROGRAMPROC) (void);
typedef void (APIENTRY *PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRY *PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLDISPATCHCOMPUTEPROC) (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void (APIENTRY *PFNGLBINDIMAGETEXTUREPROC) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (APIENTRY *PFNGLMEMORYBARRIERPROC) (GLbitfield barriers);
typedef void (APIENTRY *PFNGLGENTEXTURESPROC) (GLsizei n, GLuint* textures);
typedef void (APIENTRY *PFNGLBINDTEXTUREPROC) (GLenum target, GLuint texture);
typedef void (APIENTRY *PFNGLTEXSTORAGE2DPROC) (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRY *PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (APIENTRY *PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRY *PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const void* data, GLenum usage);
typedef void (APIENTRY *PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const void* data);
typedef void (APIENTRY *PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef void (APIENTRY *PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* params);
typedef void (APIENTRY *PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* params);
typedef void (APIENTRY *PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef void (APIENTRY *PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei* length, char* infoLog);
typedef void (APIENTRY *PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRY *PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRY *PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (APIENTRY *PFNGLUNIFORM3FPROC) (GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef GLint (APIENTRY *PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const char* name);
// glDeleteTextures ja existe no GL.h padrao do Windows
typedef void (APIENTRY *PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);

extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLDISPATCHCOMPUTEPROC glDispatchCompute;
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
extern PFNGLMEMORYBARRIERPROC glMemoryBarrier;
extern PFNGLTEXSTORAGE2DPROC glTexStorage2D;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM3FPROC glUniform3f;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;

// Funo para carregar extenses
bool load_gl_extensions();

#endif
