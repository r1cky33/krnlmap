#include "stdafx.h"

namespace global {
	bool get_os() {
		RTL_OSVERSIONINFOW version{};
		if (!NT_SUCCESS(RtlGetVersion(&version)))
			return false;

		os_build_number = version.dwBuildNumber;
		if (version.dwBuildNumber == 19041 || version.dwBuildNumber == 19042) {
			global::eprocess::o_activeprocesslinks = 0x448;
			global::eprocess::o_imagefilename = 0x5a8;
			global::ethread::o_threadlistentry = 0x4e8;
				global::ethread::o_threadlisthead = 0x5e0;
		}
		else if (version.dwBuildNumber == 18363 || version.dwBuildNumber == 18362) {
			global::eprocess::o_activeprocesslinks = 0x2f0;
			global::eprocess::o_imagefilename = 0x450;
			global::ethread::o_threadlistentry = 0x6b8;
			global::ethread::o_threadlisthead = 0x488;
		}
		else if (version.dwBuildNumber <= 17763) {
			global::eprocess::o_activeprocesslinks = 0x2e8;
			global::eprocess::o_imagefilename = 0x450;
			global::ethread::o_threadlistentry = 0x6b8;
			global::ethread::o_threadlisthead = 0x488;
		}

		return true;
	}

	uint32_t os_build_number{};

	namespace eprocess {
		uint32_t o_imagefilename{};
		uint32_t o_activeprocesslinks{};
	}

	namespace ethread {
		uint32_t o_threadlistentry{};
		uint32_t o_threadlisthead{};
	}
}