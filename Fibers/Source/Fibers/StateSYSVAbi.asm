%include "Fibers/State.asminc"

%ifdef BUILD_IS_SYSTEM_WINDOWS

GlobalLabel fibers_sysvabi_store ; RCX => state, RDX => returnAddress
	mov rdi, rcx
	mov rsi, rdx

	mov [rdi + RegisterState.m_RBX], rbx
	mov [rdi + RegisterState.m_RBP], rbp
	mov rax, rsp
	add rax, 8h
	mov [rdi + RegisterState.m_RSP], rax
	mov [rdi + RegisterState.m_R12], r12
	mov [rdi + RegisterState.m_R13], r13
	mov [rdi + RegisterState.m_R14], r14
	mov [rdi + RegisterState.m_R15], r15
	cmp rsi, 0h
	je .OrigRIPStore
	mov [rdi + RegisterState.m_RIP], rsi
	jmp .Exit

	.OrigRIPStore:
		mov rax, [rsp]
		mov [rdi + RegisterState.m_RIP], rax

	.Exit:
		ret

GlobalLabel fibers_sysvabi_restore ; RCX => state, RDX => entry
	mov rdi, rcx
	mov rsi, rdx

	mov rbx, [rdi + RegisterState.m_RBX]
	mov rbp, [rdi + RegisterState.m_RBP]
	mov rsp, [rdi + RegisterState.m_RSP]
	mov r12, [rdi + RegisterState.m_R12]
	mov r13, [rdi + RegisterState.m_R13]
	mov r14, [rdi + RegisterState.m_R14]
	mov r15, [rdi + RegisterState.m_R15]
	mov rax, [rdi + RegisterState.m_RIP]
	sub rsp, 8h
	mov [rsp], rax

	cmp rsi, 0h
	je .Exit

	vmovapd xmm0, [rsi + RegisterState.m_XMM0]
	vmovapd xmm1, [rsi + RegisterState.m_XMM1]
	vmovapd xmm2, [rsi + RegisterState.m_XMM2]
	vmovapd xmm3, [rsi + RegisterState.m_XMM3]
	vmovapd xmm4, [rsi + RegisterState.m_XMM4]
	vmovapd xmm5, [rsi + RegisterState.m_XMM5]
	vmovapd xmm6, [rsi + RegisterState.m_XMM6]
	vmovapd xmm7, [rsi + RegisterState.m_XMM7]

	mov rdi, [rsi + RegisterState.m_RDI]
	mov rdx, [rsi + RegisterState.m_RDX]
	mov rcx, [rsi + RegisterState.m_RCX]
	mov r8,  [rsi + RegisterState.m_R8]
	mov r9,  [rsi + RegisterState.m_R9]
	mov rsi, [rsi + RegisterState.m_RSI]

	.Exit:
		ret

%elif BUILD_IS_SYSTEM_UNIX

GlobalLabel fibers_sysvabi_store ; RDI => state, RSI => returnAddress
	mov [rdi + RegisterState.m_RBX], rbx
	mov [rdi + RegisterState.m_RBP], rbp
	mov rax, rsp
	add rax, 8h
	mov [rdi + RegisterState.m_RSP], rax
	mov [rdi + RegisterState.m_R12], r12
	mov [rdi + RegisterState.m_R13], r13
	mov [rdi + RegisterState.m_R14], r14
	mov [rdi + RegisterState.m_R15], r15
	cmp rsi, 0h
	je .OrigRIPStore
	mov [rdi + RegisterState.m_RIP], rsi
	jmp .Exit

	.OrigRIPStore:
		mov rax, [rsp]
		mov [rdi + Registers.m_RIP], rax

	.Exit:
		ret

GlobalLabel fibers_sysvabi_restore ; RDI => state, RSI => entry
	mov rbx, [rdi + RegisterState.m_RBX]
	mov rbp, [rdi + RegisterState.m_RBP]
	mov rsp, [rdi + RegisterState.m_RSP]
	mov r12, [rdi + RegisterState.m_R12]
	mov r13, [rdi + RegisterState.m_R13]
	mov r14, [rdi + RegisterState.m_R14]
	mov r15, [rdi + RegisterState.m_R15]
	mov rax, [rdi + RegisterState.m_RIP]
	sub rsp, 8h
	mov [rsp], rax

	cmp rsi, 0h
	je .Exit

	vmovapd xmm0, [rsi + RegisterState.m_XMM0]
	vmovapd xmm1, [rsi + RegisterState.m_XMM1]
	vmovapd xmm2, [rsi + RegisterState.m_XMM2]
	vmovapd xmm3, [rsi + RegisterState.m_XMM3]
	vmovapd xmm4, [rsi + RegisterState.m_XMM4]
	vmovapd xmm5, [rsi + RegisterState.m_XMM5]
	vmovapd xmm6, [rsi + RegisterState.m_XMM6]
	vmovapd xmm7, [rsi + RegisterState.m_XMM7]

	mov rdi, [rsi + RegisterState.m_RDI]
	mov rdx, [rsi + RegisterState.m_RDX]
	mov rcx, [rsi + RegisterState.m_RCX]
	mov r8,  [rsi + RegisterState.m_R8]
	mov r9,  [rsi + RegisterState.m_R9]
	mov rsi, [rsi + RegisterState.m_RSI]

	.Exit:
		ret

%endif