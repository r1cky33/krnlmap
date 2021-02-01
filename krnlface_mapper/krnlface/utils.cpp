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

	bool find_pool_table(uint64_t* pPoolBigPageTable, uint64_t* pPoolBigPageTableSize)
	{
		size_t size = 0;
		uintptr_t krnl_base = krnlutils::get_km_module("ntoskrnl.exe", size);
		if (!krnl_base) { return false; }

		PVOID ExProtectPoolExCallInstructionsAddress = (PVOID)utils::scan_pattern((uint8_t*)krnl_base, size, "\xE8\x00\x00\x00\x00\x83\x67\x0C\x00", "x????xxxx");

		if (!ExProtectPoolExCallInstructionsAddress)
			return false;

		PVOID ExProtectPoolExAddress = ResolveRelativeAddress(ExProtectPoolExCallInstructionsAddress, 1, 5);

		if (!ExProtectPoolExAddress)
			return false;

		PVOID PoolBigPageTableInstructionAddress = (PVOID)((ULONG64)ExProtectPoolExAddress + 0x95);
		*pPoolBigPageTable = (uint64_t)ResolveRelativeAddress(PoolBigPageTableInstructionAddress, 3, 7);

		PVOID PoolBigPageTableSizeInstructionAddress = (PVOID)((ULONG64)ExProtectPoolExAddress + 0x8E);
		*pPoolBigPageTableSize = (uint64_t)ResolveRelativeAddress(PoolBigPageTableSizeInstructionAddress, 3, 7);

		return true;
	}

	bool remove_from_bigpool(uint64_t Address)
	{
		uint64_t pPoolBigPageTable = 0;
		uint64_t pPoolBigPageTableSize = 0;

		if (find_pool_table(&pPoolBigPageTable, &pPoolBigPageTableSize))
		{
			defs::PPOOL_TRACKER_BIG_PAGES PoolBigPageTable = 0;
			RtlCopyMemory(&PoolBigPageTable, (PVOID)pPoolBigPageTable, 8);
			SIZE_T PoolBigPageTableSize = 0;
			RtlCopyMemory(&PoolBigPageTableSize, (PVOID)pPoolBigPageTableSize, 8);

			for (int i = 0; i < PoolBigPageTableSize; i++)
			{
				if (PoolBigPageTable[i].Va == (void*)Address || PoolBigPageTable[i].Va == (void*)(Address + 0x1))
				{
					PoolBigPageTable[i].Va = (void*)0x1;
					PoolBigPageTable[i].NumberOfBytes = 0x0;
					return true;
				}
			}

			return false;
		}

		return false;
	}

	bool null_pfn(defs::PMDL mdl)
	{
		PPFN_NUMBER mdl_pages = MmGetMdlPfnArray(mdl);
		if (!mdl_pages) { return false; }

		ULONG mdl_page_count = ADDRESS_AND_SIZE_TO_SPAN_PAGES(MmGetMdlVirtualAddress(mdl), MmGetMdlByteCount(mdl));

		ULONG null_pfn = 0x0;
		MM_COPY_ADDRESS source_address = { 0 };
		source_address.VirtualAddress = &null_pfn;

		for (ULONG i = 0; i < mdl_page_count; i++)
		{
			size_t bytes = 0;
			MmCopyMemory(&mdl_pages[i], source_address, sizeof(ULONG), MM_COPY_MEMORY_VIRTUAL, &bytes);
		}
		return true;
	}
}