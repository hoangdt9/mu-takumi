#pragma once
// =============================================================================
// Platform/gl_compat.h
// OpenGL fixed-function emulation on top of OpenGL ES 3.2 for Android.
// Provides: immediate mode (glBegin/glEnd), matrix stack, color state.
// Linked via gl_compat.cpp — NOT header-only.
// =============================================================================

#if defined(__ANDROID__) || defined(MU_IOS)

#include <GLES3/gl32.h>
#include <stdint.h>

// ── Must call once after GL context is created ─────────────────────────────
void GL_Compat_Init();
void GL_Compat_Shutdown();

// ── glBindTexture wrapper — tracks currently bound texture for shader ───────
void GL_TrackBindTexture(GLenum target, GLuint texture);
// Upload wrappers — must flush pending batch before mutating texture content,
// otherwise batched UI quads end up sampling the post-upload pixels (bug:
// all font glyphs rendered as the last-uploaded string).
void GL_TexImage2D_Compat(GLenum target, GLint level, GLint internalformat,
                          GLsizei width, GLsizei height, GLint border,
                          GLenum format, GLenum type, const void* pixels);
void GL_TexSubImage2D_Compat(GLenum target, GLint level,
                             GLint xoffset, GLint yoffset,
                             GLsizei width, GLsizei height,
                             GLenum format, GLenum type, const void* pixels);
void GL_BlendFunc_Compat(GLenum sfactor, GLenum dfactor);
void GL_DepthFunc_Compat(GLenum func);
void GL_DepthMask_Compat(GLboolean flag);
void GL_ColorMask_Compat(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
void GL_ClearColor_Compat(float red, float green, float blue, float alpha);
void GL_Clear_Compat(GLbitfield mask);
void GL_FlushPending();

// ── Matrix stack ───────────────────────────────────────────────────────────
void GL_MatrixMode(GLenum mode);         // GL_MODELVIEW or GL_PROJECTION
void GL_LoadIdentity();
void GL_Ortho(double l, double r, double b, double t, double n, double f);
void GL_Frustum(double l, double r, double b, double t, double n, double f);
void GL_gluPerspective(double fovy, double aspect, double zNear, double zFar);
void GL_gluLookAt(double ex,double ey,double ez,
                  double cx,double cy,double cz,
                  double ux,double uy,double uz);
void GL_gluOrtho2D(double l,double r,double b,double t);
void GL_PushMatrix();
void GL_PopMatrix();
void GL_LoadMatrixf(const float* m);
void GL_MultMatrixf(const float* m);
void GL_Translatef(float x, float y, float z);
void GL_Rotatef(float angle, float x, float y, float z);
void GL_Scalef(float x, float y, float z);
void GL_GetFloatv(GLenum pname, float* params);  // supports GL_MODELVIEW_MATRIX / GL_PROJECTION_MATRIX

// ── Immediate mode ─────────────────────────────────────────────────────────
void GL_Begin(GLenum mode);  // GL_TRIANGLES, GL_QUADS, GL_TRIANGLE_FAN, GL_LINES ...
void GL_End();
void GL_Vertex2f(float x, float y);
void GL_Vertex2i(int x, int y);
void GL_Vertex3f(float x, float y, float z);
void GL_Vertex3fv(const float* v);
void GL_TexCoord2f(float s, float t);
void GL_TexCoord2fv(const float* v);
void GL_Color3f(float r, float g, float b);
void GL_Color3fv(const float* v);
void GL_Color3ub(uint8_t r, uint8_t g, uint8_t b);
void GL_Color4f(float r, float g, float b, float a);
void GL_Color4ub(uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void GL_Normal3f(float x, float y, float z);
void GL_Normal3fv(const float* v);

// ── Bulk draw (bypasses immediate mode for max throughput) ─────────────────
// Vertex layout: x,y,z, r,g,b,a, u,v  (9 floats per vertex, 4 verts per quad)
// Draws quadCount quads with indexed triangles in ONE draw call.
void GL_DrawQuadsBulk(const float* vertexData, int quadCount);
// Same but for triangles (3 verts per tri, no index buffer)
void GL_DrawTrisBulk(const float* vertexData, int triCount);

// ── Debug stats ───────────────────────────────────────────────────────────
void GL_GetDrawStats(int* drawCalls, int* vertices);  // returns counts since last reset
void GL_ResetDrawStats();
void GL_GetDrawPathStats(int* imDrawCalls,
                         int* vaDirectDrawCalls,
                         int* vaConvertedDrawCalls,
                         int* quadIndexedDrawCalls,
                         int* quadExpandedDrawCalls);

// Runtime toggle for legacy VA draw path:
// true  -> prefer direct client-array path when compatible
// false -> force converted/batched VBO upload path
void GL_SetPreferDirectVertexArrays(bool preferDirect);
bool GL_GetPreferDirectVertexArrays();

// Skip VBO buffer-orphaning (glBufferData nullptr) on software renderers like
// SwiftShader where there is no real GPU pipeline stall to avoid.
// Eliminates ~105 unnecessary driver-level malloc calls per frame on emulator.
void GL_SetSkipVBOOrphan(bool skip);

// ── Enable/Disable for fixed-function states ───────────────────────────────
void GL_Enable_Compat(GLenum cap);
void GL_Disable_Compat(GLenum cap);

// ── Alpha test emulation (done in shader) ─────────────────────────────────
void GL_AlphaFunc(GLenum func, float ref);

// ── Fog emulation (done in shader) ────────────────────────────────────────
void GL_Fogf(GLenum pname, float param);
void GL_Fogi(GLenum pname, int param);
void GL_Fogfv(GLenum pname, const float* params);

// ── Lighting (stub — complex shading not implemented) ─────────────────────
void GL_Lightfv(GLenum light, GLenum pname, const float* params);

// ── Vertex arrays (fixed-function legacy) ─────────────────────────────────
void GL_EnableClientState(GLenum array);
void GL_DisableClientState(GLenum array);
void GL_VertexPointer(int size, GLenum type, int stride, const void* ptr);
void GL_TexCoordPointer(int size, GLenum type, int stride, const void* ptr);
void GL_ColorPointer(int size, GLenum type, int stride, const void* ptr);
void GL_NormalPointer(GLenum type, int stride, const void* ptr);
void GL_DrawArrays_Compat(GLenum mode, int first, int count);

// ── Direct batch append (high-perf path for BMD mesh rendering) ───────────
// Appends triangles DIRECTLY into the pending vertex batch, bypassing the
// intermediate s_arrayVerts copy step in GL_DrawArrays_Compat.
// This is a single-pass combine of the RenderMesh arrays into IMVertex format
// with ModelView baked in — eliminates one full memory-copy pass per mesh.
//
// positions : numVerts × 3 floats — world-space XYZ (ModelView will be baked)
// colors    : numVerts × 4 floats — RGBA, or nullptr to use current GL color
// texcoords : numVerts × 2 floats — UV
// numVerts  : total vertex count (must be a multiple of 3 for GL_TRIANGLES)
void GL_BatchAppendTriangles(const float* positions,
                             const float* colors,
                             const float* texcoords,
                             int numVerts);

#endif // __ANDROID__
