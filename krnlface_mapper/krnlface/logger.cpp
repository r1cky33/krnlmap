#include "stdafx.h"

#include <ntstrsafe.h>

namespace logger {
#define BUFFER_SIZE 30

    void errlog(char* error) {
        UNICODE_STRING     uniName;
        OBJECT_ATTRIBUTES  objAttr;

        RtlInitUnicodeString(&uniName, L"\\SystemRoot\\krnl_log.txt");
        InitializeObjectAttributes(&objAttr, &uniName,
            OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE,
            NULL, NULL);

        HANDLE   handle;
        NTSTATUS ntstatus;
        IO_STATUS_BLOCK    ioStatusBlock;

        if (KeGetCurrentIrql() != PASSIVE_LEVEL) return;

        ntstatus = ZwCreateFile(&handle,
            GENERIC_WRITE,
            &objAttr, &ioStatusBlock, NULL,
            FILE_ATTRIBUTE_NORMAL,
            0,
            FILE_OPEN_IF,
            FILE_SYNCHRONOUS_IO_NONALERT,
            NULL, 0);

        CHAR     buffer[BUFFER_SIZE];
        size_t  cb;

        if (NT_SUCCESS(ntstatus)) {
            ntstatus = RtlStringCbPrintfA(buffer, sizeof(buffer), error, 0x0);
            if (NT_SUCCESS(ntstatus)) {
                ntstatus = RtlStringCbLengthA(buffer, sizeof(buffer), &cb);
                if (NT_SUCCESS(ntstatus)) {
                    ntstatus = ZwWriteFile(handle, NULL, NULL, NULL, &ioStatusBlock,
                        buffer, (ULONG)cb, NULL, NULL);
                }
            }
            ZwClose(handle);
        }
    }
}