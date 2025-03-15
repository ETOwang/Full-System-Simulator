#include <NDL.h>
#include <SDL.h>
int SDL_Init(uint32_t flags) {
  return NDL_Init(flags);
}

void SDL_Quit() {
  NDL_Quit();
}

char *SDL_GetError() {
  return "Navy does not support SDL_GetError()";
}

int SDL_SetError(const char* fmt, ...) {
  return -1;
}

int SDL_ShowCursor(int toggle) {
  return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
  // This function is supposed to set the window caption and icon.
  // Since this is a minimal SDL implementation, we will not support this feature.
  // Therefore, we will leave it as an assertion failure.
  
}
