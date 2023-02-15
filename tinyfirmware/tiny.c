#include <stdint.h>
#include <stdbool.h>

#define OUTPORT 0x10000000

void print_str(const char *p)
{
	while (*p != 0)
		*((volatile uint32_t*)OUTPORT) = *(p++);
}


void main(void)
{
	print_str("hello world\n");
}