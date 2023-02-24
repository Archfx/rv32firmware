/* bootloader.c */
#include <inttypes.h>
#include "memory_map.h"

static void start_app(uint32_t pc) __attribute__((naked));

static void start_app(uint32_t pc)  {

    // (*(volatile uint32_t*)0x02000008) = pc;
    __asm__ ("jal ra, %A[pc]" : [pc] "+r" (pc));

}



int main(void) {
  (*(volatile uint32_t*)0x02000004) = 104; // Set UART clock rate
  (*(volatile uint32_t*)0x02000008) = 0x42; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x74; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x3A; // Write something to UART


  // uint32_t *app_code = (uint32_t *)__approm_start__;

  uint32_t app_start = (uint32_t)&__approm_start__; //working

  // uint32_t app_start_1 = 0x00104000;

  // uint8_t *arr = (uint8_t *)&app_code;
  // uint32_t app_sp = app_code[0];
  // uint32_t app_start = app_code[1];

  // (*(volatile uint32_t*)0x02000008) = app_sp;
  // (*(volatile uint32_t*)0x02000008) = app_start_1;

  //  (*(volatile uint32_t*)0x02000008) = arr[0];
  //  (*(volatile uint32_t*)0x02000008) = arr[1];
  //  (*(volatile uint32_t*)0x02000008) = arr[2];
  //  (*(volatile uint32_t*)0x02000008) = arr[3];

  // (*(volatile uint32_t*)0x02000008) = 0xff00ff00;
  // (*(volatile uint32_t*)0x02000008) = 0x00104000;
  // (*(volatile uint32_t*)0x02000008) = __approm_start__;
  // (*(volatile uint32_t*)0x02000008) = __bootrom_start__;
  // (*(volatile uint32_t*)0x02000008) = app_code[0];
  // (*(volatile uint32_t*)0x02000008) = app_code[1];
  
  
  // __asm__ volatile ("la x2, %0" : "=r"(app_sp));
  // __asm__ volatile ("jr %0" : "=r"(app_start));

  // __asm__ volatile ("la x2, 0x41");
  // __asm__ volatile ("jr 0x00");

  // __asm ("lui sp, %0" : "=r"(app_sp));
  // __asm ("jal ra, %0" : "=r"(app_code));

  // __asm ("lui sp, %0" : "=r"(arr[0]));
  // __asm ("jal ra, %0" : "=r"(arr[1]));

  // __asm__ ("lui sp, %0" : "=r"(app_code));
  // __asm__ ("jal ra, %0" : "=r"(app_start_1));
 
  // __asm__ ("lui sp, 0x104000");
  __asm__ ("jal ra, 0x00104000"); /// Working

  // __asm__ volatile ("lui sp, 0x11" );
  // __asm__ volatile ("jal ra, 0x41" );

  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write something to UART
  (*(volatile uint32_t*)0x02000008) = 0x6F; // Write something to UART

  // start_app(app_start, app_sp);
  start_app(app_start);
  /* Not Reached */
  while (1) {}
}