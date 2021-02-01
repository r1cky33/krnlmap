#pragma once

namespace utils {
	PVOID ResolveRelativeAddress(
		_In_ PVOID Instruction,
		_In_ ULONG OffsetOffset,
		_In_ ULONG InstructionSize
	);

	uintptr_t scan_pattern(uint8_t* base, 
		const size_t size,
		char* pattern, 
		char* mask);

	KPROCESSOR_MODE ke_set_previous_mode(KPROCESSOR_MODE mode);

	bool null_pfn(defs::PMDL mdl);
	bool remove_from_bigpool(uint64_t Address);
}
