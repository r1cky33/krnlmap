#include "stdafx.h"

using namespace defs;

namespace krnlutils {
	uintptr_t get_process_by_name(const char* process_name) {
		if (!global::eprocess::o_activeprocesslinks || !global::eprocess::o_imagefilename)
			return 0;

		uintptr_t list_head = *(uintptr_t*)((uintptr_t)PsInitialSystemProcess + global::eprocess::o_activeprocesslinks);
		uintptr_t list_current = list_head;

		do
		{
			uintptr_t list_entry = list_current - global::eprocess::o_activeprocesslinks;
			if (!_stricmp(process_name, (char*)(list_entry + global::eprocess::o_imagefilename)))
			{
				return list_entry;
			}
			list_current = *(uintptr_t*)list_current;
		} while (list_current != list_head);
		return 0;
	}

	uintptr_t get_processbase(PEPROCESS pprocess) {
		return reinterpret_cast<uintptr_t>(PsGetProcessSectionBaseAddress(pprocess));
	}

	uintptr_t get_processbase_by_id(uint32_t process_id) {
		PEPROCESS pProcess;
		if (NT_SUCCESS(PsLookupProcessByProcessId((HANDLE)process_id, &pProcess))) {
			uintptr_t base = (uintptr_t)PsGetProcessSectionBaseAddress(pProcess);
			ObDereferenceObject(pProcess);
			return base;
		}
		return 0;
	}

	uintptr_t get_km_module(const char* module_name, size_t& module_size) {
		ULONG bytes = 0;
		NTSTATUS status = ZwQuerySystemInformation(defs::SystemModuleInformation, 0, bytes, &bytes);

		if (!bytes)
			return 0;

		PRTL_PROCESS_MODULES modules = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, bytes, POOLTAG);
		status = ZwQuerySystemInformation(defs::SystemModuleInformation, modules, bytes, &bytes);

		if (!NT_SUCCESS(status)) {
			ExFreePoolWithTag(modules, POOLTAG);
			return 0;
		}

		PRTL_PROCESS_MODULE_INFORMATION module = modules->Modules;
		uintptr_t module_base = 0;
		HANDLE pid = 0;

		for (ULONG i = 0; i < modules->NumberOfModules; i++)
		{
			if (strcmp((char*)(module[i].FullPathName + module[i].OffsetToFileName), module_name) == 0)
			{
				module_base = uintptr_t(module[i].ImageBase);
				module_size = module[i].ImageSize;
				break;
			}
		}

		if (modules)
			ExFreePoolWithTag(modules, POOLTAG);

		if (module_base <= 0)
			return 0;

