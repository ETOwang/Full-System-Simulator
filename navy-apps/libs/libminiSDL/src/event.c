#include <NDL.h>
#include <SDL.h>
#include <string.h>
#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  assert(0);

  return 0;
}

uint8_t key_state[sizeof(keyname)/sizeof(keyname[0])] = {0};

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if (NDL_PollEvent(buf, sizeof(buf)) == 0) {
    return 0;
  }
  buf[strlen(buf)-1]='\0';
  int keydown = (buf[1] == 'd');
  char* key = buf+3;
  ev->type = keydown ? SDL_KEYDOWN : SDL_KEYUP;
  for (size_t i = 0; i <sizeof(keyname)/sizeof(keyname[0]); i++)
  {
    if(strcmp(keyname[i], key) == 0)
    {
      ev->key.keysym.sym = i;
      key_state[i] = keydown;
      assert(keydown==0||keydown==1);
      break;
    }
  }
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  while (SDL_PollEvent(event) == 0);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert(0);

  return 0;
}
uint8_t* SDL_GetKeyState(int *numkeys) {
  if (numkeys) {
    *numkeys = sizeof(key_state);
  }
  return key_state;
}
