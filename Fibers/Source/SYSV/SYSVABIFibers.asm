%include "Fibers/Utils/Core.asminc"

struc Registers
	.m_RBX: resq 1
	.m_RBP: resq 1
	.m_RSP: resq 1
	.m_R12: resq 1
	.m_R13: resq 1
	.m_R14: resq 1
	.m_R15: resq 1
	.m_RIP: resq 1
endstruc

struc EntrypointRegisters
	.m_RDI: resq 1
	.m_RSI: resq 1
	.m_RDX: resq 1
	.m_RCX: resq 1
	.m_R8:  resq 1
	.m_R9:  resq 1
	
	.m_XMM0: reso 1
	.m_XMM1: reso 1
	.m_XMM2: reso 1
	.m_XMM3: reso 1
	.m_XMM4: reso 1
	.m_XMM5: reso 1
	.m_XMM6: reso 1
	.m_XMM7: reso 1
endstruc

GlobalLabel sysvabiStoreFiberStates ; RDI => storeRegisters, RSI => returnAddress
	mov [rdi + Registers.m_RBX], rbx
	mov [rdi + Registers.m_RBP], rbp
	mov rax, rsp
	add rax, 8h
	mov [rdi + Registers.m_RSP], rax
	mov [rdi + Registers.m_R12], r12
	mov [rdi + Registers.m_R13], r13
	mov [rdi + Registers.m_R14], r14
	mov [rdi + Registers.m_R15], r15
	cmp rsi, 0h
	je .OrigRIPStore
	mov [rdi + Registers.m_RIP], rsi
	jmp .Exit
	.OrigRIPStore:
		mov rax, [rsp]
		mov [rdi + Registers.m_RIP], rax

	.Exit:
		ret

GlobalLabel sysvabiRestoreFiberStates ; RDI => restoreRegisters, RSI => entrypointRegisters
	mov rbx, [rdi + Registers.m_RBX]
	mov rbp, [rdi + Registers.m_RBP]
	mov rsp, [rdi + Registers.m_RSP]
	mov r12, [rdi + Registers.m_R12]
	mov r13, [rdi + Registers.m_R13]
	mov r14, [rdi + Registers.m_R14]
	mov r15, [rdi + Registers.m_R15]
	mov rax, [rdi + Registers.m_RIP]
	sub rsp, 8h
	mov [rsp], rax

	cmp rsi, 0h
	je .Exit

	vmovapd xmm0, [rsi + EntrypointRegisters.m_XMM0]
	vmovapd xmm1, [rsi + EntrypointRegisters.m_XMM1]
	vmovapd xmm2, [rsi + EntrypointRegisters.m_XMM2]
	vmovapd xmm3, [rsi + EntrypointRegisters.m_XMM3]
	vmovapd xmm4, [rsi + EntrypointRegisters.m_XMM4]
	vmovapd xmm5, [rsi + EntrypointRegisters.m_XMM5]
	vmovapd xmm6, [rsi + EntrypointRegisters.m_XMM6]
	vmovapd xmm7, [rsi + EntrypointRegisters.m_XMM7]

	mov rdi, [rsi + EntrypointRegisters.m_RDI]
	mov rdx, [rsi + EntrypointRegisters.m_RDX]
	mov rcx, [rsi + EntrypointRegisters.m_RCX]
	mov r8,  [rsi + EntrypointRegisters.m_R8]
	mov r9,  [rsi + EntrypointRegisters.m_R9]
	mov rsi, [rsi + EntrypointRegisters.m_RSI]

	.Exit:
		ret

GlobalLabel sysvabiStoreAndRestoreFiberStates ; RDI => storeRegisters, RSI => returnAddress, RDX => restoreRegisters, RCX => entrypointRegisters
	mov [rdi + Registers.m_RBX], rbx
	mov [rdi + Registers.m_RBP], rbp
	mov rax, rsp
	add rax, 8h
	mov [rdi + Registers.m_RSP], rax
	mov [rdi + Registers.m_R12], r12
	mov [rdi + Registers.m_R13], r13
	mov [rdi + Registers.m_R14], r14
	mov [rdi + Registers.m_R15], r15
	cmp rsi, 0h
	je .OrigRIPStore
	mov [rdi + Registers.m_RIP], rsi
	jmp .SetupRegisters
	.OrigRIPStore:
		mov rax, [rsp]
		mov [rdi + Registers.m_RIP], rax

	.SetupRegisters:
		mov rbx, [rdx + Registers.m_RBX]
		mov rbp, [rdx + Registers.m_RBP]
		mov rsp, [rdx + Registers.m_RSP]
		mov r12, [rdx + Registers.m_R12]
		mov r13, [rdx + Registers.m_R13]
		mov r14, [rdx + Registers.m_R14]
		mov r15, [rdx + Registers.m_R15]
		mov rax, [rdx + Registers.m_RIP]
		sub rsp, 8h
		mov [rsp], rax

	cmp rcx, 0h
	je .Exit
	
	vmovapd xmm0, [rcx + EntrypointRegisters.m_XMM0]
	vmovapd xmm1, [rcx + EntrypointRegisters.m_XMM1]
	vmovapd xmm2, [rcx + EntrypointRegisters.m_XMM2]
	vmovapd xmm3, [rcx + EntrypointRegisters.m_XMM3]
	vmovapd xmm4, [rcx + EntrypointRegisters.m_XMM4]
	vmovapd xmm5, [rcx + EntrypointRegisters.m_XMM5]
	vmovapd xmm6, [rcx + EntrypointRegisters.m_XMM6]
	vmovapd xmm7, [rcx + EntrypointRegisters.m_XMM7]

	mov rdi, [rcx + EntrypointRegisters.m_RDI]
	mov rsi, [rcx + EntrypointRegisters.m_RSI]
	mov rdx, [rcx + EntrypointRegisters.m_RDX]
	mov r8,  [rcx + EntrypointRegisters.m_R8]
	mov r9,  [rcx + EntrypointRegisters.m_R9]
	mov rcx, [rcx + EntrypointRegisters.m_RCX]

	.Exit:
		ret