		return module_base;
	}

	uintptr_t get_km_module_export(const char* module_name, const char* routine_name) {
		size_t size;
		uintptr_t lpModule = get_km_module(module_name, size);

		if (!lpModule)
			return NULL;

		return (uintptr_t)RtlFindExportedRoutineByName((PVOID)lpModule, routine_name);
	}

	uintptr_t get_um_module_base(PEPROCESS pprocess, LPCWSTR module_name) {
		uintptr_t base;

		if (!pprocess) {
			return 0;
		}

		KeAttachProcess((PKPROCESS)pprocess);

		defs::PPEB peb = (defs::PPEB)PsGetProcessPeb(pprocess);
		if (!peb) {
			KeDetachProcess();
			return 0;
		}

		if (!peb->Ldr || !peb->Ldr->Initialized) {
			KeDetachProcess();
			return 0;
		}

		if (!module_name) {
			return 0;
		}

		UNICODE_STRING module_name_unicode;
		RtlInitUnicodeString(&module_name_unicode, module_name);

		for (PLIST_ENTRY list = peb->Ldr->ModuleListLoadOrder.Flink;
			list != &peb->Ldr->ModuleListLoadOrder;
			list = list->Flink) {
			PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			if (RtlCompareUnicodeString(&entry->BaseDllName, &module_name_unicode, TRUE) == 0) {
				base = (uintptr_t)entry->DllBase;
				KeDetachProcess();
				return base;
			}
		}

		KeDetachProcess();
		return 0;
	}

	uintptr_t get_um_module_size(PEPROCESS pprocess, LPCWSTR module_name) {
		uintptr_t size;

		if (!pprocess) {
			return 0;
		}

		KeAttachProcess((PKPROCESS)pprocess);

		defs::PPEB peb = (defs::PPEB)PsGetProcessPeb(pprocess);
		if (!peb) {
			KeDetachProcess();
			return 0;
		}

		if (!peb->Ldr || !peb->Ldr->Initialized) {
			KeDetachProcess();
			return 0;
		}

		if (!module_name) {
			return 0;
		}

		UNICODE_STRING module_name_unicode;
		RtlInitUnicodeString(&module_name_unicode, module_name);

		for (PLIST_ENTRY list = peb->Ldr->ModuleListLoadOrder.Flink;
			list != &peb->Ldr->ModuleListLoadOrder;
			list = list->Flink) {
			PLDR_DATA_TABLE_ENTRY entry = CONTAINING_RECORD(list, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
			if (RtlCompareUnicodeString(&entry->BaseDllName, &module_name_unicode, TRUE) == 0) {
				size = (uintptr_t)entry->SizeOfImage;
				KeDetachProcess();
				return size;
			}
		}

		KeDetachProcess();
		return 0;
	}

	bool memcpy_to_readonly(PVOID dest, PVOID src, ULONG length) {
		PMDL mdl = IoAllocateMdl(dest, length, FALSE, FALSE, NULL);
		if (!mdl) {
			return FALSE;
		}

		MmProbeAndLockPages(mdl, KernelMode, IoModifyAccess);

		PVOID mapped = MmMapLockedPagesSpecifyCache(mdl, KernelMode, MmNonCached, NULL, 0, HighPagePriority);
		if (!mapped) {
			MmUnlockPages(mdl);
			IoFreeMdl(mdl);
			return FALSE;
		}

		memcpy(mapped, src, length);

		MmUnmapLockedPages(mapped, mdl);
		MmUnlockPages(mdl);
		IoFreeMdl(mdl);
		return TRUE;
	}

	bool find_pool_table(uint64_t* pbool_big_page_table, uint64_t* p_bool_big_page_table_size) {
		const char* pattern = "\xE8\x00\x00\x00\x00\x83\x67\x0C\x00";
		const char* mask = "x????xxxx";

		size_t ntsize{};
		uint64_t ntbase = krnlutils::get_km_module("ntoskrnl.exe", ntsize);

		if (!ntbase) { return false; }

		void* protect_pool_call_instruction_address = reinterpret_cast<void*>(utils::scan_pattern((uint8_t*)ntbase, ntsize, (char*)pattern, (char*)mask));
		if (!protect_pool_call_instruction_address) { return false; }

		void* expreotectpoolexaddress = utils::ResolveRelativeAddress(protect_pool_call_instruction_address, 1, 5);
		if (!expreotectpoolexaddress) { return false; }

		PVOID PoolBigPageTableInstructionAddress = (PVOID)((ULONG64)expreotectpoolexaddress + 0x95);
		*pbool_big_page_table = (uint64_t)utils::ResolveRelativeAddress(PoolBigPageTableInstructionAddress, 3, 7);

		PVOID PoolBigPageTableSizeInstructionAddress = (PVOID)((ULONG64)expreotectpoolexaddress + 0x8E);
		*p_bool_big_page_table_size = (uint64_t)utils::ResolveRelativeAddress(PoolBigPageTableSizeInstructionAddress, 3, 7);

		return true;
	}

	bool remove_from_big_pool(uint64_t address) {
		uint64_t pPoolBigPageTable = 0;
		uint64_t pPoolBigPageTableSize = 0;

		if (find_pool_table(&pPoolBigPageTable, &pPoolBigPageTableSize))
		{
			_POOL_TRACKER_BIG_PAGES* PoolBigPageTable = 0;
			RtlCopyMemory(&PoolBigPageTable, (PVOID)pPoolBigPageTable, 8);
			SIZE_T PoolBigPageTableSize = 0;
			RtlCopyMemory(&PoolBigPageTableSize, (PVOID)pPoolBigPageTableSize, 8);

			for (int i = 0; i < PoolBigPageTableSize; i++)
			{
				if (PoolBigPageTable[i].Va == address || PoolBigPageTable[i].Va == (address + 0x1))
				{
					PoolBigPageTable[i].Va = 0x10000000;
					PoolBigPageTable[i].NumberOfBytes = 0x0;
					return true;
				}
			}

			return false;
		}

		return false;
	}
}