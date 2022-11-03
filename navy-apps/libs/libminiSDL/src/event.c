#include <NDL.h>
#include <SDL.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

static uint8_t keyStates[256] = {};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[10];
  int key_val = NDL_PollEvent(buf, 10);
  if(key_val == 0) return 0;
  ev->type = (key_val > 0x7fff) ? 0 : 1;
  ev->key.keysym.sym = key_val & 0x7fff;
	keyStates[ev->key.keysym.sym] = ev->type ? 0 : 1;
  return 1;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[10];
	int key_val;
	do{
  	key_val = NDL_PollEvent(buf, 10);
  	event->type = (key_val > 0x7fff) ? 0 : 1;
  	event->key.keysym.sym = key_val & 0x7fff;
	}while(key_val == 0);
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
	return keyStates;
}
