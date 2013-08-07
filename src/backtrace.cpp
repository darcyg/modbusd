/*
 * backtrace.cpp
 *
 *  Created on: Jun 17, 2013
 *      Author: ruinmmal
 */
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <syslog.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include "Log.h"

#if defined(__arm__)
// I'm using ARM GCC 4.1 which has buggy implementation of
// __builtin_frame_address(0). It points to the last register pushed to stack
// instead of the beginning of frame so use assembly function
// __builtin_frame_address works fine on GCC 4.7
extern "C" uintptr_t* get_fp(void);
#else
inline uintptr_t* get_fp(void) {
	return (uintptr_t*)__builtin_frame_address(0);
}
#endif

void print_symbol(uintptr_t addr)
{
    Dl_info dlinf;
	if(dladdr(reinterpret_cast<const void*>(addr), &dlinf) != 0) {
		int status;
		ptrdiff_t offset = (unsigned char*)addr - (unsigned char*)dlinf.dli_saddr;
    	const char * real_name = abi::__cxa_demangle(dlinf.dli_sname, 0, 0, &status);
		if(status != 0)
			real_name = dlinf.dli_sname;
		//printf("[bt] %s(%s+0x%" PRIXPTR ") [0x%" PRIXPTR "]\n", dlinf.dli_fname, real_name, offset, addr);
		LogFatal("[bt] %s(%s+0x%" PRIXPTR ") [0x%" PRIXPTR "]\n", dlinf.dli_fname, real_name, offset, addr);
	} else {
		LogFatal("[bt] cannot get symbol. addr=%" PRIXPTR , addr);
	}
}

void print_backtrace(uintptr_t pc, uintptr_t lr)
{
	LogFatal("<<<<<BACKTRACE START>>>>>\n");
#if defined(__arm__)
	if(pc != 0) {
		print_symbol(pc);
	}

	if(lr != 0) {
		print_symbol(lr);
	}


	if(__builtin_frame_address(0) != get_fp()) {
		printf("WARNING: you have buggy implementation of __builtin_frame_address");
	}

    uintptr_t* topfp = get_fp();
    for (int i=0; i < 10 && topfp != 0; i++) {
        uintptr_t fp = *(topfp -3);
        uintptr_t sp = *(topfp -2);
        uintptr_t lr = *(topfp -1);
        uintptr_t pc = *(topfp -0);

		//first 2 frames contain signal handler
		if(i > 1) {
		    if (fp != 0) print_symbol(lr); // middle frame
		    else print_symbol(pc); // bottom frame, lr invalid
		}
        topfp = (uintptr_t*)fp;
    }
#else
	void* trace[12];
	int trace_size = backtrace(trace, 12);
	if(trace_size <= 1)
		return;

	trace[1] = (void*)pc;

	for(int i = 0; i < trace_size; i++) {
		print_symbol((uintptr_t)trace[i]);
	}
#endif
	LogFatal("<<<<<BACKTRACE END>>>>>\n");
}

void signalHandler(int sig __attribute__((unused)),
                   void *info __attribute__((unused)),
                   void *secret)
{
	uintptr_t fault_addr;
	uintptr_t fault_addr1 = 0;

	ucontext_t *uc = reinterpret_cast<ucontext_t *>(secret);

#if defined(__arm__)
	fault_addr = uc->uc_mcontext.arm_pc;
	fault_addr1 = uc->uc_mcontext.arm_lr;
#else
#if !defined(__i386__) && !defined(__x86_64__)
#error Only ARM, x86 and x86-64 are supported
#endif
#if defined(__x86_64__)
	fault_addr = uc->uc_mcontext.gregs[REG_RIP];
#else
	fault_addr = reinterpret_cast<void *>(uc->uc_mcontext.gregs[REG_EIP]);
#endif
#endif

#if defined (__arm__)
	LogFatal("SIGSEGV at PC=%" PRIXPTR " LR=%" PRIXPTR "\n\t", fault_addr, fault_addr1);
#else
	LogFatal("SIGSEGV at EIP=%" PRIXPTR "\n\t", fault_addr);
#endif
	print_backtrace(fault_addr, fault_addr1);

	_exit(EXIT_FAILURE);
}

void install_sigsegv()
{
	struct sigaction sa;
	sa.sa_handler = (__sighandler_t)signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_SIGINFO;
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGABRT, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
//
//	sa.sa_handler = (__sighandler_t)signalHandler;
//	sigemptyset(&sa.sa_mask);
//	sa.sa_flags = SA_RESTART | SA_SIGINFO;
//	sigaction(SIGABRT, &sa, NULL);
}




