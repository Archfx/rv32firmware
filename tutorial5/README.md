libc to Firmware
========

So far the Firmware that we wrote used raw syntaxes. We didn't use any of the standared c functions like `printf, memcpy, strncpy, etc`. Instead `printf` we directly wrote values to the memory-mapped registers corresponding to the UART interface. In the firmware world, nothing comes easy and you have to earn it. In this tutorial, we look at how to construct your implementations for standard c functions.

For this let's try to write the "Hello World" in a language that everyone can understand.

```c
/* hello.c */
#include <stdio.h>

void main(void)
{
	printf("HELLO WORLD\n");	
}
```
Then using the Linker Script from [tutorial2](), let's try to compile this program. 