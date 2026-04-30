#pragma once
// =============================================================================
// Platform/PlatformGL.h
// OpenGL ES 3.2 compatibility layer for Android.
// On Windows: uses GLEW + OpenGL 2.x (fixed-function)
// On Android: routes fixed-function calls through gl_compat.cpp
// =============================================================================

#if defined(__ANDROID__) || defined(MU_IOS)

#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>
#include <EGL/egl.h>
#include <stdint.h>

// ── GL_Compat must be initialized after context creation ──────────────────
// Call GL_Compat_Init() in android_main.cpp after SDL_GL_CreateContext.
#include "gl_compat.h"

// ── GLEW macros are a no-op on Android ────────────────────────────────────
#define GLEW_OK  0
#define glewInit() (GLEW_OK)
#define glewIsSupported(x) (1)

// ── OpenGL enums not in GLES2 ─────────────────────────────────────────────
#ifndef GL_MODELVIEW
#  define GL_MODELVIEW   0x1700
#endif
#ifndef GL_PROJECTION
#  define GL_PROJECTION  0x1701
#endif
#ifndef GL_MODELVIEW_MATRIX
#  define GL_MODELVIEW_MATRIX  0x0BA6
#endif
#ifndef GL_PROJECTION_MATRIX
#  define GL_PROJECTION_MATRIX 0x0BA7
#endif
#ifndef GL_QUADS
#  define GL_QUADS       0x0007
#endif
#ifndef GL_POLYGON
#  define GL_POLYGON     0x0009
#endif
#ifndef GL_QUAD_STRIP
#  define GL_QUAD_STRIP  0x0008
#endif
#ifndef GL_LINE
#  define GL_LINE  0x1B01
#endif
#ifndef GL_FILL
#  define GL_FILL  0x1B02
#endif
#ifndef GL_POINT
#  define GL_POINT 0x1B00
#endif
#ifndef GL_ADD
#  define GL_ADD  0x0104
#endif
#ifndef GL_CLAMP
#  define GL_CLAMP GL_CLAMP_TO_EDGE
#endif
#ifndef GL_DEPTH_COMPONENT
#  define GL_DEPTH_COMPONENT 0x1902
#endif

// Fixed-function enable/disable constants
#ifndef GL_TEXTURE_2D
#  define GL_TEXTURE_2D      0x0DE1
#endif
#ifndef GL_ALPHA_TEST
#  define GL_ALPHA_TEST      0x0BC0
#endif
#ifndef GL_FOG
#  define GL_FOG             0x0B60
#endif
#ifndef GL_FOG_COLOR
#  define GL_FOG_COLOR       0x0B66
#endif
#ifndef GL_FOG_DENSITY
#  define GL_FOG_DENSITY     0x0B62
#endif
#ifndef GL_FOG_START
#  define GL_FOG_START       0x0B63
#endif
#ifndef GL_FOG_END
#  define GL_FOG_END         0x0B64
#endif
#ifndef GL_FOG_MODE
#  define GL_FOG_MODE        0x0B65
#endif
#ifndef GL_EXP
#  define GL_EXP             0x0800
#endif
#ifndef GL_LIGHTING
#  define GL_LIGHTING        0x0B50
#endif
#ifndef GL_VERTEX_ARRAY
#  define GL_VERTEX_ARRAY    0x8074
#endif
#ifndef GL_TEXTURE_COORD_ARRAY
#  define GL_TEXTURE_COORD_ARRAY 0x8078
#endif
#ifndef GL_COLOR_ARRAY
#  define GL_COLOR_ARRAY     0x8076
#endif
#ifndef GL_NORMAL_ARRAY
#  define GL_NORMAL_ARRAY    0x8075
#endif

// Texture env (not in GLES2)
#ifndef GL_TEXTURE_ENV
#  define GL_TEXTURE_ENV      0x2300
#endif
#ifndef GL_TEXTURE_ENV_MODE
#  define GL_TEXTURE_ENV_MODE 0x2200
#endif
#ifndef GL_MODULATE
#  define GL_MODULATE         0x2100
#endif
#ifndef GL_DECAL
#  define GL_DECAL            0x2101
#endif
#ifndef GL_REPLACE
#  define GL_REPLACE          0x1E01
#endif

// WGL stubs
#define PFNWGLSWAPINTERVALEXTPROC void*
#define wglSwapIntervalEXT nullptr

// ── Route fixed-function calls to gl_compat.cpp ────────────────────────────
// Matrix stack
#define glMatrixMode(m)         GL_MatrixMode(m)
#define glLoadIdentity()        GL_LoadIdentity()
#define glOrtho(l,r,b,t,n,f)   GL_Ortho((double)(l),(double)(r),(double)(b),(double)(t),(double)(n),(double)(f))
#define gluPerspective(fv,a,n,f)  GL_gluPerspective((double)(fv),(double)(a),(double)(n),(double)(f))
#define glFrustum(l,r,b,t,n,f) GL_Frustum((double)(l),(double)(r),(double)(b),(double)(t),(double)(n),(double)(f))
#define gluLookAt(ex,ey,ez,cx,cy,cz,ux,uy,uz) GL_gluLookAt(ex,ey,ez,cx,cy,cz,ux,uy,uz)
#define gluOrtho2D(l,r,b,t)    GL_gluOrtho2D((double)(l),(double)(r),(double)(b),(double)(t))
#define glPushMatrix()          GL_PushMatrix()
#define glPopMatrix()           GL_PopMatrix()
#define glLoadMatrixf(m)        GL_LoadMatrixf(m)
#define glMultMatrixf(m)        GL_MultMatrixf(m)
#define glTranslatef(x,y,z)     GL_Translatef(x,y,z)
#define glRotatef(a,x,y,z)      GL_Rotatef(a,x,y,z)
#define glScalef(x,y,z)         GL_Scalef(x,y,z)
#define glGetFloatv(p,v)        GL_GetFloatv(p,v)

