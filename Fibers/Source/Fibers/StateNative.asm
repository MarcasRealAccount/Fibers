%include "Fibers/State.asminc"

extern fibers_msabi_store
extern fibers_msabi_restore
extern fibers_sysvabi_store
extern fibers_sysvabi_restore

%if BUILD_IS_SYSTEM_WINDOWS

GlobalLabel fibers_storerestore ; RCX => storeState, RDX => returnAddress, R8 => restoreState, R9 => entry
	cmp rdx, 0h
	jne .Store
	mov rdx, [rsp]
	add rsp, 8h

	.Store:
		lea rax, [.StoreJmpTable]
		mov r10d, [rcx + RegisterState.m_CallingConvention]
		jmp [rax + r10 * 8]

		.StoreMSAbi:
			call fibers_msabi_store
			jmp .Restore

		.StoreSYSVAbi:
			call fibers_sysvabi_store
			jmp .Restore
		
		.StoreJmpTable:
			dq .StoreMSAbi
			dq .StoreMSAbi
			dq .StoreSYSVAbi

	.Restore:
		lea rax, [.RestoreJmpTable]
		mov r10d, [r8 + RegisterState.m_CallingConvention]
		mov rcx, r8
		mov rdx, r9
		jmp [rax + r10 * 8]

		.RestoreMSAbi:
			call fibers_msabi_restore
			jmp .Exit

		.RestoreSYSVAbi:
			call fibers_sysvabi_restore
			jmp .Exit
		
		.RestoreJmpTable:
			dq .RestoreMSAbi
			dq .RestoreMSAbi
			dq .RestoreSYSVAbi

	.Exit:
		ret

%elif BUILD_IS_SYSTEM_UNIX

GlobalLabel fibers_storerestore ; RDI => storeState, RSI => returnAddress, RDX => restoreState, RCX => entry
	cmp rsi, 0h
	jne .Store
	mov rsi, [rsp]
	add rsp, 8

	.Store:
		lea r10, [.StoreJmpTable]
		mov eax, [rdi + RegisterState.m_CallingConvention]
		mov rax, [r10 + rax * 8]
		add rax, r10
		jmp rax

		.StoreJmpTable:
			dq .StoreMSAbi - .StoreJmpTable
			dq .StoreMSAbi - .StoreJmpTable
			dq .StoreSYSVAbi - .StoreJmpTable

		.StoreMSAbi:
			call fibers_msabi_store
			jmp .Restore

		.StoreSYSVAbi:
			call fibers_sysvabi_store
			jmp .Restore

	.Restore:
		lea r10, [.RestoreJmpTable]
		mov eax, [rdx + RegisterState.m_CallingConvention]
		mov rax, [r10 + rax * 8]
		add rax, r10
		mov rdi, rdx
		mov rsi, rcx
		jmp rax
		
		.RestoreJmpTable:
			dq .RestoreMSAbi - .RestoreJmpTable
			dq .RestoreMSAbi - .RestoreJmpTable
			dq .RestoreSYSVAbi - .RestoreJmpTable

		.RestoreMSAbi:
			call fibers_msabi_restore
			jmp .Exit

		.RestoreSYSVAbi:
			call fibers_sysvabi_restore
			jmp .Exit

	.Exit:
		ret

%endif