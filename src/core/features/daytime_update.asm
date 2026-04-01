option casemap:none

; From https://www.nexusmods.com/eldenring/mods/48
PUBLIC DaytimeUpdate

extern returnAddress    : QWORD
extern funcAddress      : QWORD
extern freeze_time_day  : BYTE
extern set_morning      : BYTE
extern cycleSpeed       : DWORD

.code
DaytimeUpdate PROC
    cvttss2si edx, xmm0                 ; orig

    cmp byte ptr [freeze_time_day], 1
    jne skip_freeze

    xor edx, edx

skip_freeze:
    cmp byte ptr [set_morning], 1
    jne cont

    mov edx, cycleSpeed

cont:
    test edx, edx                       ; orig
    jle skip_call                       ; orig
    mov rcx, rbx                        ; orig
    call qword ptr [funcAddress]        ; orig

skip_call:
    jmp qword ptr [returnAddress]
DaytimeUpdate ENDP
end


COMMENT @
    eldenring.exe.text+622181: 48 8D 94 24 80 00 00 00  - lea rdx,[rsp+00000080]
    eldenring.exe.text+622189: 48 8B CF                 - mov rcx,rdi
    eldenring.exe.text+62218C: E8 CF 17 F9 FF           - call eldenring.exe.text+5B3960
    eldenring.exe.text+622191: 44 38 73 46              - cmp [rbx+46],r14l
    eldenring.exe.text+622195: 74 25                    - je eldenring.exe.text+6221BC
    eldenring.exe.text+622197: 4C 39 33                 - cmp [rbx],r14
    eldenring.exe.text+62219A: 74 20                    - je eldenring.exe.text+6221BC
    eldenring.exe.text+62219C: 0F 28 C6                 - movaps xmm0,xmm6
    eldenring.exe.text+62219F: F3 0F 59 43 38           - mulss xmm0,[rbx+38]
    eldenring.exe.text+6221A4: F3 0F 59 05 94 BF BA 02  - mulss xmm0,[eldenring.exe.rdata+8DF140]
    // ---------- INJECTING HERE ----------
    eldenring.exe.text+6221AC: F3 0F 2C D0              - cvttss2si edx,xmm0
    eldenring.exe.text+6221B0: 85 D2                    - test edx,edx
    eldenring.exe.text+6221B2: 7E 08                    - jle eldenring.exe.text+6221BC
    eldenring.exe.text+6221B4: 48 8B CB                 - mov rcx,rbx
    eldenring.exe.text+6221B7: E8 94 F6 FF FF           - call eldenring.exe.text+621850    ; funcAddress
    // ---------- DONE INJECTING  ----------
    eldenring.exe.text+6221BC: 40 32 F6                 - xor sil,sil
    eldenring.exe.text+6221BF: 83 7B 28 17              - cmp dword ptr [rbx+28],17
    eldenring.exe.text+6221C3: 0F 87 00 01 00 00        - ja eldenring.exe.text+6222C9
    eldenring.exe.text+6221C9: 83 7B 2C 3B              - cmp dword ptr [rbx+2C],3B
    eldenring.exe.text+6221CD: 0F 87 F6 00 00 00        - ja eldenring.exe.text+6222C9
    eldenring.exe.text+6221D3: 83 7B 30 3B              - cmp dword ptr [rbx+30],3B
@