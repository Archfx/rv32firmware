/* bootloader.c */
#include <inttypes.h>
#include "memory_map.h"

// static void start_app(uint32_t pc, uint32_t sp) __attribute__((naked));

// static void start_app(uint32_t pc, uint32_t sp)  {
//     // __asm("           \n\
//     //       la x2, a1 /* load a1 into SP */\n\
//     //       jr a0       /* jump to the address at a0 */\n\
//     // ");
//     // (*(volatile uint32_t*)0x02000004) = 104; // Set UART clock rate
//     // (*(volatile uint32_t*)0x02000008) = 0x66;
//     __asm ("la x2, %0\n\t" /* load app_sp into SP */
//     "jr %1"  /* jump to the app_code */
//     : "=r" (pc) 
//     : "r" (sp));


// }



int main(void) {
  (*(volatile uint32_t*)0x02000004) = 104; // Set UART clock rate
  (*(volatile uint32_t*)0x02000008) = 0x42; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x74; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x3A; // Write something to UART


  uint32_t *app_code = (uint32_t *)__approm_start__;

  uint8_t *arr = (uint8_t *)&app_code;
  uint32_t app_sp = app_code[0];
  uint32_t app_start = app_code[1];

  // (*(volatile uint32_t*)0x02000008) = app_sp;
  // (*(volatile uint32_t*)0x02000008) = app_start;

   (*(volatile uint32_t*)0x02000008) = arr[0];
   (*(volatile uint32_t*)0x02000008) = arr[1];
   (*(volatile uint32_t*)0x02000008) = arr[2];
   (*(volatile uint32_t*)0x02000008) = arr[3];

  (*(volatile uint32_t*)0x02000008) = 0xff00ff00;
  (*(volatile uint32_t*)0x02000008) = 0x00104000;
  (*(volatile uint32_t*)0x02000008) = __approm_start__;
  (*(volatile uint32_t*)0x02000008) = __bootrom_start__;
  (*(volatile uint32_t*)0x02000008) = app_code[0];
  (*(volatile uint32_t*)0x02000008) = app_code[1];
  
  (*(volatile uint32_t*)0x02000008) = 0x74; // Write something to UART
  
  // __asm__ volatile ("la x2, %0" : "=r"(app_sp));
  // __asm__ volatile ("jr %0" : "=r"(0x00104000));

  __asm__ volatile ("la x2, 0x10413e");
  __asm__ volatile ("j 0x10413e");


  
  // start_app(app_start, app_sp);
  /* Not Reached */
  while (1) {}
}