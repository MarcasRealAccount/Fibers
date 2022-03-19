%include "Fibers/State.asminc"

%if BUILD_IS_SYSTEM_WINDOWS

GlobalLabel fibers_msabi_store ; RCX => state, RDX => returnAddress
	mov [rcx + RegisterState.m_RBX], rbx
	mov [rcx + RegisterState.m_RDI], rdi
	mov [rcx + RegisterState.m_RSI], rsi
	mov [rcx + RegisterState.m_RBP], rbp
	mov rax, rsp
	add rax, 8h
	mov [rcx + RegisterState.m_RSP], rax
	mov [rcx + RegisterState.m_R12], r12
	mov [rcx + RegisterState.m_R13], r13
	mov [rcx + RegisterState.m_R14], r14
	mov [rcx + RegisterState.m_R15], r15
	cmp rdx, 0h
	je .OrigRIPStore
	mov [rcx + RegisterState.m_RIP], rdx
	jmp .Vectors
	
	.OrigRIPStore:
		mov rax, [rsp]
		mov [rcx + RegisterState.m_RIP], rax

	.Vectors:
		vmovapd [rcx + RegisterState.m_XMM6],  xmm6
		vmovapd [rcx + RegisterState.m_XMM7],  xmm7
		vmovapd [rcx + RegisterState.m_XMM8],  xmm8
		vmovapd [rcx + RegisterState.m_XMM9],  xmm9
		vmovapd [rcx + RegisterState.m_XMM10], xmm10
		vmovapd [rcx + RegisterState.m_XMM11], xmm11
		vmovapd [rcx + RegisterState.m_XMM12], xmm12
		vmovapd [rcx + RegisterState.m_XMM13], xmm13
		vmovapd [rcx + RegisterState.m_XMM14], xmm14
		vmovapd [rcx + RegisterState.m_XMM15], xmm15

	ret

GlobalLabel fibers_msabi_restore ; RCX => state, RDX => entry
	mov rbx, [rcx + RegisterState.m_RBX]
	mov rdi, [rcx + RegisterState.m_RDI]
	mov rsi, [rcx + RegisterState.m_RSI]
	mov rbp, [rcx + RegisterState.m_RBP]
	mov rsp, [rcx + RegisterState.m_RSP]
	mov r12, [rcx + RegisterState.m_R12]
	mov r13, [rcx + RegisterState.m_R13]
	mov r14, [rcx + RegisterState.m_R14]
	mov r15, [rcx + RegisterState.m_R15]
	mov rax, [rcx + RegisterState.m_RIP]
	sub rsp, 8h
	mov [rsp], rax

	vmovapd xmm6,  [rcx + RegisterState.m_XMM6]
	vmovapd xmm7,  [rcx + RegisterState.m_XMM7]
	vmovapd xmm8,  [rcx + RegisterState.m_XMM8]
	vmovapd xmm9,  [rcx + RegisterState.m_XMM9]
	vmovapd xmm10, [rcx + RegisterState.m_XMM10]
	vmovapd xmm11, [rcx + RegisterState.m_XMM11]
	vmovapd xmm12, [rcx + RegisterState.m_XMM12]
	vmovapd xmm13, [rcx + RegisterState.m_XMM13]
	vmovapd xmm14, [rcx + RegisterState.m_XMM14]
	vmovapd xmm15, [rcx + RegisterState.m_XMM15]

	cmp rdx, 0h
	je .Exit

	vmovapd xmm0, [rcx + RegisterState.m_XMM0]
	vmovapd xmm1, [rcx + RegisterState.m_XMM1]
	vmovapd xmm2, [rcx + RegisterState.m_XMM2]
	vmovapd xmm3, [rcx + RegisterState.m_XMM3]

	mov rdx, [rcx + RegisterState.m_RDX]
	mov r8,  [rcx + RegisterState.m_R8]
	mov r9,  [rcx + RegisterState.m_R9]
	mov rcx, [rcx + RegisterState.m_RCX]

	.Exit:
		ret

