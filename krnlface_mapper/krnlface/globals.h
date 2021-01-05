#pragma once

namespace global {
	bool get_os();

	extern uint32_t os_build_number;

	namespace eprocess {
		extern uint32_t o_imagefilename;
		extern uint32_t o_activeprocesslinks;
	}

	namespace ethread {
		extern uint32_t o_threadlistentry;
		extern uint32_t o_threadlisthead;
	}
}
