#include <stdint.h>
#include <stdio.h>
#include <string.h>

static void __attribute__ ((naked)) __cpuid__( void );

static uint32_t _eax, _ebx, _ecx, _edx;

int main( void )
{
    char manufacturer[13];

    memset(manufacturer, 0, sizeof(manufacturer));
    __cpuid__();
    memcpy(manufacturer, &_ebx, sizeof(uint32_t));
    memcpy(manufacturer + sizeof(uint32_t), &_edx, sizeof(uint32_t));
    memcpy(manufacturer + 2 * sizeof(uint32_t), &_ecx, sizeof(uint32_t));

    printf("CPU manufacturer: %s\n", manufacturer);

    return 0;
}

void __cpuid__( void )
{
    asm("xor eax, eax");
    asm("cpuid");
    asm("mov _eax, eax");
    asm("mov _ebx, ebx");
    asm("mov _ecx, ecx");
    asm("mov _edx, edx");
    asm("ret");
}