%elif BUILD_IS_SYSTEM_UNIX

GlobalLabel fibers_msabi_store ; RDI => state, RSI => returnAddress
	mov rcx, rdi
	mov rdx, rsi

	mov [rcx + RegisterState.m_RBX], rbx
	mov [rcx + RegisterState.m_RDI], rdi
	mov [rcx + RegisterState.m_RSI], rsi
	mov [rcx + RegisterState.m_RBP], rbp
	mov rax, rsp
	add rax, 8h
	mov [rcx + RegisterState.m_RSP], rax
	mov [rcx + RegisterState.m_R12], r12
	mov [rcx + RegisterState.m_R13], r13
	mov [rcx + RegisterState.m_R14], r14
	mov [rcx + RegisterState.m_R15], r15
	cmp rdx, 0h
	je .OrigRIPStore
	mov [rcx + RegisterState.m_RIP], rdx
	jmp .Vectors
	
	.OrigRIPStore:
		mov rax, [rsp]
		mov [rcx + RegisterState.m_RIP], rax

	.Vectors:
		vmovapd [rcx + RegisterState.m_XMM6],  xmm6
		vmovapd [rcx + RegisterState.m_XMM7],  xmm7
		vmovapd [rcx + RegisterState.m_XMM8],  xmm8
		vmovapd [rcx + RegisterState.m_XMM9],  xmm9
		vmovapd [rcx + RegisterState.m_XMM10], xmm10
		vmovapd [rcx + RegisterState.m_XMM11], xmm11
		vmovapd [rcx + RegisterState.m_XMM12], xmm12
		vmovapd [rcx + RegisterState.m_XMM13], xmm13
		vmovapd [rcx + RegisterState.m_XMM14], xmm14
		vmovapd [rcx + RegisterState.m_XMM15], xmm15

	ret

GlobalLabel fibers_msabi_restore ; RDI => state, RSI => entry
	mov rcx, rdi
	mov rdx, rsi

	mov rbx, [rcx + RegisterState.m_RBX]
	mov rdi, [rcx + RegisterState.m_RDI]
	mov rsi, [rcx + RegisterState.m_RSI]
	mov rbp, [rcx + RegisterState.m_RBP]
	mov rsp, [rcx + RegisterState.m_RSP]
	mov r12, [rcx + RegisterState.m_R12]
	mov r13, [rcx + RegisterState.m_R13]
	mov r14, [rcx + RegisterState.m_R14]
	mov r15, [rcx + RegisterState.m_R15]
	mov rax, [rcx + RegisterState.m_RIP]
	sub rsp, 8h
	mov [rsp], rax

	vmovapd xmm6,  [rcx + RegisterState.m_XMM6]
	vmovapd xmm7,  [rcx + RegisterState.m_XMM7]
	vmovapd xmm8,  [rcx + RegisterState.m_XMM8]
	vmovapd xmm9,  [rcx + RegisterState.m_XMM9]
	vmovapd xmm10, [rcx + RegisterState.m_XMM10]
	vmovapd xmm11, [rcx + RegisterState.m_XMM11]
	vmovapd xmm12, [rcx + RegisterState.m_XMM12]
	vmovapd xmm13, [rcx + RegisterState.m_XMM13]
	vmovapd xmm14, [rcx + RegisterState.m_XMM14]
	vmovapd xmm15, [rcx + RegisterState.m_XMM15]

	cmp rdx, 0h
	je .Exit

	vmovapd xmm0, [rcx + RegisterState.m_XMM0]
	vmovapd xmm1, [rcx + RegisterState.m_XMM1]
	vmovapd xmm2, [rcx + RegisterState.m_XMM2]
	vmovapd xmm3, [rcx + RegisterState.m_XMM3]

	mov rdx, [rcx + RegisterState.m_RDX]
	mov r8,  [rcx + RegisterState.m_R8]
	mov r9,  [rcx + RegisterState.m_R9]
	mov rcx, [rcx + RegisterState.m_RCX]

	.Exit:
		ret

%endif