/*
  SDL_ttf.h — Minimal declaration for SDL2_ttf v2.x
  This is a hand-written header to expose the SDL_ttf API without the full source.
*/
#pragma once
#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _TTF_Font TTF_Font;

#define TTF_STYLE_NORMAL        0x00
#define TTF_STYLE_BOLD          0x01
#define TTF_STYLE_ITALIC        0x02
#define TTF_STYLE_UNDERLINE     0x04
#define TTF_STYLE_STRIKETHROUGH 0x08

extern DECLSPEC int  SDLCALL TTF_Init(void);
extern DECLSPEC void SDLCALL TTF_Quit(void);

extern DECLSPEC TTF_Font* SDLCALL TTF_OpenFont(const char *file, int ptsize);
extern DECLSPEC TTF_Font* SDLCALL TTF_OpenFontRW(SDL_RWops *src, int freesrc, int ptsize);
extern DECLSPEC void      SDLCALL TTF_CloseFont(TTF_Font *font);

extern DECLSPEC void SDLCALL TTF_SetFontStyle(TTF_Font *font, int style);
extern DECLSPEC int  SDLCALL TTF_GetFontStyle(const TTF_Font *font);

/* Measure text */
extern DECLSPEC int SDLCALL TTF_SizeUTF8(TTF_Font *font, const char *text, int *w, int *h);
extern DECLSPEC int SDLCALL TTF_SizeUNICODE(TTF_Font *font, const Uint16 *text, int *w, int *h);

/* Render text (returns SDL_Surface owned by caller) */
extern DECLSPEC SDL_Surface* SDLCALL TTF_RenderUTF8_Blended(TTF_Font *font, const char *text, SDL_Color fg);
extern DECLSPEC SDL_Surface* SDLCALL TTF_RenderUNICODE_Blended(TTF_Font *font, const Uint16 *text, SDL_Color fg);
extern DECLSPEC SDL_Surface* SDLCALL TTF_RenderUTF8_Solid(TTF_Font *font, const char *text, SDL_Color fg);

/* Font metrics */
extern DECLSPEC int SDLCALL TTF_FontHeight(const TTF_Font *font);
extern DECLSPEC int SDLCALL TTF_FontAscent(const TTF_Font *font);
extern DECLSPEC int SDLCALL TTF_FontDescent(const TTF_Font *font);
extern DECLSPEC int SDLCALL TTF_FontLineSkip(const TTF_Font *font);

#ifdef __cplusplus
}
#endif