// Immediate mode
#define glBegin(m)              GL_Begin(m)
#define glEnd()                 GL_End()
#define glVertex2f(x,y)         GL_Vertex2f(x,y)
#define glVertex2i(x,y)         GL_Vertex2i(x,y)
#define glVertex3f(x,y,z)       GL_Vertex3f(x,y,z)
#define glVertex3fv(v)          GL_Vertex3fv(v)
#define glTexCoord2f(s,t)       GL_TexCoord2f(s,t)
#define glTexCoord2fv(v)        GL_TexCoord2fv(v)
#define glColor3f(r,g,b)        GL_Color3f(r,g,b)
#define glColor3fv(v)           GL_Color3fv(v)
#define glColor3ub(r,g,b)       GL_Color3ub(r,g,b)
#define glColor4f(r,g,b,a)      GL_Color4f(r,g,b,a)
#define glColor4ub(r,g,b,a)     GL_Color4ub(r,g,b,a)
#define glNormal3f(x,y,z)       GL_Normal3f(x,y,z)
#define glNormal3fv(v)          GL_Normal3fv(v)
#define glRasterPos2f(x,y)      /* no-op */
#define glBitmap(w,h,ox,oy,mx,my,bm) /* no-op */

// Enable/Disable — route through compat for fixed-function states
#define glEnable(cap)           GL_Enable_Compat(cap)
#define glDisable(cap)          GL_Disable_Compat(cap)

// Alpha test
#define glAlphaFunc(f,r)        GL_AlphaFunc(f,r)

// Fog
#define glFogf(p,v)             GL_Fogf(p,v)
#define glFogi(p,v)             GL_Fogi(p,v)
#define glFogfv(p,v)            GL_Fogfv(p,v)

// Lighting
#define glLightfv(l,p,v)        GL_Lightfv(l,p,v)

// Texture env (no-op in GLES2)
#define glTexEnvf(t,p,v)        /* no-op */
#define glTexEnvi(t,p,v)        /* no-op */
#define glTexEnvfv(t,p,v)       /* no-op */

// Polygon mode (no-op in GLES2)
#define glPolygonMode(f,m)      /* no-op */

// Legacy vertex arrays
#define glEnableClientState(a)      GL_EnableClientState(a)
#define glDisableClientState(a)     GL_DisableClientState(a)
#define glVertexPointer(s,t,st,p)   GL_VertexPointer(s,t,st,p)
#define glTexCoordPointer(s,t,st,p) GL_TexCoordPointer(s,t,st,p)
#define glColorPointer(s,t,st,p)    GL_ColorPointer(s,t,st,p)
#define glNormalPointer(t,st,p)     GL_NormalPointer(t,st,p)
#define glInterleavedArrays(f,s,p)  /* no-op */
// Route ALL glDrawArrays through our compat layer so vertex-array-based
// BMD models (ZzzBMD.cpp) are handled by the emulated shader path.
// gl_compat.cpp itself does NOT include PlatformGL.h so its internal
// glDrawArrays calls remain the real GLES2 function.
#define glDrawArrays(m,f,c)         GL_DrawArrays_Compat(m,f,c)
#define glDrawArrays_compat(m,f,c)  GL_DrawArrays_Compat(m,f,c)

// BindTexture tracking
#define glBindTexture(t,n)          GL_TrackBindTexture(t,n)
// Texture upload wrappers: must flush the pending UI batch before mutating
// content of a bound texture (SDL_ttf text re-uploads into the same ID).
#define glTexImage2D(t,l,i,w,h,b,f,y,p)       GL_TexImage2D_Compat(t,l,i,w,h,b,f,y,p)
#define glTexSubImage2D(t,l,x,y,w,h,f,yy,p)   GL_TexSubImage2D_Compat(t,l,x,y,w,h,f,yy,p)
#define glBlendFunc(s,d)            GL_BlendFunc_Compat(s,d)
#define glDepthFunc(f)              GL_DepthFunc_Compat(f)
#define glDepthMask(v)              GL_DepthMask_Compat(v)
#define glColorMask(r,g,b,a)        GL_ColorMask_Compat(r,g,b,a)
#define glClearColor(r,g,b,a)       GL_ClearColor_Compat(r,g,b,a)
#define glClear(mask)               GL_Clear_Compat(mask)

// VSync → use SDL_GL_SetSwapInterval
inline void InitVSync()         {}
inline bool IsVSyncAvailable()  { return true; }
inline bool IsVSyncEnabled()    { return true; }
inline void EnableVSync()       {}
inline void DisableVSync()      {}

// SwapBuffers → handled by SDL_GL_SwapWindow in android_main.cpp
inline void SwapBuffers(HDC) {}

#else
// ── Windows: include GLEW + standard OpenGL ───────────────────────────────
#include <gl/glew.h>
#include <gl/GL.h>
#include <GL/glu.h>
#endif // __ANDROID__
