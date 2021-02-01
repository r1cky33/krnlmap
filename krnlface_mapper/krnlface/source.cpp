#include "stdafx.h"

VOID Unload(_DRIVER_OBJECT* driver_object);
typedef NTSTATUS(*remoteEntry)(PDRIVER_OBJECT, PUNICODE_STRING);

NTSTATUS map_driver(void* ntoskrnl_base, void** entryPoint) {
	uint8_t* mapperBase = (uint8_t*)ExAllocatePool(NonPagedPool, MAPPER_BUFFER_SIZE);
	uint8_t* mapperBuffer = payload;

	if (!utils::remove_from_bigpool((uintptr_t)mapperBase)) {
		ExFreePool(mapperBase);
		return STATUS_UNSUCCESSFUL;
	}

	defs::PMDL pMDL = (defs::PMDL)IoAllocateMdl(mapperBase, MAPPER_BUFFER_SIZE, FALSE, FALSE, NULL);
	if (!utils::null_pfn(pMDL)) {
		ExFreePool(mapperBase);
		return STATUS_UNSUCCESSFUL;
	}

	if (!mapperBase)
		return STATUS_UNSUCCESSFUL;

	IMAGE_NT_HEADERS64* ntHeaders =
		(IMAGE_NT_HEADERS64*)(mapperBuffer +
			((IMAGE_DOS_HEADER*)mapperBuffer)->e_lfanew);

	//memcpy(mapperBase, mapperBuffer, ntHeaders->OptionalHeader.SizeOfHeaders);

	// copy random bytes to the header
	for (uint32_t i = 0; i < ntHeaders->OptionalHeader.SizeOfHeaders / 2; i++) {
		mapperBase[i] = 0xFD;
	}

	IMAGE_SECTION_HEADER* sections =
		(IMAGE_SECTION_HEADER*)((UINT8*)&ntHeaders->OptionalHeader +
			ntHeaders->FileHeader.SizeOfOptionalHeader);

	for (UINT16 i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
		IMAGE_SECTION_HEADER* section = &sections[i];
		if (section->SizeOfRawData) {
			memcpy(mapperBase + section->VirtualAddress,
				mapperBuffer + section->PointerToRawData,
				section->SizeOfRawData);
		}
	}

	UINT32 importsRva =
		ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
		.VirtualAddress;

	if (importsRva) {
		IMAGE_IMPORT_DESCRIPTOR* importDescriptor =
			(IMAGE_IMPORT_DESCRIPTOR*)(mapperBase + importsRva);

		for (; importDescriptor->FirstThunk; ++importDescriptor) {
			IMAGE_THUNK_DATA64* thunk =
				(IMAGE_THUNK_DATA64*)(mapperBase +
					importDescriptor->FirstThunk);

			IMAGE_THUNK_DATA64* thunkOriginal =
				(IMAGE_THUNK_DATA64*)(mapperBase +
					importDescriptor->OriginalFirstThunk);

			for (; thunk->u1.AddressOfData; ++thunk, ++thunkOriginal) {
				UINT64 import = krnlutils::get_km_module_export("ntoskrnl.exe", ((IMAGE_IMPORT_BY_NAME*)(mapperBase +
					thunkOriginal->u1.AddressOfData))->Name);

				if (!import) { return STATUS_NOT_FOUND; }

				thunk->u1.Function = import;
			}
		}
	}

	IMAGE_DATA_DIRECTORY* baseRelocDir =
		&ntHeaders->OptionalHeader
		.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

	if (baseRelocDir->VirtualAddress) {
		IMAGE_BASE_RELOCATION* reloc =
			(IMAGE_BASE_RELOCATION*)(mapperBase +
				baseRelocDir->VirtualAddress);

		for (UINT32 currentSize = 0; currentSize < baseRelocDir->Size;) {
			UINT32 relocCount =
				(reloc->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) /
				sizeof(UINT16);

			UINT16* relocData =
				(UINT16*)((UINT8*)reloc + sizeof(IMAGE_BASE_RELOCATION));

			UINT8* relocBase = mapperBase + reloc->VirtualAddress;

			for (UINT32 i = 0; i < relocCount; ++i, ++relocData) {
				UINT16 data = *relocData;
				UINT16 type = data >> 12;
				UINT16 offset = data & 0xFFF;

				switch (type) {
				case IMAGE_REL_BASED_ABSOLUTE:
					break;
				case IMAGE_REL_BASED_DIR64: {
					UINT64* rva = (UINT64*)(relocBase + offset);
					*rva =
						(UINT64)(mapperBase +
							(*rva - ntHeaders->OptionalHeader.ImageBase));
					break;
				}
				default:
					return STATUS_UNSUCCESSFUL;
				}
			}
			currentSize += reloc->SizeOfBlock;
			reloc = (IMAGE_BASE_RELOCATION*)relocData;
		}
	}

	*entryPoint = mapperBase + ntHeaders->OptionalHeader.AddressOfEntryPoint;
	return STATUS_SUCCESS;
}

remoteEntry setup_mapper(uintptr_t ntoskrnl) {
	void* driverEntryPoint = nullptr;
	NTSTATUS status = map_driver((void*)ntoskrnl, &driverEntryPoint);
	if (!NT_SUCCESS(status) || !driverEntryPoint)
		return 0;

	return (remoteEntry)driverEntryPoint;
}

bool main(PDRIVER_OBJECT  driver_object,
	PUNICODE_STRING registry_path) {
	size_t krnl_size = 0;
	uintptr_t krnl_base = krnlutils::get_km_module("ntoskrnl.exe", krnl_size);

	if (!krnl_base) { errlog("[map] no krnl found"); return false; }

	remoteEntry driverEntry = (remoteEntry)setup_mapper(krnl_base);
	if (!driverEntry) {
		errlog("[map] no entry found");
		return false;
	}

	if (!NT_SUCCESS(driverEntry(driver_object, registry_path))) {
		return false;
	}
}

extern "C" NTSTATUS driver_entry(
	PDRIVER_OBJECT  driver_object,
	PUNICODE_STRING registry_path
) {
	driver_object->DriverUnload = (PDRIVER_UNLOAD)Unload;

	if(!NT_SUCCESS(cleaner::force_delete_file(&((defs::_KLDR_DATA_TABLE_ENTRY*)driver_object->DriverSection)->FullDllName)))
		return STATUS_FAILED_DRIVER_ENTRY;

	if (!global::get_os())
		return STATUS_FAILED_DRIVER_ENTRY;

	if (!main(driver_object, registry_path))
		return STATUS_FAILED_DRIVER_ENTRY;

	return STATUS_FAILED_DRIVER_ENTRY;
}

VOID Unload(_DRIVER_OBJECT* driver_object) {
}