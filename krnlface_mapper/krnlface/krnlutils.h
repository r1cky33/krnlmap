#pragma once

namespace krnlutils {
	uintptr_t	get_process_by_name(const char* process_name);
	uintptr_t	get_processbase(PEPROCESS pprocess);
	uintptr_t	get_processbase_by_id(uint32_t process_id);
	uintptr_t	get_km_module(const char* module_name, size_t& module_size);
	uintptr_t	get_km_module_export(const char* module_name, const char* routine_name);
	uintptr_t	get_um_module_base(PEPROCESS pprocess, LPCWSTR module_name);
	uintptr_t	get_um_module_size(PEPROCESS pProcess, LPCWSTR module_name);

	bool		memcpy_to_readonly(PVOID dest, PVOID src, ULONG length);
}