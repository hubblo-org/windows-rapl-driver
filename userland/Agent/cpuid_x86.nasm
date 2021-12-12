SECTION .text

global __cpuid__
__cpuid__:
    push esp ; Save pointer to the manufacturer buffer
    pushad ; Save all registers
    xor eax, eax ; Set EAX = 0
    cpuid ; Call CPUID
    pop eax ; Restore pointer to manufacturer buffer
    mov dword [eax], ebx
    mov dword [eax + 4], ecx
    mov dword [eax + 8], edx
    popad ; Restore all registers
    ret
