#include "stdafx.h"

namespace utils {
	PVOID ResolveRelativeAddress(
		_In_ PVOID Instruction,
		_In_ ULONG OffsetOffset,
		_In_ ULONG InstructionSize
	)
	{
		ULONG_PTR Instr = (ULONG_PTR)Instruction;
		LONG RipOffset = *(PLONG)(Instr + OffsetOffset);
		PVOID ResolvedAddr = (PVOID)(Instr + InstructionSize + RipOffset);

		return ResolvedAddr;
	}

	uintptr_t scan_pattern(uint8_t* base, const size_t size, char* pattern, char* mask) {
		const auto patternSize = strlen(mask);

		for (size_t i = {}; i < size - patternSize; i++)
		{
			for (size_t j = {}; j < patternSize; j++)
			{
				if (mask[j] != '?' && *reinterpret_cast<uint8_t*>(base + i + j) != static_cast<uint8_t>(pattern[j]))
					break;

				if (j == patternSize - 1)
					return reinterpret_cast<uintptr_t>(base) + i;
			}
		}
		return {};
	}

	KPROCESSOR_MODE ke_set_previous_mode(KPROCESSOR_MODE mode) {
		KPROCESSOR_MODE old = ExGetPreviousMode();
		ULONG PreviousModeOffset = *(PULONG)((PBYTE)ExGetPreviousMode + 0xC);
		*(KPROCESSOR_MODE*)((PBYTE)KeGetCurrentThread() + PreviousModeOffset) = mode;
		return old;
	}
}