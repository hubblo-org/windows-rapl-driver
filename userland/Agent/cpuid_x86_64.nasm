SECTION .text

global __cpuid__
__cpuid__:
    push rcx ; Save pointer to the manufacturer buffer
    xor eax, eax ; Set EAX = 0
    cpuid ; Call CPUID
    pop rax ; Restore pointer to manufacturer buffer
    mov dword [rax], ebx
    mov dword [rax + 4], ecx
    mov dword [rax + 8], edx
    ret
