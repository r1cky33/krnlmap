#pragma once

#include "logger.h"

#define kprintf(...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, DPFLTR_ERROR_LEVEL, __VA_ARGS__)
#define errlog(str) logger::errlog(str)

#define POOLTAG 'lieH'

#define JMP_SIZE (14)
#define MAPPER_DATA_SIZE (JMP_SIZE + 7)
#define MAPPER_BUFFER_SIZE (((IMAGE_NT_HEADERS64 *)(payload + ((IMAGE_DOS_HEADER *)payload)->e_lfanew))->OptionalHeader.SizeOfImage)

#ifndef _COMMON_TYPES
#define _COMMON_TYPES
typedef unsigned short  WORD;
typedef unsigned char   BYTE;
typedef unsigned long   DWORD;

#define near
typedef BYTE near* PBYTE;
typedef int near* PINT;

typedef unsigned char byte;
#endif

#include <ntifs.h>
#include <ntddk.h>
#include <ntdef.h>
#include <ntimage.h>
#include <intrin.h>

#include "stdint.h"
#include "defs.h"
#include "imports.h"

#include "globals.h"
#include "utils.h"
#include "krnlutils.h"
#include "cleaner.h"

#include "payload.h"