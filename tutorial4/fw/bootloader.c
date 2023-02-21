/* bootloader.c */
#include <inttypes.h>
#include "memory_map.h"

static void start_app(uint32_t pc, uint32_t sp) __attribute__((naked));

static void start_app(uint32_t pc, uint32_t sp)  {
    __asm("           \n\
          la x2, a1 /* load a1 into SP */\n\
          jr a0       /* jump to the address at a0 */\n\
    ");
}

int main(void) {
  uint32_t *app_code = (uint32_t *)__approm_start__;
  uint32_t app_sp = app_code[0];
  uint32_t app_start = app_code[1];
  start_app(app_start, app_sp);
  /* Not Reached */
  while (1) {}
}