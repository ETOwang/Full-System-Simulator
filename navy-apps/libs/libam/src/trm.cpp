#include <am.h>
#include <klib.h>
Area heap;

void putch(char ch)
{
    printf("%c", ch);
}

void halt(int code)
{
    //nemu_trap(code);
    // should not reach here
    //while (1)
     //   ;
}