#pragma once

#include <Windows.h>

/*
    Thread Hijacking Shell Code (32 Bit)
    __asm
    {
        push EBP
        
        // store all registers
        push EAX
        push ECX
        push EDX
        push EBX
        push ESI

        mov EBP, ESP

        // load address of dll path to EAX
        mov EAX, 0x11223344
        push EAX

        // load address of LoadLibraryW
        mov EBX, 0x11223344

        // Call LoadLibraryW (which is __stdcall)
        call EBX

        // restore all registers
        pop ESI
        pop EBX
        pop EDX
        pop ECX
        pop EAX

        pop EBP

        // return code execution to it's original state
        mov EDI, 0x11223344
        jmp EDI
    }

    Replace address on these indexes:
        DLL Path Address            index: 9  to 12 (including both)
        LoadLibraryW Address        index: 15 to 18 (including both)
        Instruction Pointer Address index: 28 to 31 (including both)
*/
BYTE const shellcode_x32_thread_hijacking[] = { 0x55, 0x50, 0x51, 0x52, 0x53, 0x56, 0x8b, 0xec, 0xb8, 0x44, 0x33, 0x22, 0x11, 0x50, 0xbb, 0x44, 0x33, 0x22, 0x11, 0xff, 0xd3, 0x5e, 0x5b, 0x5a, 0x59, 0x58, 0x5d, 0xbf, 0x44, 0x33, 0x22, 0x11, 0xff, 0xe7 };
BYTE const shellcode_x64_thread_hijacking[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/*
    LdrLoadDll Shellcode (32 Bit)
    __asm
    {
        // Function begining
        push ebp
        mov ebp, esp

        // param 4 - address of handle out
        push 0x11223344

        // param 3 - address of UNICODE_STRING pointer
        push 0x11223344

        // param 2
        push 0

        // param 1
        push 0

        // call LdrLoadDll
        mov edi, 0x11223344
        call edi

        // return EXIT_SUCCESS
        xor eax, eax

        // Function end
        pop ebp
        ret 0x00
    }

    Replace address on these indexes:
        Handle Address                index: 4  to  7 (including both)
        UNICODE_STRING struct Address index: 9  to 12 (including both)
        LdrLoadDll Address            index: 19 to 22 (including both)
*/
BYTE const shellcode_x32_ldrloaddll[] = { 0x55, 0x8b, 0xec, 0x68, 0x44, 0x33, 0x22, 0x11, 0x68, 0x44, 0x33, 0x22, 0x11, 0x6a, 0x00, 0x6a, 0x00, 0xbf, 0x44, 0x33, 0x22, 0x11, 0xff, 0xd7, 0x33, 0xc0, 0x5d, 0xc3 };
