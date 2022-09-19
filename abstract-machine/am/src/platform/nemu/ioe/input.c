#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000

uint32_t get_keyboard_val() {
  return inl(KBD_ADDR);
}

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t val = inl(KBD_ADDR);
  if(!(val & KEYDOWN_MASK)){
    kbd->keydown = 1;
    kbd->keycode = val;
  }else{
    kbd->keydown = 0;
    kbd->keycode = AM_KEY_NONE; 
  }

}
