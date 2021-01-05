#pragma once

namespace defs {
	typedef enum _SYS_INFO_CLASS {
		SystemBasicInformation,
		SystemProcessorInformation,
		SystemPerformanceInformation,
		SystemTimeOfDayInformation,
		SystemPathInformation,
		SystemProcessInformation,
		SystemCallCountInformation,
		SystemDeviceInformation,
		SystemProcessorPerformanceInformation,
		SystemFlagsInformation,
		SystemCallTimeInformation,
		SystemModuleInformation,
		SystemLocksInformation,
		SystemStackTraceInformation,
		SystemPagedPoolInformation,
		SystemNonPagedPoolInformation,
		SystemHandleInformation,
		SystemObjectInformation,
		SystemPageFileInformation,
		SystemVdmInstemulInformation,
		SystemVdmBopInformation,
		SystemFileCacheInformation,
		SystemPoolTagInformation,
		SystemInterruptInformation,
		SystemDpcBehaviorInformation,
		SystemFullMemoryInformation,
		SystemLoadGdiDriverInformation,
		SystemUnloadGdiDriverInformation,
		SystemTimeAdjustmentInformation,
		SystemSummaryMemoryInformation,
		SystemNextEventIdInformation,
		SystemEventIdsInformation,
		SystemCrashDumpInformation,
		SystemExceptionInformation,
		SystemCrashDumpStateInformation,
		SystemKernelDebuggerInformation,
		SystemContextSwitchInformation,
		SystemRegistryQuotaInformation,
		SystemExtendServiceTableInformation,
		SystemPrioritySeperation,
		SystemPlugPlayBusInformation,
		SystemDockInformation
	} SYS_INFO_CLASS, * PSYS_INFO_CLASS;

	typedef struct _LDR_DATA_TABLE_ENTRY
	{
		LIST_ENTRY InLoadOrderLinks;
		LIST_ENTRY InMemoryOrderLinks;
		LIST_ENTRY InInitializationOrderLinks;
		PVOID DllBase;
		PVOID EntryPoint;
		ULONG SizeOfImage;
		UNICODE_STRING FullDllName;
		UNICODE_STRING BaseDllName;
		ULONG Flags;
		WORD LoadCount;
		WORD TlsIndex;
		union
		{
			LIST_ENTRY HashLinks;
			struct
			{
				PVOID SectionPointer;
				ULONG CheckSum;
			};
		};
		union
		{
			ULONG TimeDateStamp;
			PVOID LoadedImports;
		};
		PVOID EntryPointActivationContext;
		PVOID PatchInformation;
		LIST_ENTRY ForwarderLinks;
		LIST_ENTRY ServiceTagLinks;
		LIST_ENTRY StaticLinks;
	} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;


	//FOR FINDING SYSTEM MODULES
	typedef struct _RTL_PROCESS_MODULE_INFORMATION
	{
		HANDLE Section;
		PVOID MappedBase;
		PVOID ImageBase;
		ULONG ImageSize;
		ULONG Flags;
		USHORT LoadOrderIndex;
		USHORT InitOrderIndex;
		USHORT LoadCount;
		USHORT OffsetToFileName;
		UCHAR  FullPathName[256];
	} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

	typedef struct _RTL_PROCESS_MODULES
	{
		ULONG NumberOfModules;
		RTL_PROCESS_MODULE_INFORMATION Modules[1];
	} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

	//ITERATING PEB_LDR FOR FINDING UM MODULES
	typedef struct _PEB_LDR_DATA {
		ULONG Length;
		BOOLEAN Initialized;
		PVOID SsHandle;
		LIST_ENTRY ModuleListLoadOrder;
		LIST_ENTRY ModuleListMemoryOrder;
		LIST_ENTRY ModuleListInitOrder;
	} PEB_LDR_DATA, * PPEB_LDR_DATA;

	typedef struct _RTL_USER_PROCESS_PARAMETERS {
		BYTE Reserved1[16];
		PVOID Reserved2[10];
		UNICODE_STRING ImagePathName;
		UNICODE_STRING CommandLine;
	} RTL_USER_PROCESS_PARAMETERS, * PRTL_USER_PROCESS_PARAMETERS;

	typedef void(__stdcall* PPS_POST_PROCESS_INIT_ROUTINE)(void);

	typedef struct _PEB {
		BYTE Reserved1[2];
		BYTE BeingDebugged;
		BYTE Reserved2[1];
		PVOID Reserved3[2];
		PPEB_LDR_DATA Ldr;
		PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
		PVOID Reserved4[3];
		PVOID AtlThunkSListPtr;
		PVOID Reserved5;
		ULONG Reserved6;
		PVOID Reserved7;
		ULONG Reserved8;
		ULONG AtlThunkSListPtr32;
		PVOID Reserved9[45];
		BYTE Reserved10[96];
		PPS_POST_PROCESS_INIT_ROUTINE PostProcessInitRoutine;
		BYTE Reserved11[128];
		PVOID Reserved12[1];
		ULONG SessionId;
	} PEB, * PPEB;

	// Win32k structures
	typedef struct _wnd_user_info {
		HANDLE window;
		HANDLE region;
		char unk1[0x8];
		DWORD exstyle;
		DWORD style;
		void* instance_handle;
		char unk2[0x50];
		void* wnd_procedure;
	} wnd_user_info, * pwnd_user_info;

	struct tag_thread_info
	{
		PETHREAD owning_thread;
	};

	struct tag_wnd
	{
		HANDLE window;
		void* win32_thread;
		tag_thread_info* thread_info;
		char unk1[0x8];
		tag_wnd* self;
		wnd_user_info* user_info;
		HANDLE region;
		void* region_info;
		tag_wnd* parent;
		tag_wnd* next;
		void* unk2;
		tag_wnd* child;
		tag_wnd* previous;
		void* unk3;
		void* win32;
		void* global_info_link;
		char unk4[0x48];
		DWORD user_procedures_link;
		char unk5[0x1c];
		DWORD procedure_flag;
		char unk6[0x3C];
		void* procedure_table;
	};

	struct _KLDR_DATA_TABLE_ENTRY
	{
		struct _LIST_ENTRY InLoadOrderLinks;                     
		VOID* ExceptionTable;                                            
		ULONG ExceptionTableSize;                                            
		VOID* GpValue;                                                    
		struct _NON_PAGED_DEBUG_INFO* NonPagedDebugInfo;                    
		VOID* DllBase;                                                 
		VOID* EntryPoint;                                                
		ULONG SizeOfImage;                                                
		struct _UNICODE_STRING FullDllName;                          
		struct _UNICODE_STRING BaseDllName;                         
		ULONG Flags;                                                
		USHORT LoadCount;                                       
		union
		{
			USHORT SignatureLevel : 4;                                    
			USHORT SignatureType : 3;                                      
			USHORT Unused : 9;                                          
			USHORT EntireField;                                       
		} u1;                                                            
		VOID* SectionPointer;                                           
		ULONG CheckSum;                                               
		ULONG CoverageSectionSize;                                    
		VOID* CoverageSection;                                   
		VOID* LoadedImports;                               
		VOID* Spare;                                              
		ULONG SizeOfImageNotRounded;                        
		ULONG TimeDateStamp;                               
	};

	struct _POOL_TRACKER_BIG_PAGES
	{
		volatile ULONGLONG Va;                                                  //0x0
		ULONG Key;                                                              //0x8
		ULONG Pattern : 8;                                                        //0xc
		ULONG PoolType : 12;                                                      //0xc
		ULONG SlushSize : 12;                                                     //0xc
		ULONGLONG NumberOfBytes;                                                //0x10
	};
}