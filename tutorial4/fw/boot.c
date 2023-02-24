/* boot.c */
#include <inttypes.h>
#include "memory_map.h"


int main(void) {
  (*(volatile uint32_t*)0x02000004) = 104; // Set UART clock rate
  (*(volatile uint32_t*)0x02000008) = 0x42; // Write B to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write o to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write o to UART
  (*(volatile uint32_t*)0x02000008) = 0x74; // Write t to UART



   /* Do some bootloader stuff. */
  
  
  uint32_t app_start = (uint32_t)&__approm_start__; // Where is the application
 
  __asm__ ("jal ra, %[prog_count]" /* __asm__ ("jal ra, 0x00104000") */
                    : /* No outputs. */
                    : [prog_count] "i" (app_start) ); // Jump  to the application
 
  while (1) {}
}

