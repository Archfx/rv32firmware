#include <stdint.h>
#include <stdbool.h>


#define reg_uart_data (*(volatile uint32_t*)0x02000008)

void putchar(char c)
{
	if (c == '\n')
		putchar('\r');
	reg_uart_data = c;
}

void print_str(const char *p)
{
	while (*p)
		putchar(*(p++));
}


void main(void)
{
	(*(volatile uint32_t*)0x02000004) = 104; // Set UART clock rate
	print_str("HELLO WORLD\n");	
}
