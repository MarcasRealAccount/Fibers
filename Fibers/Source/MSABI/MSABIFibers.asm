%include "Fibers/Utils/Core.asminc"

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

	.m_XMM6:  reso 1
	.m_XMM7:  reso 1
	.m_XMM8:  reso 1
	.m_XMM9:  reso 1
	.m_XMM10: reso 1
	.m_XMM11: reso 1
	.m_XMM12: reso 1
	.m_XMM13: reso 1
	.m_XMM14: reso 1
	.m_XMM15: reso 1
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

GlobalLabel msabiStoreFiberStates ; RCX => storeRegisters, RDX => returnAddress
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

	vmovapd [rcx + Registers.m_XMM6], xmm6
	vmovapd [rcx + Registers.m_XMM7], xmm7
	vmovapd [rcx + Registers.m_XMM8], xmm8
	vmovapd [rcx + Registers.m_XMM9], xmm9
	vmovapd [rcx + Registers.m_XMM10], xmm10
	vmovapd [rcx + Registers.m_XMM11], xmm11
	vmovapd [rcx + Registers.m_XMM12], xmm12
	vmovapd [rcx + Registers.m_XMM13], xmm13
	vmovapd [rcx + Registers.m_XMM14], xmm14
	vmovapd [rcx + Registers.m_XMM15], xmm15

	.Exit:
		ret

GlobalLabel msabiRestoreFiberStates ; RCX => restoreRegisters, RDX => entrypointRegisters
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

	vmovapd xmm6, [rcx + Registers.m_XMM6]
	vmovapd xmm7, [rcx + Registers.m_XMM7]
	vmovapd xmm8, [rcx + Registers.m_XMM8]
	vmovapd xmm9, [rcx + Registers.m_XMM9]
	vmovapd xmm10, [rcx + Registers.m_XMM10]
	vmovapd xmm11, [rcx + Registers.m_XMM11]
	vmovapd xmm12, [rcx + Registers.m_XMM12]
	vmovapd xmm13, [rcx + Registers.m_XMM13]
	vmovapd xmm14, [rcx + Registers.m_XMM14]
	vmovapd xmm15, [rcx + Registers.m_XMM15]

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

GlobalLabel msabiStoreAndRestoreFiberStates ; RCX => storeRegisters, RDX => returnAddress, R8 => restoreRegisters, R9 => entrypointRegisters
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
		
	vmovapd [rcx + Registers.m_XMM6], xmm6
	vmovapd [rcx + Registers.m_XMM7], xmm7
	vmovapd [rcx + Registers.m_XMM8], xmm8
	vmovapd [rcx + Registers.m_XMM9], xmm9
	vmovapd [rcx + Registers.m_XMM10], xmm10
	vmovapd [rcx + Registers.m_XMM11], xmm11
	vmovapd [rcx + Registers.m_XMM12], xmm12
	vmovapd [rcx + Registers.m_XMM13], xmm13
	vmovapd [rcx + Registers.m_XMM14], xmm14
	vmovapd [rcx + Registers.m_XMM15], xmm15

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

		vmovapd xmm6, [rcx + Registers.m_XMM6]
		vmovapd xmm7, [rcx + Registers.m_XMM7]
		vmovapd xmm8, [rcx + Registers.m_XMM8]
		vmovapd xmm9, [rcx + Registers.m_XMM9]
		vmovapd xmm10, [rcx + Registers.m_XMM10]
		vmovapd xmm11, [rcx + Registers.m_XMM11]
		vmovapd xmm12, [rcx + Registers.m_XMM12]
		vmovapd xmm13, [rcx + Registers.m_XMM13]
		vmovapd xmm14, [rcx + Registers.m_XMM14]
		vmovapd xmm15, [rcx + Registers.m_XMM15]

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
