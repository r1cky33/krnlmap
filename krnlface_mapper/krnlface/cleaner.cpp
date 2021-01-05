#include "stdafx.h"

namespace cleaner {
	NTSTATUS force_delete_file(PUNICODE_STRING pUsDriverPath)
	{
		IO_STATUS_BLOCK IoStatusBlock;
		HANDLE FileHandle;
		OBJECT_ATTRIBUTES ObjectAttributes;
		InitializeObjectAttributes(
			&ObjectAttributes,
			pUsDriverPath,
			OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
			0,
			0);

		NTSTATUS Status = IoCreateFileEx(&FileHandle,
			SYNCHRONIZE | DELETE,
			&ObjectAttributes,
			&IoStatusBlock,
			nullptr,
			FILE_ATTRIBUTE_NORMAL,
			FILE_SHARE_DELETE,
			FILE_OPEN,
			FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
			nullptr,
			0,
			CreateFileTypeNone,
			nullptr,
			IO_NO_PARAMETER_CHECKING,
			nullptr);

		if (!NT_SUCCESS(Status)) { return Status; }

		PFILE_OBJECT FileObject;
		Status = ObReferenceObjectByHandleWithTag(FileHandle,
			SYNCHRONIZE | DELETE,
			*IoFileObjectType,
			KernelMode,
			'eliF',
			reinterpret_cast<PVOID*>(&FileObject),
			nullptr);
		if (!NT_SUCCESS(Status))
		{
			ObCloseHandle(FileHandle, KernelMode);
			return Status;
		}

		const PSECTION_OBJECT_POINTERS SectionObjectPointer = FileObject->SectionObjectPointer;
		SectionObjectPointer->ImageSectionObject = nullptr;

		CONST BOOLEAN ImageSectionFlushed = MmFlushImageSection(SectionObjectPointer, MmFlushForDelete);

		ObfDereferenceObject(FileObject);
		ObCloseHandle(FileHandle, KernelMode);

		if (ImageSectionFlushed)
		{
			Status = ZwDeleteFile(&ObjectAttributes);
			if (NT_SUCCESS(Status))
			{
				return Status;
			}
		}
		return Status;
	}
}