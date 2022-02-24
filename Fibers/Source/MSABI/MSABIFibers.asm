%include "Fibers/Utils/Core.asminc"

%if BUILD_IS_SYSTEM_WINDOWS

struc Registers
	.m_RBX: resq 1
	.m_RDI: resq 1
	.m_RSI: resq 1
	.m_RBP: resq 1
	.m_RSP: resq 1
	.m_R12: resq 1
	.m_R13: resq 1
	.m_R14: resq 1
	.m_R15: resq 1
	.m_RIP: resq 1
endstruc

struc EntrypointRegisters
	.m_RCX: resq 1
	.m_RDX: resq 1
	.m_R8:  resq 1
	.m_R9:  resq 1

	.m_XMM0: reso 1
	.m_XMM1: reso 1
	.m_XMM2: reso 1
	.m_XMM3: reso 1
endstruc

GlobalLabel storeFiberStates ; RCX => storeRegisters, RDX => returnAddress
	mov [rcx + Registers.m_RBX], rbx
	mov [rcx + Registers.m_RDI], rdi
	mov [rcx + Registers.m_RSI], rsi
	mov [rcx + Registers.m_RBP], rbp
	mov rax, rsp
	add rax, 8h
	mov [rcx + Registers.m_RSP], rax
	mov [rcx + Registers.m_R12], r12
	mov [rcx + Registers.m_R13], r13
	mov [rcx + Registers.m_R14], r14
	mov [rcx + Registers.m_R15], r15
	cmp rdx, 0h
	je .OrigRIPStore
	mov [rcx + Registers.m_RIP], rdx
	jmp .Exit
	.OrigRIPStore:
		mov rax, [rsp]
		mov [rcx + Registers.m_RIP], rax

	.Exit:
		ret

GlobalLabel restoreFiberStates ; RCX => restoreRegisters, RDX => entrypointRegisters
	mov rbx, [rcx + Registers.m_RBX]
	mov rdi, [rcx + Registers.m_RDI]
	mov rsi, [rcx + Registers.m_RSI]
	mov rbp, [rcx + Registers.m_RBP]
	mov rsp, [rcx + Registers.m_RSP]
	mov r12, [rcx + Registers.m_R12]
	mov r13, [rcx + Registers.m_R13]
	mov r14, [rcx + Registers.m_R14]
	mov r15, [rcx + Registers.m_R15]
	mov rax, [rcx + Registers.m_RIP]
	sub rsp, 8h
	mov [rsp], rax

	cmp rdx, 0h
	je .Exit

	vmovapd xmm0, [rdx + EntrypointRegisters.m_XMM0]
	vmovapd xmm1, [rdx + EntrypointRegisters.m_XMM1]
	vmovapd xmm2, [rdx + EntrypointRegisters.m_XMM2]
	vmovapd xmm3, [rdx + EntrypointRegisters.m_XMM3]

	mov rcx, [rdx + EntrypointRegisters.m_RCX]
	mov r8,  [rdx + EntrypointRegisters.m_R8]
	mov r9,  [rdx + EntrypointRegisters.m_R9]
	mov rdx, [rdx + EntrypointRegisters.m_RDX]

	.Exit:
		ret

GlobalLabel storeAndRestoreFiberStates ; RCX => storeRegisters, RDX => returnAddress, R8 => restoreRegisters, R9 => entrypointRegisters
	mov [rcx + Registers.m_RBX], rbx
	mov [rcx + Registers.m_RDI], rdi
	mov [rcx + Registers.m_RSI], rsi
	mov [rcx + Registers.m_RBP], rbp
	mov rax, rsp
	add rax, 8h
	mov [rcx + Registers.m_RSP], rax
	mov [rcx + Registers.m_R12], r12
	mov [rcx + Registers.m_R13], r13
	mov [rcx + Registers.m_R14], r14
	mov [rcx + Registers.m_R15], r15
	cmp rdx, 0h
	je .OrigRIPStore
	mov [rcx + Registers.m_RIP], rdx
	jmp .SetupRegisters
	.OrigRIPStore:
		mov rax, [rsp]
		mov [rcx + Registers.m_RIP], rax

	.SetupRegisters:
		mov rbx, [r8 + Registers.m_RBX]
		mov rdi, [r8 + Registers.m_RDI]
		mov rsi, [r8 + Registers.m_RSI]
		mov rbp, [r8 + Registers.m_RBP]
		mov rsp, [r8 + Registers.m_RSP]
		mov r12, [r8 + Registers.m_R12]
		mov r13, [r8 + Registers.m_R13]
		mov r14, [r8 + Registers.m_R14]
		mov r15, [r8 + Registers.m_R15]
		mov rax, [r8 + Registers.m_RIP]
		sub rsp, 8h
		mov [rsp], rax

	cmp r9, 0h
	je .Exit

	vmovapd xmm0, [r9 + EntrypointRegisters.m_XMM0]
	vmovapd xmm1, [r9 + EntrypointRegisters.m_XMM1]
	vmovapd xmm2, [r9 + EntrypointRegisters.m_XMM2]
	vmovapd xmm3, [r9 + EntrypointRegisters.m_XMM3]

	mov rcx, [r9 + EntrypointRegisters.m_RCX]
	mov rdx, [r9 + EntrypointRegisters.m_RDX]
	mov r8,  [r9 + EntrypointRegisters.m_R8]
	mov r9,  [r9 + EntrypointRegisters.m_R9]

	.Exit:
		ret

%endif